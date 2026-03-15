/*
 * iojournal -- internal runtime header
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_IJ_INTERNAL_H
#define IOJOURNAL_IJ_INTERNAL_H

#include <iojournal/iojournal.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <threads.h>
#include <time.h>

#define IJ_VERSION_MAJOR  0
#define IJ_VERSION_MINOR  1
#define IJ_VERSION_PATCH  0
#define IJ_VERSION_STRING "0.1.0"

typedef struct {
    char *data;
    size_t len;
} ij_owned_string_t;

typedef struct {
    ij_attr_value_kind_t kind;
    union {
        ij_owned_string_t string;
        int64_t signed_value;
        uint64_t unsigned_value;
        bool bool_value;
        double double_value;
    } as;
} ij_owned_attr_value_t;

typedef struct {
    ij_owned_string_t key;
    ij_owned_attr_value_t value;
} ij_owned_attr_t;

typedef struct {
    ij_timestamp_t timestamp;
    ij_level_t level;
    ij_owned_string_t event_name;
    ij_owned_string_t message;
    ij_owned_string_t logger_name;
    ij_owned_attr_t *attributes;
    size_t attribute_count;
    uint8_t trace_id[IJ_TRACE_ID_SIZE];
    uint8_t span_id[IJ_SPAN_ID_SIZE];
    uint8_t trace_flags;
    bool has_trace_context;
    ij_owned_string_t source_file;
    uint32_t source_line;
    ij_owned_string_t source_function;
} ij_event_copy_t;

typedef struct {
    size_t capacity;
    size_t mask;
    _Atomic size_t head;
    _Atomic size_t tail;
    _Atomic size_t count;
    _Atomic size_t dropped_events;
    _Atomic size_t enqueued_events;
    _Atomic size_t high_water_mark;
    mtx_t lock;
    ij_event_copy_t *slots;
    bool *occupied;
} ij_ring_t;

typedef struct {
    char *path;
    FILE *stream;
    size_t rotate_bytes;
    uint32_t rotate_interval_seconds;
    size_t retention_files;
    size_t bytes_written;
    uint64_t rotation_sequence;
    time_t last_rotation_epoch;
    char **rotated_paths;
    size_t rotated_path_count;
} ij_file_sink_t;

typedef struct {
    int fd;
    struct sockaddr_storage address;
    socklen_t address_len;
} ij_syslog_sink_t;

struct ij_logger {
    ij_logger_config_t config;
    bool active;
    ij_ring_t ring;
    ij_file_sink_t file_sink;
    ij_syslog_sink_t syslog_sink;
};

bool ij_level_is_valid(ij_level_t level);
bool ij_sink_kind_is_valid(ij_sink_kind_t sink_kind);
bool ij_syslog_transport_is_valid(ij_syslog_transport_t transport);
bool ij_syslog_facility_is_valid(ij_syslog_facility_t facility);
bool ij_redaction_mode_is_valid(ij_redaction_mode_t mode);
bool ij_level_is_enabled(ij_level_t min_level, ij_level_t event_level);
const char *ij_level_to_text(ij_level_t level);
bool ij_key_should_redact(const char *key);
void ij_redact_owned_attr_value(const char *key, ij_owned_attr_value_t *value);
ij_status_t ij_validate_logger_config(const ij_logger_config_t *config);
ij_status_t ij_validate_event(const ij_event_t *event);
ij_status_t ij_event_copy_from_input(ij_event_copy_t *out_event, const ij_event_t *event,
                                     ij_redaction_mode_t redaction_mode);
void ij_event_copy_dispose(ij_event_copy_t *event);
ij_status_t ij_ring_init(ij_ring_t *ring, size_t capacity);
void ij_ring_destroy(ij_ring_t *ring);
ij_status_t ij_ring_enqueue(ij_ring_t *ring, ij_event_copy_t *event);
bool ij_ring_try_dequeue(ij_ring_t *ring, ij_event_copy_t *out_event);
ij_status_t ij_format_timestamp_rfc3339(ij_timestamp_t timestamp, char *buffer, size_t buffer_size);
ij_status_t ij_json_console_encode(const ij_event_copy_t *event, char *buffer, size_t buffer_size,
                                   size_t *out_len);
ij_status_t ij_ndjson_encode(const ij_event_copy_t *event, char *buffer, size_t buffer_size,
                             size_t *out_len);
ij_status_t ij_rfc5424_encode(const ij_event_copy_t *event, const ij_logger_config_t *config,
                              char *buffer, size_t buffer_size, size_t *out_len);
ij_status_t ij_console_sink_write(const char *payload, size_t payload_len);
ij_status_t ij_file_sink_open(ij_file_sink_t *sink, const char *path);
ij_status_t ij_file_sink_write(ij_file_sink_t *sink, const char *payload, size_t payload_len);
ij_status_t ij_file_sink_flush(ij_file_sink_t *sink);
void ij_file_sink_close(ij_file_sink_t *sink);
ij_status_t ij_syslog_udp_open(ij_syslog_sink_t *sink, const char *host, uint16_t port);
ij_status_t ij_syslog_udp_write(ij_syslog_sink_t *sink, const char *payload, size_t payload_len);
ij_status_t ij_syslog_tcp_open(ij_syslog_sink_t *sink, const char *host, uint16_t port);
ij_status_t ij_syslog_tcp_write(ij_syslog_sink_t *sink, const char *payload, size_t payload_len);
void ij_syslog_sink_close(ij_syslog_sink_t *sink);

#endif /* IOJOURNAL_IJ_INTERNAL_H */
