/*
 * iojournal -- logger lifecycle scaffolding for Sprint 05 Task 1
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <stdlib.h>
#include <string.h>

bool ij_level_is_valid(ij_level_t level)
{
    return level >= IJ_LEVEL_TRACE && level <= IJ_LEVEL_FATAL;
}

bool ij_sink_kind_is_valid(ij_sink_kind_t sink_kind)
{
    return sink_kind >= IJ_SINK_KIND_CONSOLE && sink_kind <= IJ_SINK_KIND_SYSLOG;
}

bool ij_syslog_transport_is_valid(ij_syslog_transport_t transport)
{
    return transport == IJ_SYSLOG_TRANSPORT_UDP || transport == IJ_SYSLOG_TRANSPORT_TCP;
}

bool ij_syslog_facility_is_valid(ij_syslog_facility_t facility)
{
    return (facility >= IJ_SYSLOG_FACILITY_KERN && facility <= IJ_SYSLOG_FACILITY_FTP) ||
           (facility >= IJ_SYSLOG_FACILITY_LOCAL0 && facility <= IJ_SYSLOG_FACILITY_LOCAL7);
}

bool ij_redaction_mode_is_valid(ij_redaction_mode_t mode)
{
    return mode == IJ_REDACTION_MODE_DISABLED || mode == IJ_REDACTION_MODE_ENABLED;
}

static bool ij_is_power_of_two(size_t value)
{
    return value != 0U && (value & (value - 1U)) == 0U;
}

ij_status_t ij_validate_logger_config(const ij_logger_config_t *config)
{
    if (config == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (!ij_level_is_valid(config->min_level) || !ij_sink_kind_is_valid(config->sink_kind) ||
        !ij_redaction_mode_is_valid(config->redaction_mode)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    if (config->queue_capacity != 0U) {
        if (config->queue_capacity < IJ_QUEUE_CAPACITY_MIN ||
            config->queue_capacity > IJ_QUEUE_CAPACITY_MAX ||
            !ij_is_power_of_two(config->queue_capacity)) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
    }

    if (config->sink_kind == IJ_SINK_KIND_FILE) {
        if (config->file_path == NULL || config->file_path[0] == '\0') {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
    }

    if (config->sink_kind == IJ_SINK_KIND_SYSLOG) {
        if (!ij_syslog_transport_is_valid(config->syslog_transport) ||
            !ij_syslog_facility_is_valid(config->syslog_facility) || config->syslog_host == NULL ||
            config->syslog_host[0] == '\0' || config->syslog_port == 0U) {
            return IJ_STATUS_INVALID_ARGUMENT;
        }
    }

    return IJ_STATUS_OK;
}

static void ij_apply_logger_defaults(ij_logger_t *logger, const ij_logger_config_t *config)
{
    logger->config = *config;
    if (logger->config.queue_capacity == 0U) {
        logger->config.queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT;
    }
    logger->active = true;
}

ij_status_t ij_logger_init(ij_logger_t **out_logger, const ij_logger_config_t *config)
{
    ij_logger_t *logger;
    ij_status_t status;

    if (out_logger == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    *out_logger = NULL;
    status = ij_validate_logger_config(config);
    if (status != IJ_STATUS_OK) {
        return status;
    }

    logger = calloc(1U, sizeof(*logger));
    if (logger == NULL) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    ij_apply_logger_defaults(logger, config);
    status = ij_ring_init(&logger->ring, logger->config.queue_capacity);
    if (status != IJ_STATUS_OK) {
        free(logger);
        return status;
    }
    if (logger->config.sink_kind == IJ_SINK_KIND_FILE) {
        status = ij_file_sink_open(&logger->file_sink, logger->config.file_path);
        if (status != IJ_STATUS_OK) {
            ij_ring_destroy(&logger->ring);
            free(logger);
            return status;
        }
        logger->file_sink.rotate_bytes = logger->config.file_rotate_bytes;
        logger->file_sink.rotate_interval_seconds = logger->config.file_rotate_interval_seconds;
        logger->file_sink.retention_files = logger->config.file_retention_files;
    } else if (logger->config.sink_kind == IJ_SINK_KIND_SYSLOG) {
        if (logger->config.syslog_transport == IJ_SYSLOG_TRANSPORT_UDP) {
            status = ij_syslog_udp_open(&logger->syslog_sink, logger->config.syslog_host,
                                        logger->config.syslog_port);
        } else if (logger->config.syslog_transport == IJ_SYSLOG_TRANSPORT_TCP) {
            status = ij_syslog_tcp_open(&logger->syslog_sink, logger->config.syslog_host,
                                        logger->config.syslog_port);
        } else {
            status = IJ_STATUS_SINK_ERROR;
        }
        if (status != IJ_STATUS_OK) {
            ij_ring_destroy(&logger->ring);
            free(logger);
            return status;
        }
    }
    *out_logger = logger;
    return IJ_STATUS_OK;
}

ij_status_t ij_logger_shutdown(ij_logger_t *logger)
{
    if (logger == NULL) {
        return IJ_STATUS_OK;
    }

    if (!logger->active) {
        return IJ_STATUS_INVALID_STATE;
    }

    logger->active = false;
    ij_file_sink_close(&logger->file_sink);
    ij_syslog_sink_close(&logger->syslog_sink);
    ij_ring_destroy(&logger->ring);
    free(logger);
    return IJ_STATUS_OK;
}

ij_status_t ij_logger_flush(ij_logger_t *logger)
{
    if (logger == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (!logger->active) {
        return IJ_STATUS_INVALID_STATE;
    }

    if (logger->config.sink_kind == IJ_SINK_KIND_FILE) {
        return ij_file_sink_flush(&logger->file_sink);
    }

    return IJ_STATUS_OK;
}

ij_status_t ij_logger_log(ij_logger_t *logger, const ij_event_t *event)
{
    ij_event_copy_t copied_event = {0};
    ij_event_copy_t emitted_event = {0};
    char payload[8192];
    size_t payload_len = 0U;
    ij_status_t status;

    if (logger == NULL || event == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (!logger->active) {
        return IJ_STATUS_INVALID_STATE;
    }
    if (!ij_level_is_enabled(logger->config.min_level, event->level)) {
        return IJ_STATUS_OK;
    }

    status = ij_event_copy_from_input(&copied_event, event, logger->config.redaction_mode);
    if (status != IJ_STATUS_OK) {
        return status;
    }
    status = ij_ring_enqueue(&logger->ring, &copied_event);
    if (status != IJ_STATUS_OK) {
        ij_event_copy_dispose(&copied_event);
        return status;
    }
    if (!ij_ring_try_dequeue(&logger->ring, &emitted_event)) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    switch (logger->config.sink_kind) {
    case IJ_SINK_KIND_CONSOLE:
        status = ij_json_console_encode(&emitted_event, payload, sizeof(payload), &payload_len);
        if (status == IJ_STATUS_OK) {
            status = ij_console_sink_write(payload, payload_len);
        }
        break;
    case IJ_SINK_KIND_FILE:
        status = ij_ndjson_encode(&emitted_event, payload, sizeof(payload), &payload_len);
        if (status == IJ_STATUS_OK) {
            status = ij_file_sink_write(&logger->file_sink, payload, payload_len);
        }
        break;
    case IJ_SINK_KIND_SYSLOG:
        status = ij_rfc5424_encode(&emitted_event, &logger->config, payload, sizeof(payload),
                                   &payload_len);
        if (status == IJ_STATUS_OK) {
            if (logger->config.syslog_transport == IJ_SYSLOG_TRANSPORT_UDP) {
                status = ij_syslog_udp_write(&logger->syslog_sink, payload, payload_len);
            } else if (logger->config.syslog_transport == IJ_SYSLOG_TRANSPORT_TCP) {
                status = ij_syslog_tcp_write(&logger->syslog_sink, payload, payload_len);
            } else {
                status = IJ_STATUS_SINK_ERROR;
            }
        }
        break;
    default:
        status = IJ_STATUS_INVALID_ARGUMENT;
        break;
    }

    ij_event_copy_dispose(&emitted_event);
    return status;
}
