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

static ij_status_t ij_builder_append_raw(ij_json_builder_t *builder, const char *text)
{
    size_t len = strlen(text);

    if (builder->length + len + 1U > builder->capacity) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    memcpy(builder->buffer + builder->length, text, len);
    builder->length += len;
    builder->buffer[builder->length] = '\0';
    return IJ_STATUS_OK;
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

static ij_status_t ij_builder_append_escaped_string(ij_json_builder_t *builder, const char *text)
{
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor != '\0'; ++cursor) {
        switch (*cursor) {
        case '\\':
            if (ij_builder_append_raw(builder, "\\\\") != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '"':
            if (ij_builder_append_raw(builder, "\\\"") != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '\n':
            if (ij_builder_append_raw(builder, "\\n") != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '\r':
            if (ij_builder_append_raw(builder, "\\r") != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        case '\t':
            if (ij_builder_append_raw(builder, "\\t") != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        default:
            if (*cursor < 0x20U) {
                char escaped[7];

                if (snprintf(escaped, sizeof(escaped), "\\u%04x", *cursor) >=
                    (int)sizeof(escaped)) {
                    return IJ_STATUS_ENCODE_ERROR;
                }
                if (ij_builder_append_raw(builder, escaped) != IJ_STATUS_OK) {
                    return IJ_STATUS_ENCODE_ERROR;
                }
            } else if (ij_builder_append_char(builder, (char)*cursor) != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            break;
        }
    }

    return IJ_STATUS_OK;
}

static ij_status_t ij_builder_append_json_string(ij_json_builder_t *builder, const char *text)
{
    if (ij_builder_append_char(builder, '"') != IJ_STATUS_OK ||
        ij_builder_append_escaped_string(builder, text) != IJ_STATUS_OK ||
        ij_builder_append_char(builder, '"') != IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    return IJ_STATUS_OK;
}

static ij_status_t ij_builder_append_json_key(ij_json_builder_t *builder, const char *key)
{
    if (ij_builder_append_json_string(builder, key) != IJ_STATUS_OK ||
        ij_builder_append_char(builder, ':') != IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    return IJ_STATUS_OK;
}

static ij_status_t ij_builder_append_attr_value(ij_json_builder_t *builder,
                                                const ij_owned_attr_value_t *value)
{
    char number_buffer[64];

    switch (value->kind) {
    case IJ_ATTR_VALUE_STRING:
        return ij_builder_append_json_string(builder, value->as.string.data);
    case IJ_ATTR_VALUE_SIGNED:
        if (snprintf(number_buffer, sizeof(number_buffer), "%lld",
                     (long long)value->as.signed_value) >= (int)sizeof(number_buffer)) {
            return IJ_STATUS_ENCODE_ERROR;
        }
        return ij_builder_append_raw(builder, number_buffer);
    case IJ_ATTR_VALUE_UNSIGNED:
        if (snprintf(number_buffer, sizeof(number_buffer), "%llu",
                     (unsigned long long)value->as.unsigned_value) >= (int)sizeof(number_buffer)) {
            return IJ_STATUS_ENCODE_ERROR;
        }
        return ij_builder_append_raw(builder, number_buffer);
    case IJ_ATTR_VALUE_BOOL:
        return ij_builder_append_raw(builder, value->as.bool_value ? "true" : "false");
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
    uint32_t milliseconds;

    if (buffer == NULL || buffer_size < 25U || timestamp.nanoseconds >= 1000000000U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (gmtime_r(&seconds, &tm_utc) == NULL) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    milliseconds = timestamp.nanoseconds / 1000000U;
    if (snprintf(buffer, buffer_size, "%04d-%02d-%02dT%02d:%02d:%02d.%03uZ", tm_utc.tm_year + 1900,
                 tm_utc.tm_mon + 1, tm_utc.tm_mday, tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec,
                 milliseconds) >= (int)buffer_size) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    return IJ_STATUS_OK;
}

ij_status_t ij_json_console_encode(const ij_event_copy_t *event, char *buffer, size_t buffer_size,
                                   size_t *out_len)
{
    char timestamp[32];
    ij_json_builder_t builder = {
        .buffer = buffer,
        .capacity = buffer_size,
        .length = 0U,
    };

    if (event == NULL || buffer == NULL || out_len == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    buffer[0] = '\0';

    if (ij_format_timestamp_rfc3339(event->timestamp, timestamp, sizeof(timestamp)) !=
            IJ_STATUS_OK ||
        ij_builder_append_char(&builder, '{') != IJ_STATUS_OK ||
        ij_builder_append_json_key(&builder, "timestamp") != IJ_STATUS_OK ||
        ij_builder_append_json_string(&builder, timestamp) != IJ_STATUS_OK ||
        ij_builder_append_char(&builder, ',') != IJ_STATUS_OK ||
        ij_builder_append_json_key(&builder, "level") != IJ_STATUS_OK ||
        ij_builder_append_json_string(&builder, ij_level_to_text(event->level)) != IJ_STATUS_OK ||
        ij_builder_append_char(&builder, ',') != IJ_STATUS_OK ||
        ij_builder_append_json_key(&builder, "event_name") != IJ_STATUS_OK ||
        ij_builder_append_json_string(&builder, event->event_name.data) != IJ_STATUS_OK ||
        ij_builder_append_char(&builder, ',') != IJ_STATUS_OK ||
        ij_builder_append_json_key(&builder, "message") != IJ_STATUS_OK ||
        ij_builder_append_json_string(&builder, event->message.data) != IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    if (event->logger_name.data != NULL) {
        if (ij_builder_append_char(&builder, ',') != IJ_STATUS_OK ||
            ij_builder_append_json_key(&builder, "logger") != IJ_STATUS_OK ||
            ij_builder_append_json_string(&builder, event->logger_name.data) != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }
    }

    if (event->attribute_count > 0U) {
        if (ij_builder_append_char(&builder, ',') != IJ_STATUS_OK ||
            ij_builder_append_json_key(&builder, "attributes") != IJ_STATUS_OK ||
            ij_builder_append_char(&builder, '{') != IJ_STATUS_OK) {
            return IJ_STATUS_ENCODE_ERROR;
        }

        for (size_t i = 0U; i < event->attribute_count; ++i) {
            if (i > 0U && ij_builder_append_char(&builder, ',') != IJ_STATUS_OK) {
                return IJ_STATUS_ENCODE_ERROR;
            }
            if (ij_builder_append_json_key(&builder, event->attributes[i].key.data) !=
                    IJ_STATUS_OK ||
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
