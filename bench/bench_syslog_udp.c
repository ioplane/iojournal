/*
 * iojournal -- syslog UDP benchmark harness
 *
 * SPDX-License-Identifier: MIT
 */

#include <iojournal/iojournal.h>

#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

typedef enum {
    OUTPUT_HUMAN = 0,
    OUTPUT_TSV = 1,
} output_mode_t;

typedef struct {
    int listener_fd;
    size_t expected_messages;
    size_t received_messages;
    int recv_status;
} ij_udp_receiver_t;

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

static int ij_bind_udp_listener(uint16_t *out_port)
{
    struct sockaddr_in address = {0};
    struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
    int receive_buffer_bytes = 1024 * 1024;
    socklen_t address_len = sizeof(address);
    int fd;

    if (out_port == NULL) {
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return -1;
    }
    (void)setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &receive_buffer_bytes,
                     (socklen_t)sizeof(receive_buffer_bytes));
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
        (void)close(fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(0U);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(fd, (const struct sockaddr *)&address, sizeof(address)) != 0) {
        (void)close(fd);
        return -1;
    }
    if (getsockname(fd, (struct sockaddr *)&address, &address_len) != 0) {
        (void)close(fd);
        return -1;
    }

    *out_port = ntohs(address.sin_port);
    return fd;
}

static int ij_udp_receiver_main(void *arg)
{
    ij_udp_receiver_t *receiver = arg;
    char buffer[2048];

    if (receiver == NULL) {
        return 1;
    }

    while (receiver->received_messages < receiver->expected_messages) {
        ssize_t bytes_read = recv(receiver->listener_fd, buffer, sizeof(buffer), 0);

        if (bytes_read < 0) {
            receiver->recv_status = errno;
            return 1;
        }
        receiver->received_messages += 1U;
    }

    receiver->recv_status = 0;
    return 0;
}

static ij_event_t ij_make_event(void)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 1710000000, .nanoseconds = 789000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "bench.syslog_udp",
        .message = "benchmark payload for normalized RFC 5424 UDP loopback scenario",
        .logger = "bench.syslog_udp",
        .attributes = NULL,
        .attribute_count = 0U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "bench/bench_syslog_udp.c",
        .source_line = 1U,
        .source_function = "main",
    };

    return event;
}

static int ij_print_result(output_mode_t mode, size_t iterations, uint64_t elapsed_ns,
                           size_t received_messages)
{
    double ns_per_op = (double)elapsed_ns / (double)iterations;

    if (mode == OUTPUT_TSV) {
        puts("format\ttsv\tv1");
        printf("meta\titerations\t%zu\n", iterations);
        printf("meta\treceived_messages\t%zu\n", received_messages);
        puts("columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op");
        printf("bench_syslog_udp\tudp_loopback\t%zu\t%" PRIu64 "\t%.2f\n", iterations, elapsed_ns,
               ns_per_op);
        printf("meta\tstatus_sink\t%" PRIu64 "\n", ij_status_sink);
        return 0;
    }

    puts("iojournal benchmark: syslog udp");
    printf("iterations: %zu\n", iterations);
    printf("received-messages: %zu\n", received_messages);
    printf("udp_loopback       iterations=%-8zu elapsed=%12" PRIu64 " ns ns/op=%10.2f\n",
           iterations, elapsed_ns, ns_per_op);
    printf("status-sink: %" PRIu64 "\n", ij_status_sink);
    return 0;
}

int main(int argc, char **argv)
{
    size_t iterations = 5000U;
    output_mode_t mode = OUTPUT_HUMAN;
    uint16_t port = 0U;
    int listener_fd = -1;
    ij_udp_receiver_t receiver = {0};
    thrd_t receiver_thread;
    ij_event_t event = ij_make_event();
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;
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

    listener_fd = ij_bind_udp_listener(&port);
    if (listener_fd < 0) {
        return 1;
    }

    receiver.listener_fd = listener_fd;
    receiver.expected_messages = iterations;
    if (thrd_create(&receiver_thread, ij_udp_receiver_main, &receiver) != thrd_success) {
        (void)close(listener_fd);
        return 1;
    }

    config = (ij_logger_config_t){
        .min_level = IJ_LEVEL_TRACE,
        .sink_kind = IJ_SINK_KIND_SYSLOG,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "bench.syslog_udp",
        .file_path = NULL,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = "127.0.0.1",
        .syslog_port = port,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL4,
    };

    if (ij_logger_init(&logger, &config) != IJ_STATUS_OK) {
        (void)thrd_join(receiver_thread, NULL);
        (void)close(listener_fd);
        return 1;
    }

    start_ns = ij_monotonic_ns();
    for (size_t i = 0U; i < iterations; ++i) {
        ij_status_t status = ij_logger_log(logger, &event);

        ij_status_sink += (uint64_t)status;
        if (status != IJ_STATUS_OK) {
            (void)ij_logger_shutdown(logger);
            (void)thrd_join(receiver_thread, NULL);
            (void)close(listener_fd);
            return 1;
        }
    }
    if (ij_logger_flush(logger) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        (void)thrd_join(receiver_thread, NULL);
        (void)close(listener_fd);
        return 1;
    }
    elapsed_ns = ij_monotonic_ns() - start_ns;

    if (ij_logger_shutdown(logger) != IJ_STATUS_OK) {
        (void)thrd_join(receiver_thread, NULL);
        (void)close(listener_fd);
        return 1;
    }
    if (thrd_join(receiver_thread, NULL) != thrd_success) {
        (void)close(listener_fd);
        return 1;
    }
    if (close(listener_fd) != 0) {
        return 1;
    }
    if (receiver.recv_status != 0 || receiver.received_messages != iterations) {
        return 1;
    }

    return ij_print_result(mode, iterations, elapsed_ns, receiver.received_messages);
}
