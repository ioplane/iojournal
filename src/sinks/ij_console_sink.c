/*
 * iojournal -- console sink
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <stdio.h>

ij_status_t ij_console_sink_write(const char *payload, size_t payload_len)
{
    if (payload == NULL || payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    if (fwrite(payload, 1U, payload_len, stdout) != payload_len || fputc('\n', stdout) == EOF ||
        fflush(stdout) == EOF) {
        return IJ_STATUS_SINK_ERROR;
    }

    return IJ_STATUS_OK;
}
