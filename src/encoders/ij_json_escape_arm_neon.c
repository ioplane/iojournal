/*
 * iojournal -- NEON JSON escape scanning
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_simd_dispatch.h"

#if defined(__aarch64__)

#    include <arm_neon.h>

static size_t ij_json_escape_arm_neon_impl(const unsigned char *text, size_t len)
{
    const uint8x16_t quote = vdupq_n_u8((uint8_t)'"');
    const uint8x16_t slash = vdupq_n_u8((uint8_t)'\\');
    const uint8x16_t newline = vdupq_n_u8((uint8_t)'\n');
    const uint8x16_t carriage = vdupq_n_u8((uint8_t)'\r');
    const uint8x16_t tab = vdupq_n_u8((uint8_t)'\t');
    const uint8x16_t control_limit = vdupq_n_u8(0x20U);
    size_t i = 0U;

    while (i + 16U <= len) {
        uint8x16_t chunk = vld1q_u8(text + i);
        uint8x16_t special =
            vorrq_u8(vceqq_u8(chunk, quote),
                     vorrq_u8(vceqq_u8(chunk, slash),
                              vorrq_u8(vceqq_u8(chunk, newline),
                                       vorrq_u8(vceqq_u8(chunk, carriage), vceqq_u8(chunk, tab)))));
        uint8x16_t control = vcltq_u8(chunk, control_limit);
        uint8x16_t mask = vorrq_u8(special, control);
        uint64x2_t wide = vreinterpretq_u64_u8(mask);

        if (vgetq_lane_u64(wide, 0) != 0ULL || vgetq_lane_u64(wide, 1) != 0ULL) {
            return i + ij_json_escape_scalar_find_first_special(text + i, 16U);
        }
        i += 16U;
    }

    return i + ij_json_escape_scalar_find_first_special(text + i, len - i);
}

#endif

size_t ij_json_escape_arm_neon_find_first_special(const unsigned char *text, size_t len)
{
#if defined(__aarch64__)
    if (ij_simd_runtime_has_arm_neon()) {
        return ij_json_escape_arm_neon_impl(text, len);
    }
#endif

    return ij_json_escape_scalar_find_first_special(text, len);
}
