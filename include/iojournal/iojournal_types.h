/*
 * iojournal -- public type system for the RC logging API
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_IOJOURNAL_TYPES_H
#define IOJOURNAL_IOJOURNAL_TYPES_H

#include <iojournal/iojournal_perf.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IJ_TRACE_ID_SIZE            16U
#define IJ_SPAN_ID_SIZE             8U
#define IJ_EVENT_NAME_MAX_LEN       64U
#define IJ_LOGGER_NAME_MAX_LEN      64U
#define IJ_ATTRIBUTE_KEY_MAX_LEN    64U
#define IJ_ATTRIBUTE_STRING_MAX_LEN 512U
#define IJ_ATTRIBUTE_COUNT_MAX      32U
#define IJ_MESSAGE_MAX_LEN          2048U
#define IJ_EVENT_TEXT_MAX_LEN       4096U
#define IJ_QUEUE_CAPACITY_MIN       256U
#define IJ_QUEUE_CAPACITY_DEFAULT   1024U
#define IJ_QUEUE_CAPACITY_MAX       65536U
#define IJ_FILE_ROTATE_BYTES_OFF    0U
#define IJ_FILE_ROTATE_INTERVAL_OFF 0U
#define IJ_FILE_RETENTION_OFF       0U

typedef enum {
    IJ_STATUS_OK = 0,
    IJ_STATUS_INVALID_ARGUMENT = 1,
    IJ_STATUS_INVALID_STATE = 2,
    IJ_STATUS_QUEUE_FULL = 3,
    IJ_STATUS_ENCODE_ERROR = 4,
    IJ_STATUS_SINK_ERROR = 5,
    IJ_STATUS_INTERNAL_ERROR = 6
} ij_status_t;

typedef enum {
    IJ_ERROR_NONE = 0,
    IJ_ERROR_ARGUMENT_NULL = 1,
    IJ_ERROR_ARGUMENT_ENUM = 2,
    IJ_ERROR_FIELD_LENGTH = 3,
    IJ_ERROR_ATTRIBUTE_COUNT = 4,
    IJ_ERROR_RESERVED_COLLISION = 5,
    IJ_ERROR_INVALID_UTF8 = 6,
    IJ_ERROR_INVALID_TIMESTAMP = 7,
    IJ_ERROR_QUEUE_FULL = 8,
    IJ_ERROR_FLUSH_TIMEOUT = 9,
    IJ_ERROR_ENCODE_JSON = 10,
    IJ_ERROR_ENCODE_SYSLOG = 11,
    IJ_ERROR_SINK_WRITE = 12,
    IJ_ERROR_SINK_DISCONNECTED = 13,
    IJ_ERROR_THREAD_START = 14,
    IJ_ERROR_INTERNAL_INVARIANT = 15
} ij_error_code_t;

typedef enum {
    IJ_LEVEL_TRACE = 0,
    IJ_LEVEL_DEBUG = 1,
    IJ_LEVEL_INFO = 2,
    IJ_LEVEL_WARN = 3,
    IJ_LEVEL_ERROR = 4,
    IJ_LEVEL_FATAL = 5
} ij_level_t;

typedef enum {
    IJ_SINK_KIND_CONSOLE = 0,
    IJ_SINK_KIND_FILE = 1,
    IJ_SINK_KIND_SYSLOG = 2
} ij_sink_kind_t;

typedef enum {
    IJ_SYSLOG_TRANSPORT_UDP = 0,
    IJ_SYSLOG_TRANSPORT_TCP = 1
} ij_syslog_transport_t;

typedef enum {
    IJ_SYSLOG_FACILITY_KERN = 0,
    IJ_SYSLOG_FACILITY_USER = 1,
    IJ_SYSLOG_FACILITY_MAIL = 2,
    IJ_SYSLOG_FACILITY_DAEMON = 3,
    IJ_SYSLOG_FACILITY_AUTH = 4,
    IJ_SYSLOG_FACILITY_SYSLOG = 5,
    IJ_SYSLOG_FACILITY_LPR = 6,
    IJ_SYSLOG_FACILITY_NEWS = 7,
    IJ_SYSLOG_FACILITY_UUCP = 8,
    IJ_SYSLOG_FACILITY_CRON = 9,
    IJ_SYSLOG_FACILITY_AUTHPRIV = 10,
    IJ_SYSLOG_FACILITY_FTP = 11,
    IJ_SYSLOG_FACILITY_LOCAL0 = 16,
    IJ_SYSLOG_FACILITY_LOCAL1 = 17,
    IJ_SYSLOG_FACILITY_LOCAL2 = 18,
    IJ_SYSLOG_FACILITY_LOCAL3 = 19,
    IJ_SYSLOG_FACILITY_LOCAL4 = 20,
    IJ_SYSLOG_FACILITY_LOCAL5 = 21,
    IJ_SYSLOG_FACILITY_LOCAL6 = 22,
    IJ_SYSLOG_FACILITY_LOCAL7 = 23
} ij_syslog_facility_t;

typedef enum {
    IJ_REDACTION_MODE_DISABLED = 0,
    IJ_REDACTION_MODE_ENABLED = 1
} ij_redaction_mode_t;

typedef enum {
    IJ_ATTR_VALUE_STRING = 0,
    IJ_ATTR_VALUE_SIGNED = 1,
    IJ_ATTR_VALUE_UNSIGNED = 2,
    IJ_ATTR_VALUE_BOOL = 3,
    IJ_ATTR_VALUE_DOUBLE = 4
} ij_attr_value_kind_t;

typedef struct {
    int64_t unix_seconds;
    uint32_t nanoseconds;
} ij_timestamp_t;

typedef struct ij_logger ij_logger_t;

typedef struct {
    ij_attr_value_kind_t kind;
    union {
        struct {
            const char *data;
            size_t len;
        } string;
        int64_t signed_value;
        uint64_t unsigned_value;
        bool bool_value;
        double double_value;
    } as;
} ij_attr_value_t;

typedef struct {
    const char *key;
    ij_attr_value_t value;
} ij_attr_t;

typedef struct {
    ij_timestamp_t timestamp;
    ij_level_t level;
    const char *event_name;
    const char *message;
    const char *logger;
    const ij_attr_t *attributes;
    size_t attribute_count;
    const uint8_t *trace_id;
    const uint8_t *span_id;
    uint8_t trace_flags;
    bool has_trace_context;
    const char *source_file;
    uint32_t source_line;
    const char *source_function;
} ij_event_t;

typedef struct {
    ij_level_t min_level;
    ij_sink_kind_t sink_kind;
    ij_redaction_mode_t redaction_mode;
    size_t queue_capacity;
    const char *logger_name;
    const char *file_path;
    size_t file_rotate_bytes;
    uint32_t file_rotate_interval_seconds;
    size_t file_retention_files;
    ij_file_backend_t file_backend;
    const char *syslog_host;
    uint16_t syslog_port;
    ij_syslog_transport_t syslog_transport;
    ij_syslog_facility_t syslog_facility;
} ij_logger_config_t;

#ifdef __cplusplus
}
#endif

#endif /* IOJOURNAL_IOJOURNAL_TYPES_H */
