/*
 * iojournal -- JSON console encoder
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    char *buffer;
    size_t capacity;
    size_t length;
} ij_json_builder_t;

typedef struct {
    const char *text;
    size_t len;
} ij_static_json_text_t;

static ij_status_t ij_builder_append_raw_n(ij_json_builder_t *builder, const char *text, size_t len)
{
    if (builder->length + len + 1U > builder->capacity) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    memcpy(builder->buffer + builder->length, text, len);
    builder->length += len;
    builder->buffer[builder->length] = '\0';
    return IJ_STATUS_OK;
}

static bool ij_builder_can_append(const ij_json_builder_t *builder, size_t len)
{
    return builder->length + len + 1U <= builder->capacity;
}

static ij_status_t ij_builder_append_raw(ij_json_builder_t *builder, const char *text)
{
    size_t len = strlen(text);

    return ij_builder_append_raw_n(builder, text, len);
}

static ij_status_t ij_builder_append_char(ij_json_builder_t *builder, char c)
{
    if (builder->length + 2U > builder->capacity) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    builder->buffer[builder->length++] = c;
    builder->buffer[builder->length] = '\0';
    return IJ_STATUS_OK;
}

static char ij_hex_digit(unsigned int value)
{
    return (char)(value < 10U ? ('0' + value) : ('a' + (value - 10U)));
}

static void ij_write_two_digits(char *buffer, unsigned int value)
{
    buffer[0] = (char)('0' + ((value / 10U) % 10U));
    buffer[1] = (char)('0' + (value % 10U));
}

static void ij_write_three_digits(char *buffer, unsigned int value)
{
    buffer[0] = (char)('0' + ((value / 100U) % 10U));
    buffer[1] = (char)('0' + ((value / 10U) % 10U));
    buffer[2] = (char)('0' + (value % 10U));
}

static void ij_write_four_digits(char *buffer, unsigned int value)
{
    buffer[0] = (char)('0' + ((value / 1000U) % 10U));
    buffer[1] = (char)('0' + ((value / 100U) % 10U));
    buffer[2] = (char)('0' + ((value / 10U) % 10U));
    buffer[3] = (char)('0' + (value % 10U));
}

static ij_status_t ij_builder_append_uint64_decimal(ij_json_builder_t *builder, uint64_t value)
{
    char digits[20];
    size_t position = sizeof(digits);

    do {
        digits[--position] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    return ij_builder_append_raw_n(builder, &digits[position], sizeof(digits) - position);
}

static ij_status_t ij_builder_append_int64_decimal(ij_json_builder_t *builder, int64_t value)
{
    uint64_t magnitude = (uint64_t)value;

    if (value < 0) {
        if (ij_builder_append_char(builder, '-') != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }
        magnitude = 0U - magnitude;
    }

    return ij_builder_append_uint64_decimal(builder, magnitude);
}

static ij_status_t ij_builder_append_control_escape(ij_json_builder_t *builder, unsigned char value)
{
    char escaped[6];

    escaped[0] = '\\';
    escaped[1] = 'u';
    escaped[2] = '0';
    escaped[3] = '0';
    escaped[4] = ij_hex_digit((unsigned int)(value >> 4U));
    escaped[5] = ij_hex_digit((unsigned int)(value & 0x0fU));
    return ij_builder_append_raw_n(builder, escaped, sizeof(escaped));
}

static ij_status_t ij_builder_append_escaped_string_n(ij_json_builder_t *builder, const char *text,
                                                      size_t len)
{
    const unsigned char *cursor = (const unsigned char *)text;
    size_t remaining = len;

    while (remaining > 0U) {
        size_t next = ij_json_escape_find_first_special(cursor, remaining);

        if (next > 0U &&
            ij_builder_append_raw_n(builder, (const char *)cursor, next) != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }
        cursor += next;
        remaining -= next;
        if (remaining == 0U) {
            break;
        }

        switch (*cursor) {
        case '\\':
            if (ij_builder_append_raw_n(builder, "\\\\", 2U) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '"':
            if (ij_builder_append_raw_n(builder, "\\\"", 2U) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '\n':
            if (ij_builder_append_raw_n(builder, "\\n", 2U) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '\r':
            if (ij_builder_append_raw_n(builder, "\\r", 2U) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '\t':
            if (ij_builder_append_raw_n(builder, "\\t", 2U) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        default:
            if (*cursor < 0x20U) {
                if (ij_builder_append_control_escape(builder, *cursor) != IJ_STATUS_OK) {
                    return IJ_STATUS_ENCODE_ERROR;
                }
            } else if (ij_builder_append_char(builder, (char)*cursor) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        }
        cursor += 1U;
        remaining -= 1U;
    }

    return IJ_STATUS_OK;
}

static ij_status_t ij_builder_append_json_string_known_escape(ij_json_builder_t *builder,
                                                              const char *text, size_t len,
                                                              bool needs_json_escape)
{
    if (!needs_json_escape) {
        if (!ij_builder_can_append(builder, len + 2U)) {
            return IJ_STATUS_ENCODE_ERROR;
        }

        builder->buffer[builder->length++] = '"';
        memcpy(builder->buffer + builder->length, text, len);
        builder->length += len;
        builder->buffer[builder->length++] = '"';
        builder->buffer[builder->length] = '\0';
        return IJ_STATUS_OK;
    }

    if (ij_builder_append_char(builder, '"') != IJ_STATUS_OK ||
        ij_builder_append_escaped_string_n(builder, text, len) != IJ_STATUS_OK ||
        ij_builder_append_char(builder, '"') != IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    return IJ_STATUS_OK;
}

static ij_status_t ij_builder_append_json_key_known_escape(ij_json_builder_t *builder,
                                                           const char *key, size_t len,
                                                           bool needs_json_escape)
{
    if (!needs_json_escape) {
        if (!ij_builder_can_append(builder, len + 3U)) {
            return IJ_STATUS_ENCODE_ERROR;
        }

        builder->buffer[builder->length++] = '"';
        memcpy(builder->buffer + builder->length, key, len);
        builder->length += len;
        builder->buffer[builder->length++] = '"';
        builder->buffer[builder->length++] = ':';
        builder->buffer[builder->length] = '\0';
        return IJ_STATUS_OK;
    }

    if (ij_builder_append_json_string_known_escape(builder, key, len, true) != IJ_STATUS_OK ||
        ij_builder_append_char(builder, ':') != IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    return IJ_STATUS_OK;
}

static ij_static_json_text_t ij_level_to_json_literal(ij_level_t level)
{
    switch (level) {
    case IJ_LEVEL_TRACE:
        return (ij_static_json_text_t){.text = "\"trace\"", .len = sizeof("\"trace\"") - 1U};
    case IJ_LEVEL_DEBUG:
        return (ij_static_json_text_t){.text = "\"debug\"", .len = sizeof("\"debug\"") - 1U};
    case IJ_LEVEL_INFO:
        return (ij_static_json_text_t){.text = "\"info\"", .len = sizeof("\"info\"") - 1U};
    case IJ_LEVEL_WARN:
        return (ij_static_json_text_t){.text = "\"warn\"", .len = sizeof("\"warn\"") - 1U};
    case IJ_LEVEL_ERROR:
        return (ij_static_json_text_t){.text = "\"error\"", .len = sizeof("\"error\"") - 1U};
    case IJ_LEVEL_FATAL:
        return (ij_static_json_text_t){.text = "\"fatal\"", .len = sizeof("\"fatal\"") - 1U};
    default:
        return (ij_static_json_text_t){.text = "\"unknown\"", .len = sizeof("\"unknown\"") - 1U};
    }
}

static ij_status_t ij_builder_append_attr_value(ij_json_builder_t *builder,
                                                const ij_owned_attr_value_t *value)
{
    char number_buffer[64];

    switch (value->kind) {
    case IJ_ATTR_VALUE_STRING:
        return ij_builder_append_json_string_known_escape(builder, value->as.string.data,
                                                          value->as.string.len,
                                                          value->as.string.needs_json_escape);
    case IJ_ATTR_VALUE_SIGNED:
        return ij_builder_append_int64_decimal(builder, value->as.signed_value);
    case IJ_ATTR_VALUE_UNSIGNED:
        return ij_builder_append_uint64_decimal(builder, value->as.unsigned_value);
    case IJ_ATTR_VALUE_BOOL:
        return value->as.bool_value ? ij_builder_append_raw_n(builder, "true", 4U)
                                    : ij_builder_append_raw_n(builder, "false", 5U);
    case IJ_ATTR_VALUE_DOUBLE:
        if (snprintf(number_buffer, sizeof(number_buffer), "%.17g", value->as.double_value) >=
            (int)sizeof(number_buffer)) {
            return IJ_STATUS_ENCODE_ERROR;
        }
        return ij_builder_append_raw(builder, number_buffer);
    default:
        return IJ_STATUS_ENCODE_ERROR;
    }
}

ij_status_t ij_format_timestamp_rfc3339(ij_timestamp_t timestamp, char *buffer, size_t buffer_size)
{
    time_t seconds = (time_t)timestamp.unix_seconds;
    struct tm tm_utc;
    int year;
    uint32_t milliseconds;

    if (buffer == NULL || buffer_size < 25U || timestamp.nanoseconds >= 1000000000U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (gmtime_r(&seconds, &tm_utc) == NULL) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    year = tm_utc.tm_year + 1900;
    if (year < 0 || year > 9999) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    milliseconds = timestamp.nanoseconds / 1000000U;
    ij_write_four_digits(&buffer[0], (unsigned int)year);
    buffer[4] = '-';
    ij_write_two_digits(&buffer[5], (unsigned int)(tm_utc.tm_mon + 1));
    buffer[7] = '-';
    ij_write_two_digits(&buffer[8], (unsigned int)tm_utc.tm_mday);
    buffer[10] = 'T';
    ij_write_two_digits(&buffer[11], (unsigned int)tm_utc.tm_hour);
    buffer[13] = ':';
    ij_write_two_digits(&buffer[14], (unsigned int)tm_utc.tm_min);
    buffer[16] = ':';
    ij_write_two_digits(&buffer[17], (unsigned int)tm_utc.tm_sec);
    buffer[19] = '.';
    ij_write_three_digits(&buffer[20], milliseconds);
    buffer[23] = 'Z';
    buffer[24] = '\0';

    return IJ_STATUS_OK;
}

ij_status_t ij_json_console_encode(const ij_event_copy_t *event, char *buffer, size_t buffer_size,
                                   size_t *out_len)
{
    char timestamp[32];
    ij_static_json_text_t level_text;
    ij_json_builder_t builder = {
        .buffer = buffer,
        .capacity = buffer_size,
        .length = 0U,
    };

    if (event == NULL || buffer == NULL || out_len == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    buffer[0] = '\0';
    level_text = ij_level_to_json_literal(event->level);

    if (ij_format_timestamp_rfc3339(event->timestamp, timestamp, sizeof(timestamp)) !=
            IJ_STATUS_OK ||
        ij_builder_append_raw_n(&builder, "{\"timestamp\":\"", sizeof("{\"timestamp\":\"") - 1U) !=
            IJ_STATUS_OK ||
        ij_builder_append_raw_n(&builder, timestamp, sizeof("1970-01-01T00:00:00.123Z") - 1U) !=
            IJ_STATUS_OK ||
        ij_builder_append_raw_n(&builder, "\",\"level\":", sizeof("\",\"level\":") - 1U) !=
            IJ_STATUS_OK ||
        ij_builder_append_raw_n(&builder, level_text.text, level_text.len) != IJ_STATUS_OK ||
        ij_builder_append_raw_n(&builder, ",\"event_name\":", sizeof(",\"event_name\":") - 1U) !=
            IJ_STATUS_OK ||
        ij_builder_append_json_string_known_escape(
            &builder, event->event_name.data, event->event_name.len,
            event->event_name.needs_json_escape) != IJ_STATUS_OK ||
        ij_builder_append_raw_n(&builder, ",\"message\":", sizeof(",\"message\":") - 1U) !=
            IJ_STATUS_OK ||
        ij_builder_append_json_string_known_escape(
            &builder, event->message.data, event->message.len, event->message.needs_json_escape) !=
            IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    if (event->logger_name.data != NULL) {
        if (ij_builder_append_raw_n(&builder, ",\"logger\":", sizeof(",\"logger\":") - 1U) !=
                IJ_STATUS_OK ||
            ij_builder_append_json_string_known_escape(
                &builder, event->logger_name.data, event->logger_name.len,
                event->logger_name.needs_json_escape) != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }
    }

    if (event->attribute_count > 0U) {
        if (ij_builder_append_raw_n(&builder, ",\"attributes\":{",
                                    sizeof(",\"attributes\":{") - 1U) != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }

        for (size_t i = 0U; i < event->attribute_count; ++i) {
            if (i > 0U && ij_builder_append_char(&builder, ',') != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            if (ij_builder_append_json_key_known_escape(
                    &builder, event->attributes[i].key.data, event->attributes[i].key.len,
                    event->attributes[i].key.needs_json_escape) != IJ_STATUS_OK ||
                ij_builder_append_attr_value(&builder, &event->attributes[i].value) !=
                    IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
        }

        if (ij_builder_append_char(&builder, '}') != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }
    }

    if (ij_builder_append_char(&builder, '}') != IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    *out_len = builder.length;
    return IJ_STATUS_OK;
}
