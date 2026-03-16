/*
 * iojournal -- NEON redaction matching
 *
 * SPDX-License-Identifier: MIT
 */

#include "encoders/ij_simd_dispatch.h"

#if defined(__aarch64__)

#    include <arm_neon.h>

static uint8x16_t ij_redact_lower_arm_neon(uint8x16_t value)
{
    const uint8x16_t upper_a = vdupq_n_u8((uint8_t)'A');
    const uint8x16_t upper_z = vdupq_n_u8((uint8_t)'Z');
    const uint8x16_t case_bit = vdupq_n_u8(0x20U);
    uint8x16_t ge_a = vcgeq_u8(value, upper_a);
    uint8x16_t le_z = vcleq_u8(value, upper_z);
    uint8x16_t upper = vandq_u8(ge_a, le_z);

    return vorrq_u8(value, vandq_u8(upper, case_bit));
}

static bool ij_redact_case_equal_arm_neon_impl(const char *lhs, size_t lhs_len, const char *rhs,
                                               size_t rhs_len)
{
    size_t i = 0U;

    if (lhs_len != rhs_len) {
        return false;
    }

    while (i + 16U <= lhs_len) {
        uint8x16_t lhs_chunk = vld1q_u8((const uint8_t *)(const void *)(lhs + i));
        uint8x16_t rhs_chunk = vld1q_u8((const uint8_t *)(const void *)(rhs + i));
        uint8x16_t eq =
            vceqq_u8(ij_redact_lower_arm_neon(lhs_chunk), ij_redact_lower_arm_neon(rhs_chunk));
        uint64x2_t wide = vreinterpretq_u64_u8(eq);

        if (vgetq_lane_u64(wide, 0) != UINT64_MAX || vgetq_lane_u64(wide, 1) != UINT64_MAX) {
            return false;
        }
        i += 16U;
    }

    return ij_redact_case_equal_scalar(lhs + i, lhs_len - i, rhs + i, rhs_len - i);
}

static bool ij_redact_has_dot_arm_neon_impl(const char *text, size_t len)
{
    const uint8x16_t dot = vdupq_n_u8((uint8_t)'.');
    size_t i = 0U;

    while (i + 16U <= len) {
        uint8x16_t chunk = vld1q_u8((const uint8_t *)(const void *)(text + i));
        uint8x16_t eq = vceqq_u8(chunk, dot);
        uint64x2_t wide = vreinterpretq_u64_u8(eq);

        if (vgetq_lane_u64(wide, 0) != 0ULL || vgetq_lane_u64(wide, 1) != 0ULL) {
            return true;
        }
        i += 16U;
    }

    return ij_redact_has_dot_scalar(text + i, len - i);
}

#endif

bool ij_redact_case_equal_arm_neon(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len)
{
#if defined(__aarch64__)
    if (ij_simd_runtime_has_arm_neon()) {
        return ij_redact_case_equal_arm_neon_impl(lhs, lhs_len, rhs, rhs_len);
    }
#endif

    return ij_redact_case_equal_scalar(lhs, lhs_len, rhs, rhs_len);
}

bool ij_redact_has_dot_arm_neon(const char *text, size_t len)
{
#if defined(__aarch64__)
    if (ij_simd_runtime_has_arm_neon()) {
        return ij_redact_has_dot_arm_neon_impl(text, len);
    }
#endif

    return ij_redact_has_dot_scalar(text, len);
}
