#include <iojournal/iojournal.h>

#include <unity/unity.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static ij_event_t ij_test_tcp_event(void)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_ERROR,
        .event_name = "syslog.tcp",
        .message = "tcp payload",
        .logger = "tests.syslog.tcp",
        .attributes = NULL,
        .attribute_count = 0U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = NULL,
        .source_line = 0U,
        .source_function = NULL,
    };

    return event;
}

static int ij_create_tcp_listener(uint16_t *out_port)
{
    struct sockaddr_in address = {0};
    socklen_t address_len = sizeof(address);
    int fd = -1;
    int yes = 1;

    TEST_ASSERT_NOT_NULL(out_port);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    TEST_ASSERT_TRUE(fd >= 0);
    TEST_ASSERT_EQUAL_INT(0, setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)));

    address.sin_family = AF_INET;
    address.sin_port = htons(0U);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    TEST_ASSERT_EQUAL_INT(0, bind(fd, (const struct sockaddr *)&address, sizeof(address)));
    TEST_ASSERT_EQUAL_INT(0, getsockname(fd, (struct sockaddr *)&address, &address_len));
    TEST_ASSERT_EQUAL_size_t(sizeof(address), address_len);
    TEST_ASSERT_EQUAL_INT(0, listen(fd, 1));

    *out_port = ntohs(address.sin_port);
    return fd;
}

static int ij_accept_tcp_peer(int listener_fd)
{
    struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
    int peer_fd = -1;

    TEST_ASSERT_EQUAL_INT(0, setsockopt(listener_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                                        sizeof(timeout)));
    peer_fd = accept(listener_fd, NULL, NULL);
    TEST_ASSERT_TRUE(peer_fd >= 0);
    TEST_ASSERT_EQUAL_INT(0,
                          setsockopt(peer_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));

    return peer_fd;
}

static ij_logger_config_t ij_test_tcp_config(uint16_t port)
{
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_TRACE,
        .sink_kind = IJ_SINK_KIND_SYSLOG,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "tests.syslog.default",
        .file_path = NULL,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = "127.0.0.1",
        .syslog_port = port,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_TCP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL1,
    };

    return config;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_syslog_tcp_sink_emits_octet_counted_frame(void)
{
    uint16_t port = 0U;
    int listener_fd = -1;
    int peer_fd = -1;
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;
    ij_event_t event = ij_test_tcp_event();
    const char *payload = "<139>1 1970-01-01T00:00:00.123Z - tests.syslog.tcp - - - tcp payload";
    char expected[512];
    char actual[512];
    int written;
    ssize_t bytes_read;

    listener_fd = ij_create_tcp_listener(&port);
    config = ij_test_tcp_config(port);

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    peer_fd = ij_accept_tcp_peer(listener_fd);

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &event));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_flush(logger));

    written = snprintf(expected, sizeof(expected), "%zu %s", strlen(payload), payload);
    TEST_ASSERT_TRUE(written > 0);
    TEST_ASSERT_TRUE((size_t)written < sizeof(expected));

    bytes_read = recv(peer_fd, actual, sizeof(actual) - 1U, 0);
    TEST_ASSERT_TRUE_MESSAGE(bytes_read > 0, "expected one TCP syslog frame");
    actual[(size_t)bytes_read] = '\0';
    TEST_ASSERT_EQUAL_STRING(expected, actual);

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
    TEST_ASSERT_EQUAL_INT(0, close(peer_fd));
    TEST_ASSERT_EQUAL_INT(0, close(listener_fd));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_syslog_tcp_sink_emits_octet_counted_frame);
    return UNITY_END();
}
