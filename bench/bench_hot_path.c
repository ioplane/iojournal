/*
 * iojournal -- hot path benchmark harness
 *
 * SPDX-License-Identifier: MIT
 */

#include <iojournal/iojournal.h>

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
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
    SCENARIO_ALL = 5,
} scenario_mode_t;

#define IJ_CONTENTION_PRODUCER_COUNT 4U

static volatile uint64_t ij_status_sink = 0U;

typedef struct {
    ij_logger_t *logger;
    const ij_event_t *event;
    size_t iterations;
    uint64_t status_sum;
    ij_status_t first_failure;
} ij_contention_worker_t;

static uint64_t ij_monotonic_ns(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }

    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static void ij_print_header(output_mode_t mode, size_t iterations)
{
    if (mode == OUTPUT_TSV) {
        puts("format\ttsv\tv1");
        printf("meta\titerations\t%zu\n", iterations);
        puts("columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op");
        (void)fflush(stdout);
        return;
    }

    puts("iojournal benchmark: hot path");
    printf("iterations: %zu\n\n", iterations);
    (void)fflush(stdout);
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

static bool ij_parse_scenario(const char *text, scenario_mode_t *out_scenario)
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
    if (strcmp(text, "all") == 0) {
        *out_scenario = SCENARIO_ALL;
        return true;
    }

    return false;
}

static void ij_usage(const char *argv0)
{
    (void)fprintf(stderr,
                  "usage: %s [iterations] [--tsv] [--scenario "
                  "disabled_level|enabled_console|medium_message|medium_message_with_metadata|"
                  "contention_mpsc|all]\n",
                  argv0);
}

static int ij_redirect_stdout_to_devnull(int *out_saved_fd)
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

    saved_fd = dup(STDOUT_FILENO);
    if (saved_fd < 0) {
        (void)close(devnull_fd);
        return -1;
    }

    if (dup2(devnull_fd, STDOUT_FILENO) < 0) {
        (void)close(saved_fd);
        (void)close(devnull_fd);
        return -1;
    }

    (void)close(devnull_fd);
    *out_saved_fd = saved_fd;
    return 0;
}

static int ij_restore_stdout(int saved_fd)
{
    if (saved_fd < 0) {
        return -1;
    }

    if (fflush(stdout) != 0) {
        (void)close(saved_fd);
        return -1;
    }
    if (dup2(saved_fd, STDOUT_FILENO) < 0) {
        (void)close(saved_fd);
        return -1;
    }

    return close(saved_fd);
}

static ij_event_t ij_make_console_event(void)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-bench-hot",
                         .len = 13U,
                     },
             }},
        {.key = "auth.token",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "bench-secret-token",
                         .len = 18U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 1710000000, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "bench.hot_path",
        .message = "benchmark event payload for normalized hot-path scenarios",
        .logger = "bench.hot_path",
        .attributes = attributes,
        .attribute_count = sizeof(attributes) / sizeof(attributes[0]),
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "bench/bench_hot_path.c",
        .source_line = 1U,
        .source_function = "main",
    };

    return event;
}

static ij_event_t ij_make_medium_message_event(void)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 1710000000, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "bench.medium_message",
        .message = "normalized medium message payload for shared comparison without metadata",
        .logger = "bench.hot_path.medium",
        .attributes = NULL,
        .attribute_count = 0U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "bench/bench_hot_path.c",
        .source_line = 1U,
        .source_function = "main",
    };

    return event;
}

static ij_event_t ij_make_medium_message_with_metadata_event(void)
{
    static const uint8_t trace_id[] = "0123456789abcdef0123456789abcdef";
    static const uint8_t span_id[] = "0123456789abcdef";
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-bench-meta",
                         .len = 14U,
                     },
             }},
        {.key = "service.name",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "iojournal-bench",
                         .len = 15U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 1710000000, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "bench.medium_message_with_metadata",
        .message = "normalized medium message payload for shared comparison with metadata",
        .logger = "bench.hot_path.medium_metadata",
        .attributes = attributes,
        .attribute_count = sizeof(attributes) / sizeof(attributes[0]),
        .trace_id = trace_id,
        .span_id = span_id,
        .trace_flags = 1U,
        .has_trace_context = true,
        .source_file = "bench/bench_hot_path.c",
        .source_line = 1U,
        .source_function = "main",
    };

    return event;
}

