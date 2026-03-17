#include "ij_internal.h"

#include <unity/unity.h>

#include <stdio.h>
#include <string.h>

static ij_event_t ij_test_syslog_event(const char *logger_name, ij_level_t level,
                                       const char *message)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = level,
        .event_name = "syslog.emit",
        .message = message,
        .logger = logger_name,
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

static ij_logger_config_t ij_test_syslog_config(ij_syslog_facility_t facility,
                                                const char *logger_name)
{
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_TRACE,
        .sink_kind = IJ_SINK_KIND_SYSLOG,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = logger_name,
        .file_path = NULL,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = "127.0.0.1",
        .syslog_port = 514U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = facility,
    };

    return config;
}

static void ij_read_fixture(const char *path, char *buffer, size_t buffer_size)
{
    FILE *fixture = fopen(path, "rb");
    size_t bytes_read;
    int close_status;

    TEST_ASSERT_NOT_NULL(fixture);
    bytes_read = fread(buffer, 1U, buffer_size - 1U, fixture);
    while (bytes_read > 0U &&
           (buffer[bytes_read - 1U] == '\n' || buffer[bytes_read - 1U] == '\r')) {
        bytes_read -= 1U;
    }
    buffer[bytes_read] = '\0';
    close_status = fclose(fixture);
    TEST_ASSERT_EQUAL_INT(0, close_status);
}

static void ij_fixture_path(const char *relative_path, char *buffer, size_t buffer_size)
{
    int written;

    written = snprintf(buffer, buffer_size, "%s/%s", IOJOURNAL_SOURCE_DIR, relative_path);
    TEST_ASSERT_TRUE(written > 0);
    TEST_ASSERT_TRUE((size_t)written < buffer_size);
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_rfc5424_formatter_emits_canonical_message_for_info_local0(void)
{
    ij_event_t event = ij_test_syslog_event("tests.syslog", IJ_LEVEL_INFO, "service started");
    ij_event_copy_t copied_event = {0};
    ij_logger_config_t config = ij_test_syslog_config(IJ_SYSLOG_FACILITY_LOCAL0, "tests.default");
    char actual[512];
    char expected[512];
    char fixture_path[256];
    size_t output_size = 0U;

    ij_fixture_path("tests/fixtures/rfc5424/info-local0.golden", fixture_path,
                    sizeof(fixture_path));
    ij_read_fixture(fixture_path, expected, sizeof(expected));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_rfc5424_encode(&copied_event, &config, actual,
                                                          sizeof(actual), &output_size));
    TEST_ASSERT_EQUAL_STRING(expected, actual);
    TEST_ASSERT_EQUAL_size_t(strlen(expected), output_size);

    ij_event_copy_dispose(&copied_event);
}

void test_rfc5424_formatter_falls_back_to_logger_config_when_event_logger_missing(void)
{
    ij_event_t event = ij_test_syslog_event(NULL, IJ_LEVEL_FATAL, "hard failure");
    ij_event_copy_t copied_event = {0};
    ij_logger_config_t config = ij_test_syslog_config(IJ_SYSLOG_FACILITY_USER, "tests.default");
    char actual[512];
    char expected[512];
    char fixture_path[256];
    size_t output_size = 0U;

    ij_fixture_path("tests/fixtures/rfc5424/fatal-user-fallback-app.golden", fixture_path,
                    sizeof(fixture_path));
    ij_read_fixture(fixture_path, expected, sizeof(expected));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_rfc5424_encode(&copied_event, &config, actual,
                                                          sizeof(actual), &output_size));
    TEST_ASSERT_EQUAL_STRING(expected, actual);
    TEST_ASSERT_EQUAL_size_t(strlen(expected), output_size);

    ij_event_copy_dispose(&copied_event);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_rfc5424_formatter_emits_canonical_message_for_info_local0);
    RUN_TEST(test_rfc5424_formatter_falls_back_to_logger_config_when_event_logger_missing);
    return UNITY_END();
}
