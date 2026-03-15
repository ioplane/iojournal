#include <iojournal/iojournal.h>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    char path[256];
    int written =
        snprintf(path, sizeof(path), "/tmp/iojournal-example-file-sink-%ld.ndjson", (long)getpid());
    ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-example-42",
                         .len = 14U,
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
        .event_name = "example.file",
        .message = "example emitted one file event",
        .logger = "examples.file_sink",
        .attributes = attributes,
        .attribute_count = 2U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "examples/file_sink.c",
        .source_line = 1U,
        .source_function = "main",
    };
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_FILE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "examples.file_sink",
        .file_path = path,
        .file_rotate_bytes = IJ_FILE_ROTATE_BYTES_OFF,
        .file_rotate_interval_seconds = IJ_FILE_ROTATE_INTERVAL_OFF,
        .file_retention_files = IJ_FILE_RETENTION_OFF,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };
    ij_logger_t *logger = NULL;

    if (written <= 0 || (size_t)written >= sizeof(path)) {
        return 10;
    }
    if (ij_logger_init(&logger, &config) != IJ_STATUS_OK) {
        return 1;
    }
    if (ij_logger_log(logger, &event) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        return 2;
    }
    if (ij_logger_flush(logger) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        return 3;
    }
    if (ij_logger_shutdown(logger) != IJ_STATUS_OK) {
        return 4;
    }

    puts(path);
    return 0;
}
