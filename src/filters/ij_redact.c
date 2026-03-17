/*
 * iojournal -- redaction helpers
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <string.h>

static bool ij_redact_match_known_key(const char *key, size_t key_len)
{
    switch (key_len) {
    case 5U:
        return ij_redact_case_equal_scalar(key, key_len, "token", 5U);
    case 6U:
        switch (key[0] | 0x20) {
        case 'a':
            return ij_redact_case_equal_scalar(key, key_len, "apikey", 6U);
        case 'c':
            return ij_redact_case_equal_scalar(key, key_len, "cookie", 6U);
        case 'p':
            return ij_redact_case_equal_scalar(key, key_len, "passwd", 6U);
        case 's':
            return ij_redact_case_equal_scalar(key, key_len, "secret", 6U);
        default:
            return false;
        }
    case 7U:
        switch (key[0] | 0x20) {
        case 'a':
            return ij_redact_case_equal_scalar(key, key_len, "api_key", 7U);
        case 's':
            return ij_redact_case_equal_scalar(key, key_len, "session", 7U);
        default:
            return false;
        }
    case 8U:
        switch (key[0] | 0x20) {
        case 'i':
            return ij_redact_case_equal_scalar(key, key_len, "id_token", 8U);
        case 'p':
            return ij_redact_case_equal_scalar(key, key_len, "password", 8U);
        default:
            return false;
        }
    case 10U:
        switch (key[0] | 0x20) {
        case 'p':
            return ij_redact_case_equal_scalar(key, key_len, "passphrase", 10U);
        case 's':
            return ij_redact_case_equal_scalar(key, key_len, "session_id", 10U) ||
                   ij_redact_case_equal_scalar(key, key_len, "set_cookie", 10U) ||
                   ij_redact_case_equal_scalar(key, key_len, "secret_key", 10U);
        default:
            return false;
        }
    case 11U:
        switch (key[0] | 0x20) {
        case 'p':
            return ij_redact_case_equal_scalar(key, key_len, "private_key", 11U);
        case 's':
            return ij_redact_case_equal_scalar(key, key_len, "signing_key", 11U);
        default:
            return false;
        }
    case 12U:
        return ij_redact_case_equal_scalar(key, key_len, "access_token", 12U);
    case 13U:
        switch (key[0] | 0x20) {
        case 'a':
            return ij_redact_case_equal_scalar(key, key_len, "authorization", 13U);
        case 'c':
            return ij_redact_case_equal_scalar(key, key_len, "client_secret", 13U);
        case 'r':
            return ij_redact_case_equal_scalar(key, key_len, "refresh_token", 13U);
        default:
            return false;
        }
    default:
        return false;
    }
}

static bool ij_redact_match_suffix(const char *key, size_t key_len)
{
    return (key_len > 13U && ij_redact_match_known_key(key + (key_len - 13U), 13U)) ||
           (key_len > 12U && ij_redact_match_known_key(key + (key_len - 12U), 12U)) ||
           (key_len > 11U && ij_redact_match_known_key(key + (key_len - 11U), 11U)) ||
           (key_len > 10U && ij_redact_match_known_key(key + (key_len - 10U), 10U)) ||
           (key_len > 8U && ij_redact_match_known_key(key + (key_len - 8U), 8U)) ||
           (key_len > 7U && ij_redact_match_known_key(key + (key_len - 7U), 7U)) ||
           (key_len > 6U && ij_redact_match_known_key(key + (key_len - 6U), 6U)) ||
           (key_len > 5U && ij_redact_match_known_key(key + (key_len - 5U), 5U));
}

bool ij_key_should_redact_n(const char *key, size_t key_len)
{
    if (key == NULL) {
        return false;
    }

    return ij_redact_match_known_key(key, key_len) ||
           (ij_redact_has_dot_scalar(key, key_len) && ij_redact_match_suffix(key, key_len));
}

bool ij_key_should_redact(const char *key)
{
    if (key == NULL) {
        return false;
    }

    return ij_key_should_redact_n(key, strlen(key));
}

void ij_redact_owned_attr_value_n(const char *key, size_t key_len, ij_owned_attr_value_t *value)
{
    if (value == NULL || value->kind != IJ_ATTR_VALUE_STRING || value->as.string.data == NULL) {
        return;
    }
    if (!ij_key_should_redact_n(key, key_len)) {
        return;
    }

    memcpy(value->as.string.data, IJ_REDACTED_LITERAL, sizeof(IJ_REDACTED_LITERAL));
    value->as.string.len = sizeof(IJ_REDACTED_LITERAL) - 1U;
}

void ij_redact_owned_attr_value(const char *key, ij_owned_attr_value_t *value)
{
    if (key == NULL) {
        return;
    }

    ij_redact_owned_attr_value_n(key, strlen(key), value);
}