static int ij_print_result(output_mode_t mode, const char *scenario_name, size_t iterations,
                           uint64_t elapsed_ns)
{
    double ns_per_op;

    ns_per_op = iterations == 0U ? 0.0 : (double)elapsed_ns / (double)iterations;
    if (mode == OUTPUT_TSV) {
        printf("bench_hot_path\t%s\t%zu\t%" PRIu64 "\t%.2f\n", scenario_name, iterations,
               elapsed_ns, ns_per_op);
        (void)fflush(stdout);
        return 0;
    }

    printf("%-18s iterations=%-8zu elapsed=%12" PRIu64 " ns ns/op=%10.2f\n", scenario_name,
           iterations, elapsed_ns, ns_per_op);
    (void)fflush(stdout);
    return 0;
}

static int ij_run_scenario(const char *scenario_name, const ij_logger_config_t *config,
                           const ij_event_t *event, size_t iterations, output_mode_t mode,
                           bool suppress_stdout)
{
    ij_logger_t *logger = NULL;
    uint64_t start_ns;
    uint64_t elapsed_ns;
    int saved_stdout = -1;

    if (ij_logger_init(&logger, config) != IJ_STATUS_OK) {
        return 1;
    }
    if (suppress_stdout && ij_redirect_stdout_to_devnull(&saved_stdout) != 0) {
        (void)ij_logger_shutdown(logger);
        return 1;
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_status_t status = ij_logger_log(logger, event);

        ij_status_sink += (uint64_t)status;
        if (status != IJ_STATUS_OK) {
            if (saved_stdout >= 0) {
                (void)ij_restore_stdout(saved_stdout);
            }
            (void)ij_logger_shutdown(logger);
            return 1;
        }
    }
    if (ij_logger_flush(logger) != IJ_STATUS_OK) {
        if (saved_stdout >= 0) {
            (void)ij_restore_stdout(saved_stdout);
        }
        (void)ij_logger_shutdown(logger);
        return 1;
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;

    if (saved_stdout >= 0 && ij_restore_stdout(saved_stdout) != 0) {
        (void)ij_logger_shutdown(logger);
        return 1;
    }
    if (ij_logger_shutdown(logger) != IJ_STATUS_OK) {
        return 1;
    }

    return ij_print_result(mode, scenario_name, iterations, elapsed_ns);
}

static int ij_contention_worker_main(void *arg)
{
    ij_contention_worker_t *worker = arg;

    if (worker == NULL || worker->logger == NULL || worker->event == NULL) {
        return 1;
    }

    for (size_t i = 0U; i < worker->iterations; ++i) {
        ij_status_t status = ij_logger_log(worker->logger, worker->event);

        worker->status_sum += (uint64_t)status;
        if (status != IJ_STATUS_OK) {
            worker->first_failure = status;
            return 1;
        }
    }

    return 0;
}

static int ij_run_contention_scenario(const char *scenario_name, const ij_logger_config_t *config,
                                      const ij_event_t *event, size_t iterations,
                                      output_mode_t mode)
{
    ij_logger_t *logger = NULL;
    ij_contention_worker_t workers[IJ_CONTENTION_PRODUCER_COUNT];
    thrd_t threads[IJ_CONTENTION_PRODUCER_COUNT];
    uint64_t start_ns;
    uint64_t elapsed_ns;
    int saved_stdout = -1;
    size_t remainder = iterations;

    if (ij_logger_init(&logger, config) != IJ_STATUS_OK) {
        return 1;
    }
    if (ij_redirect_stdout_to_devnull(&saved_stdout) != 0) {
        (void)ij_logger_shutdown(logger);
        return 1;
    }

    for (size_t i = 0U; i < IJ_CONTENTION_PRODUCER_COUNT; ++i) {
        size_t producer_iterations = iterations / IJ_CONTENTION_PRODUCER_COUNT;

        if (i < (iterations % IJ_CONTENTION_PRODUCER_COUNT)) {
            producer_iterations += 1U;
        }
        workers[i] = (ij_contention_worker_t){
            .logger = logger,
            .event = event,
            .iterations = producer_iterations,
            .status_sum = 0U,
            .first_failure = IJ_STATUS_OK,
        };
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < IJ_CONTENTION_PRODUCER_COUNT; ++i) {
        if (thrd_create(&threads[i], ij_contention_worker_main, &workers[i]) != thrd_success) {
            if (saved_stdout >= 0) {
                (void)ij_restore_stdout(saved_stdout);
            }
            (void)ij_logger_shutdown(logger);
            return 1;
        }
    }
    for (size_t i = 0U; i < IJ_CONTENTION_PRODUCER_COUNT; ++i) {
        int thread_rc = 0;

        if (thrd_join(threads[i], &thread_rc) != thrd_success || thread_rc != 0) {
            if (saved_stdout >= 0) {
                (void)ij_restore_stdout(saved_stdout);
            }
            (void)ij_logger_shutdown(logger);
            return 1;
        }
        remainder -= workers[i].iterations;
        ij_status_sink += workers[i].status_sum;
        if (workers[i].first_failure != IJ_STATUS_OK) {
            if (saved_stdout >= 0) {
                (void)ij_restore_stdout(saved_stdout);
            }
            (void)ij_logger_shutdown(logger);
            return 1;
        }
    }
    if (remainder != 0U || ij_logger_flush(logger) != IJ_STATUS_OK) {
        if (saved_stdout >= 0) {
            (void)ij_restore_stdout(saved_stdout);
        }
        (void)ij_logger_shutdown(logger);
        return 1;
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;

    if (saved_stdout >= 0 && ij_restore_stdout(saved_stdout) != 0) {
        (void)ij_logger_shutdown(logger);
        return 1;
    }
    if (ij_logger_shutdown(logger) != IJ_STATUS_OK) {
        return 1;
    }

    return ij_print_result(mode, scenario_name, iterations, elapsed_ns);
}

int main(int argc, char **argv)
{
    output_mode_t mode = OUTPUT_HUMAN;
    scenario_mode_t scenario = SCENARIO_ALL;
    size_t iterations = 50000U;
    ij_event_t console_event = ij_make_console_event();
    ij_event_t medium_message_event = ij_make_medium_message_event();
    ij_event_t medium_message_with_metadata_event = ij_make_medium_message_with_metadata_event();
    ij_logger_config_t disabled_config = {
        .min_level = IJ_LEVEL_ERROR,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "bench.hot_path.disabled",
        .file_path = NULL,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };
    ij_logger_config_t console_config = {
        .min_level = IJ_LEVEL_TRACE,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "bench.hot_path.console",
        .file_path = NULL,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--tsv") == 0) {
            mode = OUTPUT_TSV;
            continue;
        }
        if (strcmp(argv[i], "--scenario") == 0) {
            if (i + 1 >= argc || !ij_parse_scenario(argv[i + 1], &scenario)) {
                ij_usage(argv[0]);
                return 2;
            }
            ++i;
            continue;
        }
        if (!ij_parse_iterations(argv[i], &iterations)) {
            ij_usage(argv[0]);
            return 2;
        }
    }

    ij_print_header(mode, iterations);
    if ((scenario == SCENARIO_DISABLED_LEVEL || scenario == SCENARIO_ALL) &&
        ij_run_scenario("disabled_level", &disabled_config, &console_event, iterations, mode,
                        false) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_ENABLED_CONSOLE || scenario == SCENARIO_ALL) &&
        ij_run_scenario("enabled_console", &console_config, &console_event, iterations, mode,
                        true) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE || scenario == SCENARIO_ALL) &&
        ij_run_scenario("medium_message", &console_config, &medium_message_event, iterations, mode,
                        true) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE_WITH_METADATA || scenario == SCENARIO_ALL) &&
        ij_run_scenario("medium_message_with_metadata", &console_config,
                        &medium_message_with_metadata_event, iterations, mode, true) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_CONTENTION_MPSC || scenario == SCENARIO_ALL) &&
        ij_run_contention_scenario("contention_mpsc", &console_config,
                                   &medium_message_with_metadata_event, iterations, mode) != 0) {
        return 1;
    }

    if (mode == OUTPUT_TSV) {
        printf("meta\tstatus_sink\t%" PRIu64 "\n", ij_status_sink);
    } else {
        printf("\nstatus-sink: %" PRIu64 "\n", ij_status_sink);
    }

    return 0;
}
