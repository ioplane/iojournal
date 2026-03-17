/*
 * iojournal -- console sink
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <sys/uio.h>
#include <unistd.h>

ij_status_t ij_console_sink_write(const char *payload, size_t payload_len)
{
    static const char newline = '\n';
    struct iovec vectors[2];
    size_t remaining = payload_len + 1U;
    size_t offset = 0U;

    if (payload == NULL || payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    while (remaining > 0U) {
        ssize_t written;

        vectors[0].iov_base = (void *)(payload + offset);
        vectors[0].iov_len = payload_len - offset;
        vectors[1].iov_base = (void *)&newline;
        vectors[1].iov_len = 1U;

        if (offset >= payload_len) {
            vectors[0].iov_len = 0U;
        }

        written = writev(STDOUT_FILENO, vectors, 2);
        if (written <= 0) {
            return IJ_STATUS_SINK_ERROR;
        }

        if ((size_t)written >= remaining) {
            break;
        }

        remaining -= (size_t)written;
        if (offset < payload_len) {
            size_t payload_written = (size_t)written;

            if (payload_written > payload_len - offset) {
                payload_written = payload_len - offset;
            }
            offset += payload_written;
        }
    }

    return IJ_STATUS_OK;
}
