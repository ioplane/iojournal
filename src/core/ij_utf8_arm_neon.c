/*
 * iojournal -- NEON UTF-8 validation fast path
 *
 * SPDX-License-Identifier: MIT
 */

#include "encoders/ij_simd_dispatch.h"

#if defined(__aarch64__)

#    include <arm_neon.h>

static bool ij_utf8_arm_neon_impl(const char *data, size_t len)
{
    size_t i = 0U;

    while (i + 16U <= len) {
        uint8x16_t chunk = vld1q_u8((const uint8_t *)(const void *)(data + i));
        uint8x16_t high = vandq_u8(chunk, vdupq_n_u8(0x80U));
        uint64x2_t wide = vreinterpretq_u64_u8(high);

        if (vgetq_lane_u64(wide, 0) != 0ULL || vgetq_lane_u64(wide, 1) != 0ULL) {
            break;
        }
        i += 16U;
    }

    return ij_utf8_scalar_validate(data + i, len - i);
}

#endif

bool ij_utf8_arm_neon_validate(const char *data, size_t len)
{
#if defined(__aarch64__)
    if (ij_simd_runtime_has_arm_neon()) {
        return ij_utf8_arm_neon_impl(data, len);
    }
#endif

    return ij_utf8_scalar_validate(data, len);
}
