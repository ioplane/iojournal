/*
 * iojournal -- Tier 1 zlog comparison harness
 *
 * SPDX-License-Identifier: MIT
 */

#include "bench_tier1_common.h"

#include <threads.h>

#include "zlog.h"

typedef struct {
    zlog_category_t *category;
    size_t iterations;
    uint64_t sink;
} zlog_worker_t;

static const char *zlog_console_config(void)
{
    return "[formats]\n"
           "simple = \"%m%n\"\n"
           "[rules]\n"
           "bench.DEBUG >stdout; simple\n";
}

static const char *zlog_disabled_config(void)
{
    return "[formats]\n"
           "simple = \"%m%n\"\n"
           "[rules]\n"
           "bench.ERROR >stdout; simple\n";
}

static void zlog_usage(const char *argv0)
{
    (void)fprintf(stderr,
                  "usage: %s [iterations] [--tsv] [--scenario "
                  "disabled_level|enabled_console|medium_message|medium_message_with_metadata|"
                  "contention_mpsc|append_file|all]\n",
                  argv0);
}

static int zlog_setup(const char *config, zlog_category_t **out_category)
{
    if (zlog_init_from_string(config) != 0) {
        return 1;
    }

    *out_category = zlog_get_category("bench");
    if (*out_category == NULL) {
        zlog_fini();
        return 1;
    }

    return 0;
}

static int zlog_run_message_loop(zlog_category_t *category, const char *message, size_t iterations,
                                 uint64_t *out_sink)
{
    for (size_t i = 0U; i < iterations; ++i) {
        zlog_info(category, "%s", message);
        *out_sink += 1U;
    }

    return 0;
}

static int zlog_contention_worker_main(void *arg)
{
    zlog_worker_t *worker = arg;

    if (worker == NULL || worker->category == NULL) {
        return 1;
    }

    for (size_t i = 0U; i < worker->iterations; ++i) {
        zlog_info(worker->category,
                  "%s request_id=req-bench-meta service.name=iojournal-bench "
                  "trace_id=0123456789abcdef0123456789abcdef span_id=0123456789abcdef",
                  bench_medium_message_with_metadata());
        worker->sink += 1U;
    }

    return 0;
}

static int zlog_run_contention(zlog_category_t *category, size_t iterations, uint64_t *out_sink)
{
    zlog_worker_t workers[BENCH_TIER1_PRODUCERS];
    thrd_t threads[BENCH_TIER1_PRODUCERS];

    for (size_t i = 0U; i < BENCH_TIER1_PRODUCERS; ++i) {
        size_t thread_iterations = iterations / BENCH_TIER1_PRODUCERS;

        if (i < (iterations % BENCH_TIER1_PRODUCERS)) {
            thread_iterations += 1U;
        }
        workers[i] = (zlog_worker_t){
            .category = category,
            .iterations = thread_iterations,
            .sink = 0U,
        };
        if (thrd_create(&threads[i], zlog_contention_worker_main, &workers[i]) != thrd_success) {
            return 1;
        }
    }

    for (size_t i = 0U; i < BENCH_TIER1_PRODUCERS; ++i) {
        int rc = 0;

        if (thrd_join(threads[i], &rc) != thrd_success || rc != 0) {
            return 1;
        }
        *out_sink += workers[i].sink;
    }

    return 0;
}

static int zlog_run_scenario(const char *scenario_name, const char *config, size_t iterations,
                             output_mode_t mode)
{
    zlog_category_t *category = NULL;
    uint64_t start_ns;
    uint64_t elapsed_ns;
    uint64_t sink = 0U;
    int saved_stdout = -1;
    int saved_stderr = -1;
    int rc = 0;

    if (zlog_setup(config, &category) != 0) {
        return 1;
    }
    if (bench_redirect_fd_to_devnull(STDOUT_FILENO, &saved_stdout) != 0 ||
        bench_redirect_fd_to_devnull(STDERR_FILENO, &saved_stderr) != 0) {
        zlog_fini();
        return 1;
    }

    start_ns = bench_monotonic_ns();
    if (strcmp(scenario_name, "disabled_level") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (zlog_info_enabled(category)) {
                zlog_info(category, "%s", bench_medium_message());
                sink += 1U;
            }
        }
    } else if (strcmp(scenario_name, "enabled_console") == 0) {
        rc = zlog_run_message_loop(category, bench_console_message(), iterations, &sink);
    } else if (strcmp(scenario_name, "medium_message") == 0) {
        rc = zlog_run_message_loop(category, bench_medium_message(), iterations, &sink);
    } else if (strcmp(scenario_name, "medium_message_with_metadata") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            zlog_info(category,
                      "%s request_id=req-bench-meta service.name=iojournal-bench "
                      "trace_id=0123456789abcdef0123456789abcdef span_id=0123456789abcdef",
                      bench_medium_message_with_metadata());
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "contention_mpsc") == 0) {
        rc = zlog_run_contention(category, iterations, &sink);
    } else {
        rc = 1;
    }
    elapsed_ns = bench_monotonic_ns() - start_ns;

    if (saved_stderr >= 0) {
        (void)bench_restore_fd(saved_stderr, STDERR_FILENO);
    }
    if (saved_stdout >= 0) {
        (void)bench_restore_fd(saved_stdout, STDOUT_FILENO);
    }
    zlog_fini();

    if (rc != 0) {
        return 1;
    }
    if (bench_print_result("bench_zlog", scenario_name, iterations, elapsed_ns, mode) != 0) {
        return 1;
    }
    if (mode == OUTPUT_TSV) {
        printf("meta\tstatus_sink\t%" PRIu64 "\n", sink);
    } else {
        printf("status-sink: %" PRIu64 "\n", sink);
    }

    return 0;
}

