#include <iojournal/iojournal.h>

#include <unity/unity.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static ij_event_t ij_test_file_event(const char *event_name, const char *message)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-file-42",
                         .len = 11U,
                     },
             }},
        {.key = "auth.token",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "secret-token",
                         .len = 12U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = event_name,
        .message = message,
        .logger = "tests.file_sink",
        .attributes = attributes,
        .attribute_count = 2U,
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

static ij_logger_config_t ij_test_file_config(const char *path)
{
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "tests.file_sink",
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

    return config;
}

static void ij_make_temp_path(char *buffer, size_t buffer_size)
{
    static unsigned int counter = 0U;
    int written;

    TEST_ASSERT_TRUE(buffer_size > 32U);
    TEST_ASSERT_NOT_NULL(buffer);

    written = snprintf(buffer, buffer_size, "/tmp/iojournal-file-sink-%ld-%d-%u.ndjson",
                       (long)getpid(), (int)time(NULL), counter++);
    TEST_ASSERT_TRUE(written > 0);
    TEST_ASSERT_TRUE((size_t)written < buffer_size);
    if (unlink(buffer) != 0) {
        TEST_ASSERT_EQUAL_INT(ENOENT, errno);
    }
}

static void ij_read_file(const char *path, char *buffer, size_t buffer_size)
{
    FILE *file = fopen(path, "rb");
    size_t bytes_read;
    int close_result;

    TEST_ASSERT_NOT_NULL(file);
    bytes_read = fread(buffer, 1U, buffer_size - 1U, file);
    buffer[bytes_read] = '\0';
    close_result = fclose(file);
    TEST_ASSERT_EQUAL_INT(0, close_result);
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_file_sink_writes_append_only_ndjson_output(void)
{
    char path[128];
    char file_data[2048];
    const char *expected_output =
        "{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
        "\"level\":\"info\","
        "\"event_name\":\"file.first\","
        "\"message\":\"first line\","
        "\"logger\":\"tests.file_sink\","
        "\"attributes\":{\"request_id\":\"req-file-42\",\"auth.token\":\"[REDACTED]\"}}\n"
        "{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
        "\"level\":\"info\","
        "\"event_name\":\"file.second\","
        "\"message\":\"second line\","
        "\"logger\":\"tests.file_sink\","
        "\"attributes\":{\"request_id\":\"req-file-42\",\"auth.token\":\"[REDACTED]\"}}\n";
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;
    ij_event_t first_event;
    ij_event_t second_event;

    ij_make_temp_path(path, sizeof(path));
    config = ij_test_file_config(path);
    first_event = ij_test_file_event("file.first", "first line");
    second_event = ij_test_file_event("file.second", "second line");

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &first_event));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &second_event));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_flush(logger));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));

    ij_read_file(path, file_data, sizeof(file_data));
    TEST_ASSERT_EQUAL_STRING(expected_output, file_data);
    {
        const char *last_newline = strrchr(file_data, '\n');

        if (file_data[0] == '\0') {
            TEST_FAIL_MESSAGE("file sink produced empty output");
        }
        TEST_ASSERT_NOT_NULL(last_newline);
        TEST_ASSERT_EQUAL_CHAR('\0', last_newline[1]);
    }

    {
        size_t newline_count = 0U;

        for (size_t i = 0U; file_data[i] != '\0'; ++i) {
            if (file_data[i] == '\n') {
                newline_count += 1U;
            }
        }
        TEST_ASSERT_EQUAL_size_t(2U, newline_count);
    }

    TEST_ASSERT_EQUAL_INT(0, unlink(path));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_file_sink_writes_append_only_ndjson_output);
    return UNITY_END();
}
