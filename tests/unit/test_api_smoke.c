#include <iojournal/iojournal.h>

#include <unity/unity.h>

static ij_logger_config_t ij_test_console_config(void)
{
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "test.logger",
        .file_path = NULL,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };

    return config;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_shutdown_null_is_ok(void)
{
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(NULL));
}

void test_init_rejects_null_inputs(void)
{
    ij_logger_config_t config = ij_test_console_config();

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_init(NULL, &config));

    ij_logger_t *logger = NULL;
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_init(&logger, NULL));
    TEST_ASSERT_NULL(logger);
}

void test_init_accepts_minimal_console_config(void)
{
    ij_logger_config_t config = ij_test_console_config();
    ij_logger_t *logger = NULL;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    TEST_ASSERT_NOT_NULL(logger);
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_flush(logger));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
}

void test_file_sink_requires_path(void)
{
    ij_logger_config_t config = ij_test_console_config();
    ij_logger_t *logger = NULL;

    config.sink_kind = IJ_SINK_KIND_FILE;
    config.file_path = NULL;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_init(&logger, &config));
    TEST_ASSERT_NULL(logger);
}

void test_log_rejects_null_inputs(void)
{
    ij_logger_config_t config = ij_test_console_config();
    ij_logger_t *logger = NULL;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_log(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_logger_log(logger, NULL));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
}

void test_version_helpers_are_stable(void)
{
    TEST_ASSERT_EQUAL_STRING("0.1.0", ij_version());
    TEST_ASSERT_EQUAL_INT((0 << 16) | (1 << 8) | 0, ij_version_num());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_shutdown_null_is_ok);
    RUN_TEST(test_init_rejects_null_inputs);
    RUN_TEST(test_init_accepts_minimal_console_config);
    RUN_TEST(test_file_sink_requires_path);
    RUN_TEST(test_log_rejects_null_inputs);
    RUN_TEST(test_version_helpers_are_stable);
    return UNITY_END();
}
