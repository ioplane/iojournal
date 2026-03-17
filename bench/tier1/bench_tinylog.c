/*
 * iojournal -- Tier 1 tinylog comparison harness
 *
 * SPDX-License-Identifier: MIT
 */

#include "bench_tier1_common.h"

#include <threads.h>

#include "tlog.h"

typedef struct {
    size_t iterations;
    uint64_t sink;
} tinylog_worker_t;

static int tinylog_bench_output(struct tlog_loginfo *info, const char *buff, int bufflen,
                                void *private_data)
{
    (void)info;
    (void)buff;
    (void)private_data;
    return bufflen;
}

static void tinylog_usage(const char *argv0)
{
    (void)fprintf(stderr,
                  "usage: %s [iterations] [--tsv] [--scenario "
                  "disabled_level|enabled_console|medium_message|medium_message_with_metadata|"
                  "contention_mpsc|append_file|all]\n",
                  argv0);
}

static int tinylog_contention_worker_main(void *arg)
{
    tinylog_worker_t *worker = arg;

    if (worker == NULL) {
        return 1;
    }

    for (size_t i = 0U; i < worker->iterations; ++i) {
        if (tlog_info("%s request_id=req-bench-meta service.name=iojournal-bench "
                      "trace_id=0123456789abcdef0123456789abcdef span_id=0123456789abcdef",
                      bench_medium_message_with_metadata()) < 0) {
            return 1;
        }
        worker->sink += 1U;
    }

    return 0;
}

static int tinylog_setup(const char *scenario_name, char *temp_path, size_t temp_path_size)
{
    if (strcmp(scenario_name, "append_file") == 0) {
        if (bench_make_temp_path("iojournal-tier1-tinylog", temp_path, temp_path_size) != 0) {
            return 1;
        }
        if (tlog_init(temp_path, 1024 * 1024, 4, 0, 0) != 0) {
            (void)unlink(temp_path);
            return 1;
        }
    } else {
        if (tlog_init("/dev/null", 1024 * 1024, 4, 0, TLOG_SEGMENT) != 0) {
            return 1;
        }
        if (tlog_reg_log_output_func(tinylog_bench_output, NULL) != 0) {
            tlog_exit();
            return 1;
        }
    }

    return 0;
}

static int tinylog_run_scenario(const char *scenario_name, size_t iterations, output_mode_t mode)
{
    char temp_path[128];
    uint64_t start_ns;
    uint64_t elapsed_ns;
    uint64_t sink = 0U;
    int rc = 0;

    if (tinylog_setup(scenario_name, temp_path, sizeof(temp_path)) != 0) {
        return 1;
    }

    if (strcmp(scenario_name, "disabled_level") == 0) {
        (void)tlog_setlevel(TLOG_WARN);
    } else {
        (void)tlog_setlevel(TLOG_DEBUG);
    }

    start_ns = bench_monotonic_ns();
    if (strcmp(scenario_name, "disabled_level") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            sink += (uint64_t)tlog_info("%s", bench_medium_message());
        }
    } else if (strcmp(scenario_name, "enabled_console") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (tlog_info("%s", bench_console_message()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "medium_message") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (tlog_info("%s", bench_medium_message()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "medium_message_with_metadata") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (tlog_info("%s request_id=req-bench-meta service.name=iojournal-bench "
                          "trace_id=0123456789abcdef0123456789abcdef span_id=0123456789abcdef",
                          bench_medium_message_with_metadata()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "contention_mpsc") == 0) {
        tinylog_worker_t workers[BENCH_TIER1_PRODUCERS];
        thrd_t threads[BENCH_TIER1_PRODUCERS];

        for (size_t i = 0U; i < BENCH_TIER1_PRODUCERS; ++i) {
            size_t thread_iterations = iterations / BENCH_TIER1_PRODUCERS;

            if (i < (iterations % BENCH_TIER1_PRODUCERS)) {
                thread_iterations += 1U;
            }
            workers[i] = (tinylog_worker_t){
                .iterations = thread_iterations,
                .sink = 0U,
            };
            if (thrd_create(&threads[i], tinylog_contention_worker_main, &workers[i]) !=
                thrd_success) {
                rc = 1;
                break;
            }
        }
        for (size_t i = 0U; rc == 0 && i < BENCH_TIER1_PRODUCERS; ++i) {
            int thread_rc = 0;

            if (thrd_join(threads[i], &thread_rc) != thrd_success || thread_rc != 0) {
                rc = 1;
                break;
            }
            sink += workers[i].sink;
        }
    } else if (strcmp(scenario_name, "append_file") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (tlog_info("%s", bench_medium_message()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else {
        rc = 1;
    }
    elapsed_ns = bench_monotonic_ns() - start_ns;

    tlog_exit();
    if (strcmp(scenario_name, "append_file") == 0) {
        (void)unlink(temp_path);
    }

    if (rc != 0) {
        return 1;
    }
    if (bench_print_result("bench_tinylog", scenario_name, iterations, elapsed_ns, mode) != 0) {
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

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--tsv") == 0) {
            mode = OUTPUT_TSV;
            continue;
        }
        if (strcmp(argv[i], "--scenario") == 0) {
            if (i + 1 >= argc || !bench_parse_scenario(argv[i + 1], &scenario)) {
                tinylog_usage(argv[0]);
                return 2;
            }
            ++i;
            continue;
        }
        if (!bench_parse_iterations(argv[i], &iterations)) {
            tinylog_usage(argv[0]);
            return 2;
        }
    }

    bench_print_header("tinylog", mode, iterations);
    if ((scenario == SCENARIO_DISABLED_LEVEL || scenario == SCENARIO_ALL) &&
        tinylog_run_scenario("disabled_level", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_ENABLED_CONSOLE || scenario == SCENARIO_ALL) &&
        tinylog_run_scenario("enabled_console", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE || scenario == SCENARIO_ALL) &&
        tinylog_run_scenario("medium_message", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE_WITH_METADATA || scenario == SCENARIO_ALL) &&
        tinylog_run_scenario("medium_message_with_metadata", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_CONTENTION_MPSC || scenario == SCENARIO_ALL) &&
        tinylog_run_scenario("contention_mpsc", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_APPEND_FILE || scenario == SCENARIO_ALL) &&
        tinylog_run_scenario("append_file", iterations, mode) != 0) {
        return 1;
    }

    return 0;
}