static int zlog_run_file_scenario(const char *file_path, size_t iterations, output_mode_t mode)
{
    zlog_category_t *category = NULL;
    uint64_t start_ns;
    uint64_t elapsed_ns;
    uint64_t sink = 0U;
    int saved_stdout = -1;
    int saved_stderr = -1;
    int rc = 0;

    if (zlog_setup(zlog_console_config(), &category) != 0) {
        return 1;
    }
    if (bench_redirect_fd_to_path(file_path, STDOUT_FILENO, &saved_stdout) != 0 ||
        bench_redirect_fd_to_devnull(STDERR_FILENO, &saved_stderr) != 0) {
        zlog_fini();
        return 1;
    }

    start_ns = bench_monotonic_ns();
    rc = zlog_run_message_loop(category, bench_medium_message(), iterations, &sink);
    elapsed_ns = bench_monotonic_ns() - start_ns;

    if (saved_stderr >= 0) {
        (void)bench_restore_fd(saved_stderr, STDERR_FILENO);
    }
    if (saved_stdout >= 0) {
        (void)bench_restore_fd(saved_stdout, STDOUT_FILENO);
    }
    zlog_fini();
    (void)unlink(file_path);

    if (rc != 0) {
        return 1;
    }
    if (bench_print_result("bench_zlog", "append_file", iterations, elapsed_ns, mode) != 0) {
        return 1;
    }
    if (mode == OUTPUT_TSV) {
        printf("meta\tstatus_sink\t%" PRIu64 "\n", sink);
    } else {
        printf("status-sink: %" PRIu64 "\n", sink);
    }

    return 0;
}

int main(int argc, char **argv)
{
    output_mode_t mode = OUTPUT_HUMAN;
    scenario_mode_t scenario = SCENARIO_ALL;
    size_t iterations = 50000U;
    const char *console_config = zlog_console_config();
    const char *disabled_config = zlog_disabled_config();
    char file_path[128];

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--tsv") == 0) {
            mode = OUTPUT_TSV;
            continue;
        }
        if (strcmp(argv[i], "--scenario") == 0) {
            if (i + 1 >= argc || !bench_parse_scenario(argv[i + 1], &scenario)) {
                zlog_usage(argv[0]);
                return 2;
            }
            ++i;
            continue;
        }
        if (!bench_parse_iterations(argv[i], &iterations)) {
            zlog_usage(argv[0]);
            return 2;
        }
    }

    bench_print_header("zlog", mode, iterations);
    if ((scenario == SCENARIO_DISABLED_LEVEL || scenario == SCENARIO_ALL) &&
        zlog_run_scenario("disabled_level", disabled_config, iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_ENABLED_CONSOLE || scenario == SCENARIO_ALL) &&
        zlog_run_scenario("enabled_console", console_config, iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE || scenario == SCENARIO_ALL) &&
        zlog_run_scenario("medium_message", console_config, iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE_WITH_METADATA || scenario == SCENARIO_ALL) &&
        zlog_run_scenario("medium_message_with_metadata", console_config, iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_CONTENTION_MPSC || scenario == SCENARIO_ALL) &&
        zlog_run_scenario("contention_mpsc", console_config, iterations, mode) != 0) {
        return 1;
    }
    if (scenario == SCENARIO_APPEND_FILE || scenario == SCENARIO_ALL) {
        if (bench_make_temp_path("iojournal-tier1-zlog", file_path, sizeof(file_path)) != 0) {
            return 1;
        }
        if (zlog_run_file_scenario(file_path, iterations, mode) != 0) {
            (void)unlink(file_path);
            return 1;
        }
    }

    return 0;
}
