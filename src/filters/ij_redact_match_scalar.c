/*
 * iojournal -- scalar redaction matching
 *
 * SPDX-License-Identifier: MIT
 */

#include "encoders/ij_simd_dispatch.h"

#include <ctype.h>

bool ij_redact_case_equal_scalar(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len)
{
    size_t i;

    if (lhs_len != rhs_len) {
        return false;
    }

    for (i = 0U; i < lhs_len; ++i) {
        if (tolower((unsigned char)lhs[i]) != (unsigned char)rhs[i]) {
            return false;
        }
    }

    return true;
}

bool ij_redact_has_dot_scalar(const char *text, size_t len)
{
    size_t i;

    for (i = 0U; i < len; ++i) {
        if (text[i] == '.') {
            return true;
        }
    }

    return false;
}

bool ij_redact_case_equal(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len)
{
    return ij_redact_case_equal_scalar(lhs, lhs_len, rhs, rhs_len);
}

bool ij_redact_has_dot(const char *text, size_t len)
{
    return ij_redact_has_dot_scalar(text, len);
}
