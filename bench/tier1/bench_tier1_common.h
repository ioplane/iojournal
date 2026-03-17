/*
 * iojournal -- Tier 1 comparison benchmark helpers
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_BENCH_TIER1_COMMON_H
#define IOJOURNAL_BENCH_TIER1_COMMON_H

#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef enum {
    OUTPUT_HUMAN = 0,
    OUTPUT_TSV = 1,
} output_mode_t;

typedef enum {
    SCENARIO_DISABLED_LEVEL = 0,
    SCENARIO_ENABLED_CONSOLE = 1,
    SCENARIO_MEDIUM_MESSAGE = 2,
    SCENARIO_MEDIUM_MESSAGE_WITH_METADATA = 3,
    SCENARIO_CONTENTION_MPSC = 4,
    SCENARIO_APPEND_FILE = 5,
    SCENARIO_ALL = 6,
} scenario_mode_t;

#define BENCH_TIER1_PRODUCERS 4U

static inline uint64_t bench_monotonic_ns(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }

    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static inline bool bench_parse_iterations(const char *text, size_t *out_iterations)
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

static inline bool bench_parse_scenario(const char *text, scenario_mode_t *out_scenario)
{
    if (text == NULL || out_scenario == NULL) {
        return false;
    }

    if (strcmp(text, "disabled_level") == 0) {
        *out_scenario = SCENARIO_DISABLED_LEVEL;
        return true;
    }
    if (strcmp(text, "enabled_console") == 0) {
        *out_scenario = SCENARIO_ENABLED_CONSOLE;
        return true;
    }
    if (strcmp(text, "medium_message") == 0) {
        *out_scenario = SCENARIO_MEDIUM_MESSAGE;
        return true;
    }
    if (strcmp(text, "medium_message_with_metadata") == 0) {
        *out_scenario = SCENARIO_MEDIUM_MESSAGE_WITH_METADATA;
        return true;
    }
    if (strcmp(text, "contention_mpsc") == 0) {
        *out_scenario = SCENARIO_CONTENTION_MPSC;
        return true;
    }
    if (strcmp(text, "append_file") == 0) {
        *out_scenario = SCENARIO_APPEND_FILE;
        return true;
    }
    if (strcmp(text, "all") == 0) {
        *out_scenario = SCENARIO_ALL;
        return true;
    }

    return false;
}

static inline void bench_print_header(const char *name, output_mode_t mode, size_t iterations)
{
    if (mode == OUTPUT_TSV) {
        puts("format\ttsv\tv1");
        printf("meta\titerations\t%zu\n", iterations);
        puts("columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op");
        (void)fflush(stdout);
        return;
    }

    printf("%s benchmark\n", name);
    printf("iterations: %zu\n\n", iterations);
    (void)fflush(stdout);
}

static inline int bench_print_result(const char *benchmark_name, const char *scenario_name,
                                     size_t iterations, uint64_t elapsed_ns, output_mode_t mode)
{
    double ns_per_op = iterations == 0U ? 0.0 : (double)elapsed_ns / (double)iterations;

    if (mode == OUTPUT_TSV) {
        printf("%s\t%s\t%zu\t%" PRIu64 "\t%.2f\n", benchmark_name, scenario_name, iterations,
               elapsed_ns, ns_per_op);
        return 0;
    }

    printf("%-28s iterations=%-8zu elapsed=%12" PRIu64 " ns ns/op=%10.2f\n", scenario_name,
           iterations, elapsed_ns, ns_per_op);
    return 0;
}

static inline int bench_redirect_fd_to_devnull(int fd, int *out_saved_fd)
{
    int devnull_fd;
    int saved_fd;

    if (out_saved_fd == NULL) {
        return -1;
    }

    devnull_fd = open("/dev/null", O_WRONLY);
    if (devnull_fd < 0) {
        return -1;
    }

    saved_fd = dup(fd);
    if (saved_fd < 0) {
        (void)close(devnull_fd);
        return -1;
    }

    if (dup2(devnull_fd, fd) < 0) {
        (void)close(saved_fd);
        (void)close(devnull_fd);
        return -1;
    }

    (void)close(devnull_fd);
    *out_saved_fd = saved_fd;
    return 0;
}

static inline int bench_restore_fd(int saved_fd, int fd)
{
    if (saved_fd < 0) {
        return -1;
    }
    if (dup2(saved_fd, fd) < 0) {
        (void)close(saved_fd);
        return -1;
    }

    return close(saved_fd);
}

static inline int bench_redirect_fd_to_path(const char *path, int fd, int *out_saved_fd)
{
    int output_fd;
    int saved_fd;

    if (path == NULL || out_saved_fd == NULL) {
        return -1;
    }

    output_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (output_fd < 0) {
        return -1;
    }

    saved_fd = dup(fd);
    if (saved_fd < 0) {
        (void)close(output_fd);
        return -1;
    }

    if (dup2(output_fd, fd) < 0) {
        (void)close(saved_fd);
        (void)close(output_fd);
        return -1;
    }

    (void)close(output_fd);
    *out_saved_fd = saved_fd;
    return 0;
}

static inline int bench_make_temp_path(const char *prefix, char *buffer, size_t buffer_size)
{
    int fd;

    if (prefix == NULL || buffer == NULL || buffer_size < 32U) {
        return -1;
    }

    if (snprintf(buffer, buffer_size, "/tmp/%s-XXXXXX", prefix) < 0) {
        return -1;
    }

    fd = mkstemp(buffer);
    if (fd < 0) {
        return -1;
    }

    return close(fd);
}

static inline const char *bench_console_message(void)
{
    return "benchmark event payload for normalized console scenario";
}

static inline const char *bench_medium_message(void)
{
    return "normalized medium message payload for shared comparison without metadata";
}

static inline const char *bench_medium_message_with_metadata(void)
{
    return "normalized medium message payload for shared comparison with metadata-like context";
}

#endif /* IOJOURNAL_BENCH_TIER1_COMMON_H */
