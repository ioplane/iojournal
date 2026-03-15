#include <iojournal/iojournal.h>

#include <stdint.h>

int main(void)
{
    static const uint8_t trace_id[IJ_TRACE_ID_SIZE] = {
        0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
        0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
    };
    static const uint8_t span_id[IJ_SPAN_ID_SIZE] = {
        0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
    };
    ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-42",
                         .len = 6U,
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
        .event_name = "example.start",
        .message = "example emitted one console event",
        .logger = "examples.basic_console",
        .attributes = attributes,
        .attribute_count = 2U,
        .trace_id = trace_id,
        .span_id = span_id,
        .trace_flags = 0x01U,
        .has_trace_context = true,
        .source_file = "examples/basic_console.c",
        .source_line = 1U,
        .source_function = "main",
    };
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "examples.basic_console",
        .file_path = NULL,
        .syslog_host = NULL,
        .syslog_port = 0U,
        .syslog_transport = IJ_SYSLOG_TRANSPORT_UDP,
        .syslog_facility = IJ_SYSLOG_FACILITY_LOCAL0,
    };
    ij_logger_t *logger = NULL;

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

    return 0;
}
