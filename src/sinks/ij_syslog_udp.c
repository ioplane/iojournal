/*
 * iojournal -- RFC 5426 UDP syslog sink
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static ij_status_t ij_syslog_resolve_udp(const char *host, uint16_t port,
                                         struct sockaddr_storage *out_address,
                                         socklen_t *out_address_len, int *out_fd)
{
    struct addrinfo hints = {0};
    struct addrinfo *result = NULL;
    struct addrinfo *entry = NULL;
    char service[6];
    int socket_fd = -1;
    int written;

    if (host == NULL || out_address == NULL || out_address_len == NULL || out_fd == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(service, sizeof(service), "%u", (unsigned int)port);
    if (written <= 0 || (size_t)written >= sizeof(service)) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(host, service, &hints, &result) != 0) {
        return IJ_STATUS_SINK_ERROR;
    }

    for (entry = result; entry != NULL; entry = entry->ai_next) {
        socket_fd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }

        memcpy(out_address, entry->ai_addr, entry->ai_addrlen);
        *out_address_len = (socklen_t)entry->ai_addrlen;
        *out_fd = socket_fd;
        freeaddrinfo(result);
        return IJ_STATUS_OK;
    }

    freeaddrinfo(result);
    return IJ_STATUS_SINK_ERROR;
}

ij_status_t ij_syslog_udp_open(ij_syslog_sink_t *sink, const char *host, uint16_t port)
{
    if (sink == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    sink->fd = -1;
    sink->address_len = 0;
    memset(&sink->address, 0, sizeof(sink->address));

    return ij_syslog_resolve_udp(host, port, &sink->address, &sink->address_len, &sink->fd);
}

ij_status_t ij_syslog_udp_write(ij_syslog_sink_t *sink, const char *payload, size_t payload_len)
{
    ssize_t bytes_sent;

    if (sink == NULL || payload == NULL || payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (sink->fd < 0 || sink->address_len == 0U) {
        return IJ_STATUS_INVALID_STATE;
    }
    if (payload_len > IJ_SYSLOG_UDP_TARGET_MAX) {
        return IJ_STATUS_SINK_ERROR;
    }

    bytes_sent = sendto(sink->fd, payload, payload_len, 0, (const struct sockaddr *)&sink->address,
                        sink->address_len);
    if (bytes_sent < 0 || (size_t)bytes_sent != payload_len) {
        return IJ_STATUS_SINK_ERROR;
    }

    return IJ_STATUS_OK;
}

void ij_syslog_sink_close(ij_syslog_sink_t *sink)
{
    if (sink == NULL) {
        return;
    }
    if (sink->fd >= 0) {
        close(sink->fd);
    }

    sink->fd = -1;
    sink->address_len = 0;
    memset(&sink->address, 0, sizeof(sink->address));
}
