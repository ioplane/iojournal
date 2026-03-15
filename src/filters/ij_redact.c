/*
 * iojournal -- redaction helpers
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define IJ_REDACTED_LITERAL "[REDACTED]"

static bool ij_ascii_case_equal(const char *lhs, const char *rhs)
{
    while (*lhs != '\0' && *rhs != '\0') {
        if (tolower((unsigned char)*lhs) != tolower((unsigned char)*rhs)) {
            return false;
        }
        ++lhs;
        ++rhs;
    }

    return *lhs == '\0' && *rhs == '\0';
}

static bool ij_ascii_case_ends_with(const char *value, const char *suffix)
{
    size_t value_len = strlen(value);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > value_len) {
        return false;
    }

    return ij_ascii_case_equal(value + (value_len - suffix_len), suffix);
}

bool ij_key_should_redact(const char *key)
{
    static const char *const redact_keys[] = {
        "password",   "passwd",        "passphrase",    "secret",      "client_secret",
        "token",      "access_token",  "refresh_token", "id_token",    "api_key",
        "apikey",     "authorization", "session_id",    "session",     "cookie",
        "set_cookie", "private_key",   "secret_key",    "signing_key",
    };

    if (key == NULL) {
        return false;
    }

    for (size_t i = 0U; i < (sizeof(redact_keys) / sizeof(redact_keys[0])); ++i) {
        if (ij_ascii_case_equal(key, redact_keys[i])) {
            return true;
        }
        if (ij_ascii_case_ends_with(key, redact_keys[i]) && strchr(key, '.') != NULL) {
            return true;
        }
    }

    return false;
}

void ij_redact_owned_attr_value(const char *key, ij_owned_attr_value_t *value)
{
    char *replacement;

    if (!ij_key_should_redact(key) || value == NULL || value->kind != IJ_ATTR_VALUE_STRING) {
        return;
    }

    replacement = malloc(sizeof(IJ_REDACTED_LITERAL));
    if (replacement == NULL) {
        return;
    }

    memcpy(replacement, IJ_REDACTED_LITERAL, sizeof(IJ_REDACTED_LITERAL));
    free(value->as.string.data);
    value->as.string.data = replacement;
    value->as.string.len = sizeof(IJ_REDACTED_LITERAL) - 1U;
}
