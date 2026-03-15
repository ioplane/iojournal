#include <iojournal/iojournal.h>

#include <unity/unity.h>

#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_version_api_returns_expected_semver(void)
{
    TEST_ASSERT_EQUAL_STRING("0.1.0", ij_version());
    TEST_ASSERT_EQUAL_HEX32(0x000100U, (uint32_t)ij_version_num());
}

static void test_shutdown_accepts_null(void)
{
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(NULL));
}

static void test_init_rejects_null_output_pointer(void)
{
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "surface.test",
        .file_path = NULL,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_USER,
    };

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_init(NULL, &config));
}

static void test_init_rejects_invalid_queue_capacity(void)
{
    ij_logger_t *logger = NULL;
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = 300U,
        .logger_name = "surface.test",
        .file_path = NULL,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_USER,
    };

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_init(&logger, &config));
    TEST_ASSERT_NULL(logger);
}

static void test_log_rejects_null_inputs(void)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 1, .nanoseconds = 0U},
        .level = IJ_LEVEL_INFO,
        .event_name = "startup",
        .message = "hello",
        .logger = NULL,
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

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_log(NULL, &event));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_version_api_returns_expected_semver);
    RUN_TEST(test_shutdown_accepts_null);
    RUN_TEST(test_init_rejects_null_output_pointer);
    RUN_TEST(test_init_rejects_invalid_queue_capacity);
    RUN_TEST(test_log_rejects_null_inputs);
    return UNITY_END();
}
