/*
 * iojournal -- file sink benchmark harness
 *
 * SPDX-License-Identifier: MIT
 */

#include <iojournal/iojournal.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef enum {
    OUTPUT_HUMAN = 0,
    OUTPUT_TSV = 1,
} output_mode_t;

static volatile uint64_t ij_status_sink = 0U;

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

static ij_event_t ij_make_event(void)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-bench-file",
                         .len = 14U,
                     },
             }},
        {.key = "auth.token",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "file-bench-secret",
                         .len = 17U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 1710000000, .nanoseconds = 456000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "bench.file_sink",
        .message = "benchmark event payload for normalized append-only file sink scenario",
        .logger = "bench.file_sink",
        .attributes = attributes,
        .attribute_count = sizeof(attributes) / sizeof(attributes[0]),
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "bench/bench_file_sink.c",
        .source_line = 1U,
        .source_function = "main",
    };

    return event;
}

static int ij_make_temp_path(char *buffer, size_t buffer_size)
{
    int written;

    written = snprintf(buffer, buffer_size, "/tmp/iojournal-bench-file-%ld-%ld.ndjson",
                       (long)getpid(), (long)time(NULL));
    if (written <= 0 || (size_t)written >= buffer_size) {
        return -1;
    }

    return 0;
}

static int ij_print_result(output_mode_t mode, size_t iterations, uint64_t elapsed_ns,
                           uintmax_t file_bytes)
{
    double ns_per_op = (double)elapsed_ns / (double)iterations;

    if (mode == OUTPUT_TSV) {
        puts("format\ttsv\tv1");
        printf("meta\titerations\t%zu\n", iterations);
        printf("meta\tfile_bytes\t%ju\n", file_bytes);
        puts("columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op");
        printf("bench_file_sink\tappend_ndjson\t%zu\t%" PRIu64 "\t%.2f\n", iterations, elapsed_ns,
               ns_per_op);
        printf("meta\tstatus_sink\t%" PRIu64 "\n", ij_status_sink);
        return 0;
    }

    puts("iojournal benchmark: file sink");
    printf("iterations: %zu\n", iterations);
    printf("file-bytes: %ju\n", file_bytes);
    printf("append_ndjson      iterations=%-8zu elapsed=%12" PRIu64 " ns ns/op=%10.2f\n",
           iterations, elapsed_ns, ns_per_op);
    printf("status-sink: %" PRIu64 "\n", ij_status_sink);
    return 0;
}

int main(int argc, char **argv)
{
    size_t iterations = 20000U;
    output_mode_t mode = OUTPUT_HUMAN;
    char path[256];
    ij_event_t event = ij_make_event();
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;
    uint64_t start_ns;
    uint64_t elapsed_ns;
    struct stat file_stat = {0};

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

    if (ij_make_temp_path(path, sizeof(path)) != 0) {
        return 1;
    }
    (void)unlink(path);

    config = (ij_logger_config_t){
        .min_level = IJ_LEVEL_TRACE,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "bench.file_sink",
        .file_path = path,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .file_backend = IJ_FILE_BACKEND_AUTO,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };

    if (ij_logger_init(&logger, &config) != IJ_STATUS_OK) {
        return 1;
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_status_t status = ij_logger_log(logger, &event);

        ij_status_sink += (uint64_t)status;
        if (status != IJ_STATUS_OK) {
            (void)ij_logger_shutdown(logger);
            (void)unlink(path);
            return 1;
        }
    }
    if (ij_logger_flush(logger) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        (void)unlink(path);
        return 1;
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;

    if (ij_logger_shutdown(logger) != IJ_STATUS_OK) {
        (void)unlink(path);
        return 1;
    }
    if (stat(path, &file_stat) != 0) {
        (void)unlink(path);
        return 1;
    }
    if (unlink(path) != 0) {
        return 1;
    }

    return ij_print_result(mode, iterations, elapsed_ns, (uintmax_t)file_stat.st_size);
}
