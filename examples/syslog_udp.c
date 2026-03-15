#include <iojournal/iojournal.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static int ij_bind_udp_listener(uint16_t *out_port)
{
    struct sockaddr_in address = {0};
    struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
    socklen_t address_len = sizeof(address);
    int fd = -1;

    if (out_port == NULL) {
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return -1;
    }
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

int main(void)
{
    uint16_t port = 0U;
    int listener_fd = -1;
    char buffer[512];
    ssize_t bytes_read;
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "example.syslog.udp",
        .message = "udp example payload",
        .logger = "examples.syslog_udp",
        .attributes = NULL,
        .attribute_count = 0U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "examples/syslog_udp.c",
        .source_line = 1U,
        .source_function = "main",
    };
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_SYSLOG,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "examples.syslog_udp",
        .file_path = NULL,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = "127.0.0.1",
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL4,
    };
    ij_logger_t *logger = NULL;

    listener_fd = ij_bind_udp_listener(&port);
    if (listener_fd < 0) {
        return 10;
    }
    config.syslog_port = port;

    if (ij_logger_init(&logger, &config) != IJ_STATUS_OK) {
        (void)close(listener_fd);
        return 1;
    }
    if (ij_logger_log(logger, &event) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        (void)close(listener_fd);
        return 2;
    }
    if (ij_logger_flush(logger) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        (void)close(listener_fd);
        return 3;
    }

    bytes_read = recv(listener_fd, buffer, sizeof(buffer) - 1U, 0);
    if (bytes_read <= 0) {
        (void)ij_logger_shutdown(logger);
        (void)close(listener_fd);
        return 4;
    }
    buffer[(size_t)bytes_read] = '\0';
    puts(buffer);

    if (ij_logger_shutdown(logger) != IJ_STATUS_OK) {
        (void)close(listener_fd);
        return 5;
    }
    if (close(listener_fd) != 0) {
        return 6;
    }

    return 0;
}
