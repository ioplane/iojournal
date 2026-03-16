/*
 * iojournal -- AVX2 UTF-8 validation fast path
 *
 * SPDX-License-Identifier: MIT
 */

#include "encoders/ij_simd_dispatch.h"

#if defined(__x86_64__) || defined(__i386__)

#    include <immintrin.h>

__attribute__((target("avx2"))) static bool ij_utf8_x86_avx2_impl(const char *data, size_t len)
{
    size_t i = 0U;

    while (i + 32U <= len) {
        __m256i chunk = _mm256_loadu_si256((const __m256i *)(const void *)(data + i));

        if (_mm256_movemask_epi8(chunk) != 0) {
            break;
        }
        i += 32U;
    }

    return ij_utf8_scalar_validate(data + i, len - i);
}

#endif

bool ij_utf8_x86_avx2_validate(const char *data, size_t len)
{
#if defined(__x86_64__) || defined(__i386__)
    if (ij_simd_runtime_has_x86_avx2()) {
        return ij_utf8_x86_avx2_impl(data, len);
    }
#endif

    return ij_utf8_scalar_validate(data, len);
}
