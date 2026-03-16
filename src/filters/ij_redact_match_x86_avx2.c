/*
 * iojournal -- AVX2 redaction matching
 *
 * SPDX-License-Identifier: MIT
 */

#include "encoders/ij_simd_dispatch.h"

#if defined(__x86_64__) || defined(__i386__)

#    include <immintrin.h>

__attribute__((target("avx2"))) static __m256i ij_redact_lower_x86_avx2(__m256i value)
{
    const __m256i upper_a = _mm256_set1_epi8('A' - 1);
    const __m256i upper_z = _mm256_set1_epi8('Z' + 1);
    const __m256i case_bit = _mm256_set1_epi8(0x20);
    __m256i ge_a = _mm256_cmpgt_epi8(value, upper_a);
    __m256i lt_z = _mm256_cmpgt_epi8(upper_z, value);
    __m256i upper = _mm256_and_si256(ge_a, lt_z);

    return _mm256_or_si256(value, _mm256_and_si256(upper, case_bit));
}

__attribute__((target("avx2"))) static bool
ij_redact_case_equal_x86_avx2_impl(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len)
{
    size_t i = 0U;

    if (lhs_len != rhs_len) {
        return false;
    }

    while (i + 32U <= lhs_len) {
        __m256i lhs_chunk = _mm256_loadu_si256((const __m256i *)(const void *)(lhs + i));
        __m256i rhs_chunk = _mm256_loadu_si256((const __m256i *)(const void *)(rhs + i));
        unsigned mask = (unsigned)_mm256_movemask_epi8(_mm256_cmpeq_epi8(
            ij_redact_lower_x86_avx2(lhs_chunk), ij_redact_lower_x86_avx2(rhs_chunk)));

        if (mask != 0xffffffffU) {
            return false;
        }
        i += 32U;
    }

    return ij_redact_case_equal_scalar(lhs + i, lhs_len - i, rhs + i, rhs_len - i);
}

__attribute__((target("avx2"))) static bool ij_redact_has_dot_x86_avx2_impl(const char *text,
                                                                            size_t len)
{
    const __m256i dot = _mm256_set1_epi8('.');
    size_t i = 0U;

    while (i + 32U <= len) {
        __m256i chunk = _mm256_loadu_si256((const __m256i *)(const void *)(text + i));

        if (_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, dot)) != 0) {
            return true;
        }
        i += 32U;
    }

    return ij_redact_has_dot_scalar(text + i, len - i);
}

#endif

bool ij_redact_case_equal_x86_avx2(const char *lhs, size_t lhs_len, const char *rhs, size_t rhs_len)
{
#if defined(__x86_64__) || defined(__i386__)
    if (ij_simd_runtime_has_x86_avx2()) {
        return ij_redact_case_equal_x86_avx2_impl(lhs, lhs_len, rhs, rhs_len);
    }
#endif

    return ij_redact_case_equal_scalar(lhs, lhs_len, rhs, rhs_len);
}

bool ij_redact_has_dot_x86_avx2(const char *text, size_t len)
{
#if defined(__x86_64__) || defined(__i386__)
    if (ij_simd_runtime_has_x86_avx2()) {
        return ij_redact_has_dot_x86_avx2_impl(text, len);
    }
#endif

    return ij_redact_has_dot_scalar(text, len);
}
