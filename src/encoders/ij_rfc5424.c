/*
 * iojournal -- RFC 5424 formatter
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <stdio.h>
#include <string.h>

static int ij_syslog_severity_from_level(ij_level_t level)
{
    switch (level) {
    case IJ_LEVEL_FATAL:
        return 2;
    case IJ_LEVEL_ERROR:
        return 3;
    case IJ_LEVEL_WARN:
        return 4;
    case IJ_LEVEL_INFO:
        return 6;
    case IJ_LEVEL_DEBUG:
    case IJ_LEVEL_TRACE:
        return 7;
    default:
        return -1;
    }
}

static const char *ij_syslog_app_name(const ij_event_copy_t *event,
                                      const ij_logger_config_t *config)
{
    if (event->logger_name.data != NULL && event->logger_name.data[0] != '\0') {
        return event->logger_name.data;
    }
    if (config->logger_name != NULL && config->logger_name[0] != '\0') {
        return config->logger_name;
    }

    return "-";
}

#define IJ_RFC5424_APP_NAME_MAX 48U

static void ij_syslog_sanitize_app_name(const char *raw, char *out, size_t out_size)
{
    size_t i = 0U;
    size_t raw_len = strlen(raw);

    if (raw_len > IJ_RFC5424_APP_NAME_MAX) {
        raw_len = IJ_RFC5424_APP_NAME_MAX;
    }
    if (raw_len >= out_size) {
        raw_len = out_size - 1U;
    }

    for (; i < raw_len; i++) {
        unsigned char c = (unsigned char)raw[i];
        out[i] = (c >= 33U && c <= 126U) ? (char)c : '_';
    }
    out[i] = '\0';
}

ij_status_t ij_rfc5424_encode(const ij_event_copy_t *event, const ij_logger_config_t *config,
                              char *buffer, size_t buffer_size, size_t *out_len)
{
    char timestamp[32];
    char safe_app_name[IJ_RFC5424_APP_NAME_MAX + 1U];
    const char *message = NULL;
    int severity;
    unsigned int pri;
    int written;

    if (event == NULL || config == NULL || buffer == NULL || buffer_size == 0U || out_len == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    severity = ij_syslog_severity_from_level(event->level);
    if (severity < 0) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    if (ij_format_timestamp_rfc3339(event->timestamp, timestamp, sizeof(timestamp)) !=
        IJ_STATUS_OK) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    pri = ((unsigned int)config->syslog_facility * 8U) + (unsigned int)severity;
    ij_syslog_sanitize_app_name(ij_syslog_app_name(event, config), safe_app_name,
                                sizeof(safe_app_name));
    message = event->message.data != NULL ? event->message.data : "";

    if (message[0] == '\0') {
        written = snprintf(buffer, buffer_size, "<%u>%u %s - %s - - -", pri,
                           IJ_SYSLOG_RFC5424_VERSION, timestamp, safe_app_name);
    } else {
        written = snprintf(buffer, buffer_size, "<%u>%u %s - %s - - - %s", pri,
                           IJ_SYSLOG_RFC5424_VERSION, timestamp, safe_app_name, message);
    }

    if (written < 0 || (size_t)written >= buffer_size) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    *out_len = (size_t)written;
    return IJ_STATUS_OK;
}
