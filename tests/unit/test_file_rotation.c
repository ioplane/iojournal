#include "ij_internal.h"

#include <unity/unity.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static ij_event_t ij_test_rotation_event(const char *event_name, const char *message)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-rotation-42",
                         .len = 15U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = event_name,
        .message = message,
        .logger = "tests.file_rotation",
        .attributes = attributes,
        .attribute_count = 1U,
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

static void ij_make_rotation_path(char *buffer, size_t buffer_size)
{
    static unsigned int counter = 0U;
    int written = snprintf(buffer, buffer_size, "/tmp/iojournal-file-rotation-%ld-%d-%u.ndjson",
                           (long)getpid(), (int)time(NULL), counter++);

    TEST_ASSERT_TRUE(written > 0);
    TEST_ASSERT_TRUE((size_t)written < buffer_size);
    if (unlink(buffer) != 0) {
        TEST_ASSERT_EQUAL_INT(ENOENT, errno);
    }
}

static size_t ij_count_path_prefix_matches(const char *path_prefix)
{
    const char *basename = strrchr(path_prefix, '/');
    DIR *directory = opendir("/tmp");
    struct dirent *entry;
    size_t matches = 0U;

    TEST_ASSERT_NOT_NULL(directory);
    basename = basename == NULL ? path_prefix : basename + 1;

    for (entry = readdir(directory); entry != NULL; entry = readdir(directory)) {
        if (strncmp(entry->d_name, basename, strlen(basename)) == 0) {
            matches += 1U;
        }
    }

    TEST_ASSERT_EQUAL_INT(0, closedir(directory));
    return matches;
}

static void ij_remove_path_matches(const char *path_prefix)
{
    const char *basename = strrchr(path_prefix, '/');
    DIR *directory = opendir("/tmp");
    struct dirent *entry;

    TEST_ASSERT_NOT_NULL(directory);
    basename = basename == NULL ? path_prefix : basename + 1;

    for (entry = readdir(directory); entry != NULL; entry = readdir(directory)) {
        if (strncmp(entry->d_name, basename, strlen(basename)) == 0) {
            char full_path[256];
            int written = snprintf(full_path, sizeof(full_path), "/tmp/%s", entry->d_name);

            TEST_ASSERT_TRUE(written > 0);
            TEST_ASSERT_TRUE((size_t)written < sizeof(full_path));
            TEST_ASSERT_EQUAL_INT(0, unlink(full_path));
        }
    }

    TEST_ASSERT_EQUAL_INT(0, closedir(directory));
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_file_sink_rotates_when_size_limit_is_exceeded(void)
{
    char path[128];
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;
    ij_event_t first_event = ij_test_rotation_event(
        "rotate.first", "0123456789012345678901234567890123456789012345678901234567890123456789");
    ij_event_t second_event = ij_test_rotation_event(
        "rotate.second",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");

    ij_make_rotation_path(path, sizeof(path));
    config = (ij_logger_config_t){
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "tests.file_rotation",
        .file_path = path,
        .file_rotate_bytes = 180U,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = 4U,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &first_event));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &second_event));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
    TEST_ASSERT_TRUE(ij_count_path_prefix_matches(path) >= 2U);

    ij_remove_path_matches(path);
}

void test_file_sink_rotates_when_rotation_interval_expires(void)
{
    char path[128];
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;
    ij_event_t first_event = ij_test_rotation_event("rotate.interval.first", "before interval");
    ij_event_t second_event = ij_test_rotation_event("rotate.interval.second", "after interval");

    ij_make_rotation_path(path, sizeof(path));
    config = (ij_logger_config_t){
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "tests.file_rotation",
        .file_path = path,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = 1U,
        .file_retention_files = 4U,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &first_event));
    logger->file_sink.last_rotation_epoch = 0;
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &second_event));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
    TEST_ASSERT_TRUE(ij_count_path_prefix_matches(path) >= 2U);

    ij_remove_path_matches(path);
}

void test_file_sink_retention_limits_rotated_file_count(void)
{
    char path[128];
    ij_logger_t *logger = NULL;
    ij_logger_config_t config;

    ij_make_rotation_path(path, sizeof(path));
    config = (ij_logger_config_t){
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "tests.file_rotation",
        .file_path = path,
        .file_rotate_bytes = 150U,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = 2U,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_init(&logger, &config));
    for (size_t i = 0U; i < 5U; ++i) {
        char event_name[32];
        char message[96];
        int name_written = snprintf(event_name, sizeof(event_name), "rotate.retention.%zu", i);
        int message_written =
            snprintf(message, sizeof(message),
                     "retention payload %zu 0123456789012345678901234567890123456789", i);
        ij_event_t event;

        TEST_ASSERT_TRUE(name_written > 0);
        TEST_ASSERT_TRUE((size_t)name_written < sizeof(event_name));
        TEST_ASSERT_TRUE(message_written > 0);
        TEST_ASSERT_TRUE((size_t)message_written < sizeof(message));
        event = ij_test_rotation_event(event_name, message);
        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_log(logger, &event));
    }
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_logger_shutdown(logger));
    TEST_ASSERT_EQUAL_size_t(3U, ij_count_path_prefix_matches(path));

    ij_remove_path_matches(path);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_file_sink_rotates_when_size_limit_is_exceeded);
    RUN_TEST(test_file_sink_rotates_when_rotation_interval_expires);
    RUN_TEST(test_file_sink_retention_limits_rotated_file_count);
    return UNITY_END();
}
