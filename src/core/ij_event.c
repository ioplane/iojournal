/*
 * iojournal -- event validation and copy helpers
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define IJ_REDACTED_LITERAL "[REDACTED]"

static bool ij_utf8_is_valid(const char *data, size_t len)
{
    size_t i = 0U;

    while (i < len) {
        unsigned char c = (unsigned char)data[i];
        size_t remaining = len - i;

        if (c <= 0x7fU) {
            i += 1U;
            continue;
        }

        if ((c & 0xe0U) == 0xc0U) {
            if (remaining < 2U || (data[i + 1U] & 0xc0) != 0x80 || c < 0xc2U) {
                return false;
            }
            i += 2U;
            continue;
        }

        if ((c & 0xf0U) == 0xe0U) {
            unsigned char c1 = (unsigned char)data[i + 1U];
            unsigned char c2 = (unsigned char)data[i + 2U];

            if (remaining < 3U || (c1 & 0xc0U) != 0x80U || (c2 & 0xc0U) != 0x80U) {
                return false;
            }
            if ((c == 0xe0U && c1 < 0xa0U) || (c == 0xedU && c1 >= 0xa0U)) {
                return false;
            }
            i += 3U;
            continue;
        }

        if ((c & 0xf8U) == 0xf0U) {
            unsigned char c1 = (unsigned char)data[i + 1U];
            unsigned char c2 = (unsigned char)data[i + 2U];
            unsigned char c3 = (unsigned char)data[i + 3U];

            if (remaining < 4U || (c1 & 0xc0U) != 0x80U || (c2 & 0xc0U) != 0x80U ||
                (c3 & 0xc0U) != 0x80U) {
                return false;
            }
            if ((c == 0xf0U && c1 < 0x90U) || (c == 0xf4U && c1 >= 0x90U) || c > 0xf4U) {
                return false;
            }
            i += 4U;
            continue;
        }

        return false;
    }

    return true;
}

static bool ij_validate_string_field(const char *value, size_t max_len, bool allow_empty)
{
    size_t len;

    if (value == NULL) {
        return false;
    }

    len = strlen(value);
    if ((!allow_empty && len == 0U) || len > max_len) {
        return false;
    }

    return ij_utf8_is_valid(value, len);
}

static bool ij_validate_attr_value(const ij_attr_value_t *value)
{
    if (value == NULL) {
        return false;
    }

    switch (value->kind) {
    case IJ_ATTR_VALUE_STRING:
        if (value->as.string.data == NULL || value->as.string.len > IJ_ATTRIBUTE_STRING_MAX_LEN) {
            return false;
        }
        return ij_utf8_is_valid(value->as.string.data, value->as.string.len);
    case IJ_ATTR_VALUE_SIGNED:
    case IJ_ATTR_VALUE_UNSIGNED:
    case IJ_ATTR_VALUE_BOOL:
        return true;
    case IJ_ATTR_VALUE_DOUBLE:
        return isfinite(value->as.double_value);
    default:
        return false;
    }
}

static bool ij_duplicate_attr_keys(const ij_attr_t *attributes, size_t attribute_count)
{
    for (size_t i = 0U; i < attribute_count; ++i) {
        for (size_t j = i + 1U; j < attribute_count; ++j) {
            if (strcmp(attributes[i].key, attributes[j].key) == 0) {
                return true;
            }
        }
    }

    return false;
}

static size_t ij_event_text_size(const ij_event_t *event)
{
    size_t total = strlen(event->event_name) + strlen(event->message);

    if (event->logger != NULL) {
        total += strlen(event->logger);
    }
    if (event->source_file != NULL) {
        total += strlen(event->source_file);
    }
    if (event->source_function != NULL) {
        total += strlen(event->source_function);
    }

    for (size_t i = 0U; i < event->attribute_count; ++i) {
        const ij_attr_t *attribute = &event->attributes[i];

        total += strlen(attribute->key);
        if (attribute->value.kind == IJ_ATTR_VALUE_STRING) {
            total += attribute->value.as.string.len;
        }
    }

    return total;
}

static ij_status_t ij_copy_owned_string(ij_owned_string_t *out_string, const char *value,
                                        size_t len)
{
    char *buffer;

    out_string->data = NULL;
    out_string->len = 0U;

    buffer = malloc(len + 1U);
    if (buffer == NULL) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    memcpy(buffer, value, len);
    buffer[len] = '\0';
    out_string->data = buffer;
    out_string->len = len;
    return IJ_STATUS_OK;
}

static ij_status_t ij_copy_attr_value(ij_owned_attr_value_t *out_value, const ij_attr_t *attribute)
{
    out_value->kind = attribute->value.kind;

    switch (attribute->value.kind) {
    case IJ_ATTR_VALUE_STRING:
        return ij_copy_owned_string(&out_value->as.string, attribute->value.as.string.data,
                                    attribute->value.as.string.len);
    case IJ_ATTR_VALUE_SIGNED:
        out_value->as.signed_value = attribute->value.as.signed_value;
        return IJ_STATUS_OK;
    case IJ_ATTR_VALUE_UNSIGNED:
        out_value->as.unsigned_value = attribute->value.as.unsigned_value;
        return IJ_STATUS_OK;
    case IJ_ATTR_VALUE_BOOL:
        out_value->as.bool_value = attribute->value.as.bool_value;
        return IJ_STATUS_OK;
    case IJ_ATTR_VALUE_DOUBLE:
        out_value->as.double_value = attribute->value.as.double_value;
        return IJ_STATUS_OK;
    default:
        return IJ_STATUS_INVALID_ARGUMENT;
    }
}

ij_status_t ij_validate_event(const ij_event_t *event)
{
    if (event == NULL || !ij_level_is_valid(event->level)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->timestamp.nanoseconds >= 1000000000U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (!ij_validate_string_field(event->event_name, IJ_EVENT_NAME_MAX_LEN, false) ||
        !ij_validate_string_field(event->message, IJ_MESSAGE_MAX_LEN, true)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->logger != NULL &&
        !ij_validate_string_field(event->logger, IJ_LOGGER_NAME_MAX_LEN, false)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->source_file != NULL &&
        !ij_validate_string_field(event->source_file, IJ_EVENT_TEXT_MAX_LEN, false)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->source_function != NULL &&
        !ij_validate_string_field(event->source_function, IJ_EVENT_TEXT_MAX_LEN, false)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->attribute_count > IJ_ATTRIBUTE_COUNT_MAX) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->attribute_count > 0U && event->attributes == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (event->has_trace_context && (event->trace_id == NULL || event->span_id == NULL)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    for (size_t i = 0U; i < event->attribute_count; ++i) {
        const ij_attr_t *attribute = &event->attributes[i];

        if (!ij_validate_string_field(attribute->key, IJ_ATTRIBUTE_KEY_MAX_LEN, false) ||
            !ij_validate_attr_value(&attribute->value)) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
    }

    if (ij_duplicate_attr_keys(event->attributes, event->attribute_count) ||
        ij_event_text_size(event) > IJ_EVENT_TEXT_MAX_LEN) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    return IJ_STATUS_OK;
}

void ij_event_copy_dispose(ij_event_copy_t *event)
{
    if (event == NULL) {
        return;
    }

    free(event->event_name.data);
    free(event->message.data);
    free(event->logger_name.data);
    free(event->source_file.data);
    free(event->source_function.data);

    if (event->attributes != NULL) {
        for (size_t i = 0U; i < event->attribute_count; ++i) {
            free(event->attributes[i].key.data);
            if (event->attributes[i].value.kind == IJ_ATTR_VALUE_STRING) {
                free(event->attributes[i].value.as.string.data);
            }
        }
    }

    free(event->attributes);
    memset(event, 0, sizeof(*event));
}

ij_status_t ij_event_copy_from_input(ij_event_copy_t *out_event, const ij_event_t *event,
                                     ij_redaction_mode_t redaction_mode)
{
    ij_status_t status;

    if (out_event == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    memset(out_event, 0, sizeof(*out_event));

    status = ij_validate_event(event);
    if (status != IJ_STATUS_OK) {
        return status;
    }

    out_event->timestamp = event->timestamp;
    out_event->level = event->level;
    out_event->trace_flags = event->trace_flags;
    out_event->has_trace_context = event->has_trace_context;
    out_event->source_line = event->source_line;

    if (event->has_trace_context) {
        memcpy(out_event->trace_id, event->trace_id, IJ_TRACE_ID_SIZE);
        memcpy(out_event->span_id, event->span_id, IJ_SPAN_ID_SIZE);
    }

    status =
        ij_copy_owned_string(&out_event->event_name, event->event_name, strlen(event->event_name));
    if (status != IJ_STATUS_OK) {
        goto fail;
    }
    status = ij_copy_owned_string(&out_event->message, event->message, strlen(event->message));
    if (status != IJ_STATUS_OK) {
        goto fail;
    }
    if (event->logger != NULL) {
        status =
            ij_copy_owned_string(&out_event->logger_name, event->logger, strlen(event->logger));
        if (status != IJ_STATUS_OK) {
            goto fail;
        }
    }
    if (event->source_file != NULL) {
        status = ij_copy_owned_string(&out_event->source_file, event->source_file,
                                      strlen(event->source_file));
        if (status != IJ_STATUS_OK) {
            goto fail;
        }
    }
    if (event->source_function != NULL) {
        status = ij_copy_owned_string(&out_event->source_function, event->source_function,
                                      strlen(event->source_function));
        if (status != IJ_STATUS_OK) {
            goto fail;
        }
    }

    if (event->attribute_count > 0U) {
        out_event->attributes = calloc(event->attribute_count, sizeof(*out_event->attributes));
        if (out_event->attributes == NULL) {
            status = IJ_STATUS_INTERNAL_ERROR;
            goto fail;
        }

        for (size_t i = 0U; i < event->attribute_count; ++i) {
            const ij_attr_t *attribute = &event->attributes[i];

            status = ij_copy_owned_string(&out_event->attributes[i].key, attribute->key,
                                          strlen(attribute->key));
            if (status != IJ_STATUS_OK) {
                goto fail;
            }

            status = ij_copy_attr_value(&out_event->attributes[i].value, attribute);
            if (status != IJ_STATUS_OK) {
                goto fail;
            }

            if (redaction_mode == IJ_REDACTION_MODE_ENABLED) {
                ij_redact_owned_attr_value(attribute->key, &out_event->attributes[i].value);
            }
        }

        out_event->attribute_count = event->attribute_count;
    }

    return IJ_STATUS_OK;

fail:
    ij_event_copy_dispose(out_event);
    return status;
}
