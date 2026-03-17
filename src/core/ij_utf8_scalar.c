/*
 * iojournal -- scalar UTF-8 validation
 *
 * SPDX-License-Identifier: MIT
 */

#include "encoders/ij_simd_dispatch.h"

bool ij_utf8_scalar_validate(const char *data, size_t len)
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
            if (remaining < 2U || (((unsigned char)data[i + 1U]) & 0xc0U) != 0x80U || c < 0xc2U) {
                return false;
            }
            i += 2U;
            continue;
        }

        if ((c & 0xf0U) == 0xe0U) {
            if (remaining < 3U) {
                return false;
            }
            unsigned char c1 = (unsigned char)data[i + 1U];
            unsigned char c2 = (unsigned char)data[i + 2U];

            if ((c1 & 0xc0U) != 0x80U || (c2 & 0xc0U) != 0x80U) {
                return false;
            }
            if ((c == 0xe0U && c1 < 0xa0U) || (c == 0xedU && c1 >= 0xa0U)) {
                return false;
            }
            i += 3U;
            continue;
        }

        if ((c & 0xf8U) == 0xf0U) {
            if (remaining < 4U) {
                return false;
            }
            unsigned char c1 = (unsigned char)data[i + 1U];
            unsigned char c2 = (unsigned char)data[i + 2U];
            unsigned char c3 = (unsigned char)data[i + 3U];

            if ((c1 & 0xc0U) != 0x80U || (c2 & 0xc0U) != 0x80U || (c3 & 0xc0U) != 0x80U) {
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

bool ij_utf8_is_valid_dispatch(const char *data, size_t len)
{
    if (len < IJ_SIMD_UTF8_MIN_LEN) {
        return ij_utf8_scalar_validate(data, len);
    }

    switch (ij_simd_utf8_kind()) {
    case IJ_SIMD_KIND_X86_AVX2:
        return ij_utf8_x86_avx2_validate(data, len);
    case IJ_SIMD_KIND_ARM_NEON:
        return ij_utf8_arm_neon_validate(data, len);
    case IJ_SIMD_KIND_SCALAR:
    default:
        return ij_utf8_scalar_validate(data, len);
    }
}
