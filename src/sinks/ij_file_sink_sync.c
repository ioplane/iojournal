/*
 * iojournal -- synchronous file sink backend
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

ij_status_t ij_file_sink_sync_open(ij_file_sink_t *sink)
{
    if (sink == NULL || sink->path == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    sink->stream = fopen(sink->path, "ab");
    if (sink->stream == NULL) {
        return IJ_STATUS_SINK_ERROR;
    }

    return IJ_STATUS_OK;
}

ij_status_t ij_file_sink_sync_write(ij_file_sink_t *sink, const char *payload, size_t payload_len)
{
    if (sink == NULL || sink->stream == NULL || payload == NULL || payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    if (fwrite(payload, 1U, payload_len, sink->stream) != payload_len) {
        return IJ_STATUS_SINK_ERROR;
    }

    return IJ_STATUS_OK;
}

ij_status_t ij_file_sink_sync_flush(ij_file_sink_t *sink)
{
    if (sink == NULL || sink->stream == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    return fflush(sink->stream) == 0 ? IJ_STATUS_OK : IJ_STATUS_SINK_ERROR;
}

void ij_file_sink_sync_close(ij_file_sink_t *sink)
{
    if (sink == NULL || sink->stream == NULL) {
        return;
    }

    (void)fclose(sink->stream);
    sink->stream = NULL;
}
