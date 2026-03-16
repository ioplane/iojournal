/*
 * iojournal -- internal SIMD dispatch helpers
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_IJ_SIMD_DISPATCH_H
#define IOJOURNAL_IJ_SIMD_DISPATCH_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    IJ_SIMD_KIND_SCALAR = 0,
    IJ_SIMD_KIND_X86_AVX2 = 1,
    IJ_SIMD_KIND_ARM_NEON = 2,
} ij_simd_kind_t;

#define IJ_SIMD_JSON_ESCAPE_MIN_LEN 32U
#define IJ_SIMD_UTF8_MIN_LEN        32U

static inline bool ij_simd_runtime_has_x86_avx2(void)
{
#if defined(__x86_64__) || defined(__i386__)
    static _Atomic int cached = -1;
    int value = atomic_load_explicit(&cached, memory_order_relaxed);

    if (value >= 0) {
        return value != 0;
    }

    __builtin_cpu_init();
    value = __builtin_cpu_supports("avx2") ? 1 : 0;
    atomic_store_explicit(&cached, value, memory_order_relaxed);
    return value != 0;
#else
    return false;
#endif
}

static inline bool ij_simd_runtime_has_arm_neon(void)
{
#if defined(__aarch64__)
    return true;
#else
    return false;
#endif
}

static inline ij_simd_kind_t ij_simd_json_escape_kind(void)
{
    if (ij_simd_runtime_has_x86_avx2()) {
        return IJ_SIMD_KIND_X86_AVX2;
    }

    return IJ_SIMD_KIND_SCALAR;
}

static inline ij_simd_kind_t ij_simd_redact_match_kind(void)
{
    return IJ_SIMD_KIND_SCALAR;
}

static inline ij_simd_kind_t ij_simd_utf8_kind(void)
{
    if (ij_simd_runtime_has_x86_avx2()) {
        return IJ_SIMD_KIND_X86_AVX2;
    }

    return IJ_SIMD_KIND_SCALAR;
}

size_t ij_json_escape_scalar_find_first_special(const unsigned char *text, size_t len);
size_t ij_json_escape_x86_avx2_find_first_special(const unsigned char *text, size_t len);
size_t ij_json_escape_arm_neon_find_first_special(const unsigned char *text, size_t len);
size_t ij_json_escape_find_first_special(const unsigned char *text, size_t len);

bool ij_redact_case_equal_scalar(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len);
bool ij_redact_case_equal_x86_avx2(const char *lhs, size_t lhs_len, const char *rhs,
                                   size_t rhs_len);
bool ij_redact_case_equal_arm_neon(const char *lhs, size_t lhs_len, const char *rhs,
                                   size_t rhs_len);
bool ij_redact_has_dot_scalar(const char *text, size_t len);
bool ij_redact_has_dot_x86_avx2(const char *text, size_t len);
bool ij_redact_has_dot_arm_neon(const char *text, size_t len);
bool ij_redact_case_equal(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len);
bool ij_redact_has_dot(const char *text, size_t len);

bool ij_utf8_scalar_validate(const char *data, size_t len);
bool ij_utf8_x86_avx2_validate(const char *data, size_t len);
bool ij_utf8_arm_neon_validate(const char *data, size_t len);
bool ij_utf8_is_valid_dispatch(const char *data, size_t len);

#endif /* IOJOURNAL_IJ_SIMD_DISPATCH_H */
