#include <iojournal/iojournal.h>

#include "ij_internal.h"

#include <unity/unity.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void ij_make_temp_path(char *buffer, size_t buffer_size)
{
    static unsigned int counter = 0U;
    int written;

    TEST_ASSERT_TRUE(buffer_size > 32U);
    TEST_ASSERT_NOT_NULL(buffer);

    written = snprintf(buffer, buffer_size, "/tmp/iojournal-file-uring-%ld-%d-%u.ndjson",
                       (long)getpid(), (int)time(NULL), counter++);
    TEST_ASSERT_TRUE(written > 0);
    TEST_ASSERT_TRUE((size_t)written < buffer_size);
    if (unlink(buffer) != 0) {
        TEST_ASSERT_EQUAL_INT(ENOENT, errno);
    }
}

static ij_logger_config_t ij_test_file_config(const char *path)
{
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "tests.file_sink_uring",
        .file_path = path,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .file_backend = IJ_FILE_BACKEND_IO_URING,
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

void test_explicit_file_backend_selection_uses_available_backend_when_requested(void)
{
    char path[128];
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;

    ij_make_temp_path(path, sizeof(path));
    config = ij_test_file_config(path);

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    TEST_ASSERT_TRUE(logger->file_sink.active_backend == IJ_FILE_BACKEND_IO_URING ||
                     logger->file_sink.active_backend == IJ_FILE_BACKEND_SYNC);
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
    TEST_ASSERT_EQUAL_INT(0, unlink(path));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_explicit_file_backend_selection_uses_available_backend_when_requested);
    return UNITY_END();
}
