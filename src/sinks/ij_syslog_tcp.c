/*
 * iojournal -- RFC 6587 TCP syslog sink
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static ij_status_t ij_syslog_connect_tcp(const char *host, uint16_t port, int *out_fd)
{
    struct addrinfo hints = {0};
    struct addrinfo *result = NULL;
    struct addrinfo *entry = NULL;
    char service[6];
    int socket_fd = -1;
    int written;

    if (host == NULL || out_fd == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(service, sizeof(service), "%u", (unsigned int)port);
    if (written <= 0 || (size_t)written >= sizeof(service)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, service, &hints, &result) != 0) {
        return IJ_STATUS_SINK_ERROR;
    }

    for (entry = result; entry != NULL; entry = entry->ai_next) {
        socket_fd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }
        if (connect(socket_fd, entry->ai_addr, entry->ai_addrlen) == 0) {
            *out_fd = socket_fd;
            freeaddrinfo(result);
            return IJ_STATUS_OK;
        }

        close(socket_fd);
    }

    freeaddrinfo(result);
    return IJ_STATUS_SINK_ERROR;
}

static ij_status_t ij_syslog_send_all(int fd, const char *buffer, size_t buffer_len)
{
    size_t total_sent = 0U;

    while (total_sent < buffer_len) {
        ssize_t bytes_sent = send(fd, buffer + total_sent, buffer_len - total_sent, 0);

        if (bytes_sent <= 0) {
            return IJ_STATUS_SINK_ERROR;
        }
        total_sent += (size_t)bytes_sent;
    }

    return IJ_STATUS_OK;
}

ij_status_t ij_syslog_tcp_open(ij_syslog_sink_t *sink, const char *host, uint16_t port)
{
    if (sink == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    sink->fd = -1;
    sink->address_len = 0;
    memset(&sink->address, 0, sizeof(sink->address));

    return ij_syslog_connect_tcp(host, port, &sink->fd);
}

ij_status_t ij_syslog_tcp_write(ij_syslog_sink_t *sink, const char *payload, size_t payload_len)
{
    char frame_prefix[32];
    int prefix_len;
    ij_status_t status;

    if (sink == NULL || payload == NULL || payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (sink->fd < 0) {
        return IJ_STATUS_INVALID_STATE;
    }

    prefix_len = snprintf(frame_prefix, sizeof(frame_prefix), "%zu ", payload_len);
    if (prefix_len <= 0 || (size_t)prefix_len >= sizeof(frame_prefix)) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    status = ij_syslog_send_all(sink->fd, frame_prefix, (size_t)prefix_len);
    if (status != IJ_STATUS_OK) {
        return status;
    }

    return ij_syslog_send_all(sink->fd, payload, payload_len);
}
