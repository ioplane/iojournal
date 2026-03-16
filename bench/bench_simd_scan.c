/*
 * iojournal -- SIMD hotspot microbenchmarks
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    OUTPUT_HUMAN = 0,
    OUTPUT_TSV = 1,
} output_mode_t;

static volatile uint64_t ij_bench_sink = 0U;

static uint64_t ij_monotonic_ns(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }

    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static bool ij_parse_iterations(const char *text, size_t *out_iterations)
{
    char *end = NULL;
    unsigned long long parsed;

    if (text == NULL || out_iterations == NULL) {
        return false;
    }

    parsed = strtoull(text, &end, 10);
    if (end == text || *end != '\0' || parsed == 0ULL) {
        return false;
    }

    *out_iterations = (size_t)parsed;
    return true;
}

static void ij_usage(const char *argv0)
{
    (void)fprintf(stderr, "usage: %s [iterations] [--tsv]\n", argv0);
}

static const char *ij_simd_kind_label(ij_simd_kind_t kind)
{
    switch (kind) {
    case IJ_SIMD_KIND_X86_AVX2:
        return "x86_avx2";
    case IJ_SIMD_KIND_ARM_NEON:
        return "arm_neon";
    case IJ_SIMD_KIND_SCALAR:
    default:
        return "scalar";
    }
}

static int ij_print_row(output_mode_t mode, const char *scenario, size_t iterations,
                        uint64_t elapsed_ns)
{
    double ns_per_op = (double)elapsed_ns / (double)iterations;

    if (mode == OUTPUT_TSV) {
        printf("bench_simd_scan\t%s\t%zu\t%" PRIu64 "\t%.2f\n", scenario, iterations, elapsed_ns,
               ns_per_op);
        return 0;
    }

    printf("%-24s iterations=%-8zu elapsed=%12" PRIu64 " ns ns/op=%10.2f\n", scenario, iterations,
           elapsed_ns, ns_per_op);
    return 0;
}

int main(int argc, char **argv)
{
    static const unsigned char json_payload[] =
        "service=request path=/api/v1/items message=\"hello world\" region=eu-west-1 trace=req-42";
    static const char redact_key[] = "auth.token";
    static const char redact_suffix[] = "token";
    static const char utf8_payload[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    output_mode_t mode = OUTPUT_HUMAN;
    size_t iterations = 200000U;
    size_t redact_key_len = strlen(redact_key);
    size_t redact_suffix_len = strlen(redact_suffix);
    bool has_x86_avx2 = ij_simd_runtime_has_x86_avx2();
    bool has_arm_neon = ij_simd_runtime_has_arm_neon();
    uint64_t start_ns;
    uint64_t elapsed_ns;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--tsv") == 0) {
            mode = OUTPUT_TSV;
            continue;
        }
        if (!ij_parse_iterations(argv[i], &iterations)) {
            ij_usage(argv[0]);
            return 2;
        }
    }

    if (mode == OUTPUT_TSV) {
        puts("format\ttsv\tv1");
        printf("meta\titerations\t%zu\n", iterations);
        printf("meta\tjson_kind\t%s\n", ij_simd_kind_label(ij_simd_json_escape_kind()));
        printf("meta\tredact_kind\t%s\n", ij_simd_kind_label(ij_simd_redact_match_kind()));
        printf("meta\tutf8_kind\t%s\n", ij_simd_kind_label(ij_simd_utf8_kind()));
        puts("columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op");
    } else {
        puts("iojournal benchmark: SIMD scan");
        printf("iterations: %zu\n", iterations);
        printf("json-kind: %s\n", ij_simd_kind_label(ij_simd_json_escape_kind()));
        printf("redact-kind: %s\n", ij_simd_kind_label(ij_simd_redact_match_kind()));
        printf("utf8-kind: %s\n\n", ij_simd_kind_label(ij_simd_utf8_kind()));
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_bench_sink += (uint64_t)ij_json_escape_scalar_find_first_special(
            json_payload, sizeof(json_payload) - 1U);
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;
    (void)ij_print_row(mode, "json_escape_scalar", iterations, elapsed_ns);

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_bench_sink +=
            (uint64_t)ij_json_escape_find_first_special(json_payload, sizeof(json_payload) - 1U);
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;
    (void)ij_print_row(mode, "json_escape_active", iterations, elapsed_ns);

    if (has_x86_avx2) {
        start_ns = ij_monotonic_ns();
        for (size_t i = 0U; i < iterations; ++i) {
            ij_bench_sink += (uint64_t)ij_json_escape_x86_avx2_find_first_special(
                json_payload, sizeof(json_payload) - 1U);
        }
        elapsed_ns = ij_monotonic_ns() - start_ns;
        (void)ij_print_row(mode, "json_escape_x86_avx2", iterations, elapsed_ns);
    } else if (has_arm_neon) {
        start_ns = ij_monotonic_ns();
        for (size_t i = 0U; i < iterations; ++i) {
            ij_bench_sink += (uint64_t)ij_json_escape_arm_neon_find_first_special(
                json_payload, sizeof(json_payload) - 1U);
        }
        elapsed_ns = ij_monotonic_ns() - start_ns;
        (void)ij_print_row(mode, "json_escape_arm_neon", iterations, elapsed_ns);
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_bench_sink += (uint64_t)ij_redact_case_equal_scalar(redact_key, redact_key_len,
                                                               redact_suffix, redact_suffix_len);
        ij_bench_sink += (uint64_t)ij_redact_has_dot_scalar(redact_key, redact_key_len);
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;
    (void)ij_print_row(mode, "redact_match_scalar", iterations, elapsed_ns);

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_bench_sink += (uint64_t)ij_redact_case_equal(redact_key, redact_key_len, redact_suffix,
                                                        redact_suffix_len);
        ij_bench_sink += (uint64_t)ij_redact_has_dot(redact_key, redact_key_len);
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;
    (void)ij_print_row(mode, "redact_match_active", iterations, elapsed_ns);

    if (has_x86_avx2) {
        start_ns = ij_monotonic_ns();
        for (size_t i = 0U; i < iterations; ++i) {
            ij_bench_sink += (uint64_t)ij_redact_case_equal_x86_avx2(
                redact_key, redact_key_len, redact_suffix, redact_suffix_len);
            ij_bench_sink += (uint64_t)ij_redact_has_dot_x86_avx2(redact_key, redact_key_len);
        }
        elapsed_ns = ij_monotonic_ns() - start_ns;
        (void)ij_print_row(mode, "redact_match_x86_avx2", iterations, elapsed_ns);
    } else if (has_arm_neon) {
        start_ns = ij_monotonic_ns();
        for (size_t i = 0U; i < iterations; ++i) {
            ij_bench_sink += (uint64_t)ij_redact_case_equal_arm_neon(
                redact_key, redact_key_len, redact_suffix, redact_suffix_len);
            ij_bench_sink += (uint64_t)ij_redact_has_dot_arm_neon(redact_key, redact_key_len);
        }
        elapsed_ns = ij_monotonic_ns() - start_ns;
        (void)ij_print_row(mode, "redact_match_arm_neon", iterations, elapsed_ns);
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_bench_sink += (uint64_t)ij_utf8_scalar_validate(utf8_payload, sizeof(utf8_payload) - 1U);
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;
    (void)ij_print_row(mode, "utf8_scalar", iterations, elapsed_ns);

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_bench_sink +=
            (uint64_t)ij_utf8_is_valid_dispatch(utf8_payload, sizeof(utf8_payload) - 1U);
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;
    (void)ij_print_row(mode, "utf8_active", iterations, elapsed_ns);

    if (has_x86_avx2) {
        start_ns = ij_monotonic_ns();
        for (size_t i = 0U; i < iterations; ++i) {
            ij_bench_sink +=
                (uint64_t)ij_utf8_x86_avx2_validate(utf8_payload, sizeof(utf8_payload) - 1U);
        }
        elapsed_ns = ij_monotonic_ns() - start_ns;
        (void)ij_print_row(mode, "utf8_x86_avx2", iterations, elapsed_ns);
    } else if (has_arm_neon) {
        start_ns = ij_monotonic_ns();
        for (size_t i = 0U; i < iterations; ++i) {
            ij_bench_sink +=
                (uint64_t)ij_utf8_arm_neon_validate(utf8_payload, sizeof(utf8_payload) - 1U);
        }
        elapsed_ns = ij_monotonic_ns() - start_ns;
        (void)ij_print_row(mode, "utf8_arm_neon", iterations, elapsed_ns);
    }

    if (mode == OUTPUT_TSV) {
        printf("meta\tbench_sink\t%" PRIu64 "\n", ij_bench_sink);
    } else {
        printf("bench-sink: %" PRIu64 "\n", ij_bench_sink);
    }

    return 0;
}
