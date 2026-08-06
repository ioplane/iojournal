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

static bool ij_validate_string_field(const char *value, size_t max_len, bool allow_empty)
{
    size_t len;

    if (value == NULL) {
        return false;
    }

    len = strnlen(value, max_len + 1U);
    if ((!allow_empty && len == 0U) || len > max_len) {
        return false;
    }

    return ij_utf8_is_valid_dispatch(value, len);
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
        return ij_utf8_is_valid_dispatch(value->as.string.data, value->as.string.len);
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

static size_t ij_max_size(size_t left, size_t right)
{
    return left > right ? left : right;
}

static bool ij_ascii_needs_json_escape(unsigned char c)
{
    return c < 0x20U || c == '"' || c == '\\';
}

static bool ij_scan_string_field_copy(const char *value, size_t max_len, bool allow_empty,
                                      size_t *out_len, bool *out_needs_json_escape)
{
    bool needs_json_escape = false;

    if (value == NULL || out_len == NULL || out_needs_json_escape == NULL) {
        return false;
    }

    for (size_t i = 0U; i <= max_len; ++i) {
        unsigned char c = (unsigned char)value[i];

        if (c == '\0') {
            if (!allow_empty && i == 0U) {
                return false;
            }
            *out_len = i;
            *out_needs_json_escape = needs_json_escape;
            return true;
        }
        if ((c & 0x80U) != 0U) {
            size_t len = strnlen(value, max_len + 1U);

            if ((!allow_empty && len == 0U) || len > max_len) {
                return false;
            }
            if (!ij_utf8_is_valid_dispatch(value, len)) {
                return false;
            }

            *out_len = len;
            *out_needs_json_escape =
                ij_json_escape_find_first_special((const unsigned char *)value, len) != len;
            return true;
        }
        if (!needs_json_escape) {
            needs_json_escape = ij_ascii_needs_json_escape(c);
        }
    }

    return false;
}

static bool ij_validate_attr_string_copy(const char *value, size_t len, bool *out_needs_json_escape)
{
    bool needs_json_escape = false;

    if (value == NULL || out_needs_json_escape == NULL || len > IJ_ATTRIBUTE_STRING_MAX_LEN) {
        return false;
    }

    for (size_t i = 0U; i < len; ++i) {
        unsigned char c = (unsigned char)value[i];

        if ((c & 0x80U) != 0U) {
            if (!ij_utf8_is_valid_dispatch(value, len)) {
                return false;
            }

            *out_needs_json_escape =
                ij_json_escape_find_first_special((const unsigned char *)value, len) != len;
            return true;
        }
        if (!needs_json_escape) {
            needs_json_escape = ij_ascii_needs_json_escape(c);
        }
    }

    *out_needs_json_escape = needs_json_escape;
    return true;
}

static bool ij_attr_keys_equal(const char *left, size_t left_len, const char *right,
                               size_t right_len)
{
    return left_len == right_len && memcmp(left, right, left_len) == 0;
}

static ij_status_t ij_copy_owned_string_into_arena(ij_owned_string_t *out_string, const char *value,
                                                   size_t len, size_t capacity,
                                                   bool needs_json_escape, char **cursor)
{
    char *buffer = *cursor;

    memcpy(buffer, value, len);
    buffer[len] = '\0';

    out_string->data = buffer;
    out_string->len = len;
    out_string->needs_json_escape = needs_json_escape;
    *cursor += capacity + 1U;
    return IJ_STATUS_OK;
}

static ij_status_t ij_copy_attr_value_into_arena(ij_owned_attr_value_t *out_value,
                                                 const ij_attr_t *attribute, bool needs_json_escape,
                                                 char **cursor)
{
    out_value->kind = attribute->value.kind;

    switch (attribute->value.kind) {
    case IJ_ATTR_VALUE_STRING: {
        size_t capacity =
            ij_max_size(attribute->value.as.string.len, (size_t)(sizeof(IJ_REDACTED_LITERAL) - 1U));

        return ij_copy_owned_string_into_arena(&out_value->as.string,
                                               attribute->value.as.string.data,
                                               attribute->value.as.string.len, capacity,
                                               needs_json_escape, cursor);
    }
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

    free(event->text_arena);
    free(event->attributes);
    memset(event, 0, sizeof(*event));
}

ij_status_t ij_event_copy_from_input(ij_event_copy_t *out_event, const ij_event_t *event,
                                     ij_redaction_mode_t redaction_mode)
{
    const char *attr_keys[IJ_ATTRIBUTE_COUNT_MAX];
    size_t attr_key_lens[IJ_ATTRIBUTE_COUNT_MAX];
    bool attr_key_needs_json_escape[IJ_ATTRIBUTE_COUNT_MAX];
    bool attr_string_needs_json_escape[IJ_ATTRIBUTE_COUNT_MAX];
    bool attr_should_redact[IJ_ATTRIBUTE_COUNT_MAX];
    char *cursor = NULL;
    size_t total_text_size = 0U;
    size_t event_name_len = 0U;
    size_t message_len = 0U;
    size_t logger_len = 0U;
    size_t source_file_len = 0U;
    size_t source_function_len = 0U;
    ij_status_t status;

    if (out_event == NULL || event == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    memset(out_event, 0, sizeof(*out_event));

    if (!ij_level_is_valid(event->level) || event->timestamp.nanoseconds >= 1000000000U ||
        event->attribute_count > IJ_ATTRIBUTE_COUNT_MAX ||
        (event->attribute_count > 0U && event->attributes == NULL) ||
        (event->has_trace_context && (event->trace_id == NULL || event->span_id == NULL))) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (!ij_scan_string_field_copy(event->event_name, IJ_EVENT_NAME_MAX_LEN, false, &event_name_len,
                                   &out_event->event_name.needs_json_escape) ||
        !ij_scan_string_field_copy(event->message, IJ_MESSAGE_MAX_LEN, true, &message_len,
                                   &out_event->message.needs_json_escape)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    total_text_size += event_name_len + message_len;

    if (event->logger != NULL) {
        if (!ij_scan_string_field_copy(event->logger, IJ_LOGGER_NAME_MAX_LEN, false, &logger_len,
                                       &out_event->logger_name.needs_json_escape)) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
        total_text_size += logger_len;
    }
    if (event->source_file != NULL) {
        if (!ij_scan_string_field_copy(event->source_file, IJ_EVENT_TEXT_MAX_LEN, false,
                                       &source_file_len,
                                       &out_event->source_file.needs_json_escape)) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
        total_text_size += source_file_len;
    }
    if (event->source_function != NULL) {
        if (!ij_scan_string_field_copy(event->source_function, IJ_EVENT_TEXT_MAX_LEN, false,
                                       &source_function_len,
                                       &out_event->source_function.needs_json_escape)) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
        total_text_size += source_function_len;
    }
    if (total_text_size > IJ_EVENT_TEXT_MAX_LEN) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    out_event->text_arena_size = event_name_len + 1U + message_len + 1U;
    if (event->logger != NULL) {
        out_event->text_arena_size += logger_len + 1U;
    }
    if (event->source_file != NULL) {
        out_event->text_arena_size += source_file_len + 1U;
    }
    if (event->source_function != NULL) {
        out_event->text_arena_size += source_function_len + 1U;
    }

    for (size_t i = 0U; i < event->attribute_count; ++i) {
        const ij_attr_t *attribute = &event->attributes[i];
        size_t key_len = 0U;

        if (!ij_scan_string_field_copy(attribute->key, IJ_ATTRIBUTE_KEY_MAX_LEN, false, &key_len,
                                       &attr_key_needs_json_escape[i])) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
        if (attribute->value.kind == IJ_ATTR_VALUE_STRING) {
            if (!ij_validate_attr_string_copy(attribute->value.as.string.data,
                                              attribute->value.as.string.len,
                                              &attr_string_needs_json_escape[i])) {
                return IJ_STATUS_INVALID_ARGUMENT;
            }
        } else if (!ij_validate_attr_value(&attribute->value)) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }

        for (size_t j = 0U; j < i; ++j) {
            if (ij_attr_keys_equal(attribute->key, key_len, attr_keys[j], attr_key_lens[j])) {
                return IJ_STATUS_INVALID_ARGUMENT;
            }
        }

        attr_keys[i] = attribute->key;
        attr_key_lens[i] = key_len;
        attr_should_redact[i] = redaction_mode == IJ_REDACTION_MODE_ENABLED &&
                                attribute->value.kind == IJ_ATTR_VALUE_STRING &&
                                ij_key_should_redact_n(attribute->key, key_len);
        total_text_size += key_len;
        out_event->text_arena_size += key_len + 1U;

        if (attribute->value.kind == IJ_ATTR_VALUE_STRING) {
            size_t capacity = ij_max_size(attribute->value.as.string.len,
                                          (size_t)(sizeof(IJ_REDACTED_LITERAL) - 1U));

            total_text_size += attribute->value.as.string.len;
            out_event->text_arena_size += capacity + 1U;
        }

        if (total_text_size > IJ_EVENT_TEXT_MAX_LEN) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
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

    out_event->text_arena = malloc(out_event->text_arena_size);
    if (out_event->text_arena == NULL) {
        status = IJ_STATUS_INTERNAL_ERROR;
        goto fail;
    }
    cursor = out_event->text_arena;

    status = ij_copy_owned_string_into_arena(&out_event->event_name, event->event_name,
                                             event_name_len, event_name_len,
                                             out_event->event_name.needs_json_escape, &cursor);
    if (status != IJ_STATUS_OK) {
        goto fail;
    }
    status = ij_copy_owned_string_into_arena(&out_event->message, event->message, message_len,
                                             message_len, out_event->message.needs_json_escape,
                                             &cursor);
    if (status != IJ_STATUS_OK) {
        goto fail;
    }
    if (event->logger != NULL) {
        status = ij_copy_owned_string_into_arena(&out_event->logger_name, event->logger, logger_len,
                                                 logger_len,
                                                 out_event->logger_name.needs_json_escape, &cursor);
        if (status != IJ_STATUS_OK) {
            goto fail;
        }
    }
    if (event->source_file != NULL) {
        status = ij_copy_owned_string_into_arena(&out_event->source_file, event->source_file,
                                                 source_file_len, source_file_len,
                                                 out_event->source_file.needs_json_escape, &cursor);
        if (status != IJ_STATUS_OK) {
            goto fail;
        }
    }
    if (event->source_function != NULL) {
        status = ij_copy_owned_string_into_arena(
            &out_event->source_function, event->source_function, source_function_len,
            source_function_len, out_event->source_function.needs_json_escape, &cursor);
        if (status != IJ_STATUS_OK) {
            goto fail;
        }
    }

    if (event->attribute_count > 0U) {
        out_event->attributes = malloc(event->attribute_count * sizeof(*out_event->attributes));
        if (out_event->attributes == NULL) {
            status = IJ_STATUS_INTERNAL_ERROR;
            goto fail;
        }

        for (size_t i = 0U; i < event->attribute_count; ++i) {
            const ij_attr_t *attribute = &event->attributes[i];

            status = ij_copy_owned_string_into_arena(&out_event->attributes[i].key, attribute->key,
                                                     attr_key_lens[i], attr_key_lens[i],
                                                     attr_key_needs_json_escape[i], &cursor);
            if (status != IJ_STATUS_OK) {
                goto fail;
            }

            status = ij_copy_attr_value_into_arena(&out_event->attributes[i].value, attribute,
                                                   attr_string_needs_json_escape[i], &cursor);
            if (status != IJ_STATUS_OK) {
                goto fail;
            }

            if (attr_should_redact[i]) {
                memcpy(out_event->attributes[i].value.as.string.data, IJ_REDACTED_LITERAL,
                       sizeof(IJ_REDACTED_LITERAL));
                out_event->attributes[i].value.as.string.len = sizeof(IJ_REDACTED_LITERAL) - 1U;
            }
        }

        out_event->attribute_count = event->attribute_count;
    }

    return IJ_STATUS_OK;

fail:
    ij_event_copy_dispose(out_event);
    return status;
}
