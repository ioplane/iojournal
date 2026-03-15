/*
 * iojournal -- NDJSON encoder
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <string.h>

ij_status_t ij_ndjson_encode(const ij_event_copy_t *event, char *buffer, size_t buffer_size,
                             size_t *out_len)
{
    size_t json_len = 0U;
    ij_status_t status;

    if (buffer == NULL || out_len == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    status = ij_json_console_encode(event, buffer, buffer_size, &json_len);
    if (status != IJ_STATUS_OK) {
        return status;
    }
    if (json_len + 2U > buffer_size) {
        return IJ_STATUS_ENCODE_ERROR;
    }

    buffer[json_len] = '\n';
    buffer[json_len + 1U] = '\0';
    *out_len = json_len + 1U;
    return IJ_STATUS_OK;
}
