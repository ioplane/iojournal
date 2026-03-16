/*
 * iojournal -- Tier 1 stumpless comparison harness
 *
 * SPDX-License-Identifier: MIT
 */

#include "bench_tier1_common.h"

#include <syslog.h>
#include <threads.h>

#include <stumpless.h>
#include <stumpless/log.h>
#include <stumpless/severity.h>
#include <stumpless/target.h>
#include <stumpless/target/file.h>
#include <stumpless/target/stream.h>

typedef struct {
    size_t iterations;
    uint64_t sink;
} stumpless_worker_t;

static void stumpless_usage(const char *argv0)
{
    (void)fprintf(stderr,
                  "usage: %s [iterations] [--tsv] [--scenario "
                  "disabled_level|enabled_console|medium_message|medium_message_with_metadata|"
                  "contention_mpsc|append_file|all]\n",
                  argv0);
}

static int stumpless_contention_worker_main(void *arg)
{
    stumpless_worker_t *worker = arg;

    if (worker == NULL) {
        return 1;
    }

    for (size_t i = 0U; i < worker->iterations; ++i) {
        if (stump_i("%s request_id=req-bench-meta service.name=iojournal-bench "
                    "trace_id=0123456789abcdef0123456789abcdef span_id=0123456789abcdef",
                    bench_medium_message_with_metadata()) < 0) {
            return 1;
        }
        worker->sink += 1U;
    }

    return 0;
}

static int stumpless_run_scenario(const char *scenario_name, size_t iterations, output_mode_t mode)
{
    struct stumpless_target *target = NULL;
    FILE *devnull_stream = NULL;
    char temp_path[128];
    uint64_t start_ns;
    uint64_t elapsed_ns;
    uint64_t sink = 0U;
    int rc = 0;

    if (strcmp(scenario_name, "append_file") == 0) {
        if (bench_make_temp_path("iojournal-tier1-stumpless", temp_path, sizeof(temp_path)) != 0) {
            return 1;
        }
        target = stumpless_open_file_target(temp_path);
        if (target == NULL) {
            (void)unlink(temp_path);
            return 1;
        }
    } else {
        devnull_stream = fopen("/dev/null", "w");
        if (devnull_stream == NULL) {
            return 1;
        }

        target = stumpless_open_stream_target("iojournal-tier1-bench", devnull_stream);
        if (target == NULL) {
            (void)fclose(devnull_stream);
            return 1;
        }
    }
    stumpless_set_current_target(target);

    if (strcmp(scenario_name, "disabled_level") == 0) {
        (void)stumpless_set_target_mask(target, STUMPLESS_SEVERITY_MASK_UPTO(LOG_ERR));
    } else {
        (void)stumpless_set_target_mask(target, STUMPLESS_SEVERITY_MASK_UPTO(LOG_INFO));
    }

    start_ns = bench_monotonic_ns();
    if (strcmp(scenario_name, "disabled_level") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (stump_i("%s", bench_medium_message()) < 0) {
                rc = 1;
                break;
            }
        }
    } else if (strcmp(scenario_name, "enabled_console") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (stump_i("%s", bench_console_message()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "medium_message") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (stump_i("%s", bench_medium_message()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "medium_message_with_metadata") == 0) {
        for (size_t i = 0U; i < iterations; ++i) {
            if (stump_i("%s request_id=req-bench-meta service.name=iojournal-bench "
                        "trace_id=0123456789abcdef0123456789abcdef span_id=0123456789abcdef",
                        bench_medium_message_with_metadata()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else if (strcmp(scenario_name, "contention_mpsc") == 0) {
        stumpless_worker_t workers[BENCH_TIER1_PRODUCERS];
        thrd_t threads[BENCH_TIER1_PRODUCERS];

        for (size_t i = 0U; i < BENCH_TIER1_PRODUCERS; ++i) {
            size_t thread_iterations = iterations / BENCH_TIER1_PRODUCERS;

            if (i < (iterations % BENCH_TIER1_PRODUCERS)) {
                thread_iterations += 1U;
            }
            workers[i] = (stumpless_worker_t){
                .iterations = thread_iterations,
                .sink = 0U,
            };
            if (thrd_create(&threads[i], stumpless_contention_worker_main, &workers[i]) !=
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
            if (stump_i("%s", bench_medium_message()) < 0) {
                rc = 1;
                break;
            }
            sink += 1U;
        }
    } else {
        rc = 1;
    }
    elapsed_ns = bench_monotonic_ns() - start_ns;

    (void)stumpless_close_target(target);
    if (devnull_stream != NULL) {
        (void)fclose(devnull_stream);
    }
    if (strcmp(scenario_name, "append_file") == 0) {
        (void)unlink(temp_path);
    }

    if (rc != 0) {
        return 1;
    }
    if (bench_print_result("bench_stumpless", scenario_name, iterations, elapsed_ns, mode) != 0) {
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
                stumpless_usage(argv[0]);
                return 2;
            }
            ++i;
            continue;
        }
        if (!bench_parse_iterations(argv[i], &iterations)) {
            stumpless_usage(argv[0]);
            return 2;
        }
    }

    bench_print_header("stumpless", mode, iterations);
    if ((scenario == SCENARIO_DISABLED_LEVEL || scenario == SCENARIO_ALL) &&
        stumpless_run_scenario("disabled_level", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_ENABLED_CONSOLE || scenario == SCENARIO_ALL) &&
        stumpless_run_scenario("enabled_console", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE || scenario == SCENARIO_ALL) &&
        stumpless_run_scenario("medium_message", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_MEDIUM_MESSAGE_WITH_METADATA || scenario == SCENARIO_ALL) &&
        stumpless_run_scenario("medium_message_with_metadata", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_CONTENTION_MPSC || scenario == SCENARIO_ALL) &&
        stumpless_run_scenario("contention_mpsc", iterations, mode) != 0) {
        return 1;
    }
    if ((scenario == SCENARIO_APPEND_FILE || scenario == SCENARIO_ALL) &&
        stumpless_run_scenario("append_file", iterations, mode) != 0) {
        return 1;
    }

    return 0;
}
