/*
 * iojournal -- AVX2 JSON escape scanning
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_simd_dispatch.h"

#if defined(__x86_64__) || defined(__i386__)

#    include <immintrin.h>

__attribute__((target("avx2"))) static size_t
ij_json_escape_x86_avx2_impl(const unsigned char *text, size_t len)
{
    const __m256i quote = _mm256_set1_epi8('"');
    const __m256i slash = _mm256_set1_epi8('\\');
    const __m256i newline = _mm256_set1_epi8('\n');
    const __m256i carriage = _mm256_set1_epi8('\r');
    const __m256i tab = _mm256_set1_epi8('\t');
    const __m256i control_limit = _mm256_set1_epi8(0x1f);
    const __m256i zero = _mm256_setzero_si256();
    size_t i = 0U;

    while (i + 32U <= len) {
        __m256i chunk = _mm256_loadu_si256((const __m256i *)(const void *)(text + i));
        __m256i special = _mm256_or_si256(
            _mm256_cmpeq_epi8(chunk, quote),
            _mm256_or_si256(_mm256_cmpeq_epi8(chunk, slash),
                            _mm256_or_si256(_mm256_cmpeq_epi8(chunk, newline),
                                            _mm256_or_si256(_mm256_cmpeq_epi8(chunk, carriage),
                                                            _mm256_cmpeq_epi8(chunk, tab)))));
        __m256i control = _mm256_cmpeq_epi8(_mm256_subs_epu8(chunk, control_limit), zero);
        unsigned mask = (unsigned)_mm256_movemask_epi8(_mm256_or_si256(special, control));

        if (mask != 0U) {
            return i + (size_t)__builtin_ctz(mask);
        }
        i += 32U;
    }

    return i + ij_json_escape_scalar_find_first_special(text + i, len - i);
}

#endif

size_t ij_json_escape_x86_avx2_find_first_special(const unsigned char *text, size_t len)
{
#if defined(__x86_64__) || defined(__i386__)
    if (ij_simd_runtime_has_x86_avx2()) {
        return ij_json_escape_x86_avx2_impl(text, len);
    }
#endif

    return ij_json_escape_scalar_find_first_special(text, len);
}
