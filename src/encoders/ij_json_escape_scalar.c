/*
 * iojournal -- scalar JSON escape scanning
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_simd_dispatch.h"

size_t ij_json_escape_scalar_find_first_special(const unsigned char *text, size_t len)
{
    size_t i;

    for (i = 0U; i < len; ++i) {
        unsigned char c = text[i];

        if (c == '\\' || c == '"' || c == '\n' || c == '\r' || c == '\t' || c < 0x20U) {
            return i;
        }
    }

    return len;
}

size_t ij_json_escape_find_first_special(const unsigned char *text, size_t len)
{
    if (len < IJ_SIMD_JSON_ESCAPE_MIN_LEN) {
        return ij_json_escape_scalar_find_first_special(text, len);
    }

    switch (ij_simd_json_escape_kind()) {
    case IJ_SIMD_KIND_X86_AVX2:
        return ij_json_escape_x86_avx2_find_first_special(text, len);
    case IJ_SIMD_KIND_ARM_NEON:
        return ij_json_escape_arm_neon_find_first_special(text, len);
    case IJ_SIMD_KIND_SCALAR:
    default:
        return ij_json_escape_scalar_find_first_special(text, len);
    }
}
