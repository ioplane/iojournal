/*
 * iojournal -- file sink backend dispatch
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    ij_status_t (*open)(ij_file_sink_t *sink);
    ij_status_t (*write)(ij_file_sink_t *sink, const char *payload, size_t payload_len);
    ij_status_t (*flush)(ij_file_sink_t *sink);
    void (*close)(ij_file_sink_t *sink);
} ij_file_sink_ops_t;

static const ij_file_sink_ops_t IJ_FILE_SINK_SYNC_OPS = {
    .open = ij_file_sink_sync_open,
    .write = ij_file_sink_sync_write,
    .flush = ij_file_sink_sync_flush,
    .close = ij_file_sink_sync_close,
};

static const ij_file_sink_ops_t IJ_FILE_SINK_IO_URING_OPS = {
    .open = ij_file_sink_uring_open,
    .write = ij_file_sink_uring_write,
    .flush = ij_file_sink_uring_flush,
    .close = ij_file_sink_uring_close,
};

static void ij_file_sink_reset_rotation_cache(ij_file_sink_t *sink)
{
    if (sink->rotated_paths != NULL) {
        for (size_t i = 0U; i < sink->rotated_path_count; ++i) {
            free(sink->rotated_paths[i]);
        }
    }
    free(sink->rotated_paths);
    sink->rotated_paths = NULL;
    sink->rotated_path_count = 0U;
}

static ij_status_t ij_file_sink_store_path(ij_file_sink_t *sink, const char *path)
{
    size_t len = strlen(path);

    sink->path = malloc(len + 1U);
    if (sink->path == NULL) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    memcpy(sink->path, path, len + 1U);
    return IJ_STATUS_OK;
}

static char *ij_file_sink_strdup(const char *value)
{
    size_t len = strlen(value);
    char *copy = malloc(len + 1U);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, value, len + 1U);
    return copy;
}

static ij_status_t ij_file_sink_record_rotation(ij_file_sink_t *sink, const char *rotated_path)
{
    char **new_paths;
    char *path_copy;

    path_copy = ij_file_sink_strdup(rotated_path);
    if (path_copy == NULL) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    new_paths = realloc(sink->rotated_paths, (sink->rotated_path_count + 1U) * sizeof(*new_paths));
    if (new_paths == NULL) {
        free(path_copy);
        return IJ_STATUS_INTERNAL_ERROR;
    }

    sink->rotated_paths = new_paths;
    sink->rotated_paths[sink->rotated_path_count++] = path_copy;
    return IJ_STATUS_OK;
}

static void ij_file_sink_apply_retention(ij_file_sink_t *sink)
{
    while (sink->retention_files > 0U && sink->rotated_path_count > sink->retention_files) {
        char *obsolete_path = sink->rotated_paths[0];

        (void)unlink(obsolete_path);
        free(obsolete_path);
        memmove(&sink->rotated_paths[0], &sink->rotated_paths[1],
                (sink->rotated_path_count - 1U) * sizeof(*sink->rotated_paths));
        sink->rotated_path_count -= 1U;
    }
}

static ij_status_t ij_file_sink_make_rotation_path(const ij_file_sink_t *sink, char *buffer,
                                                   size_t buffer_size)
{
    struct tm tm_utc;
    time_t now = time(NULL);
    size_t path_len;
    int written;

    if (buffer == NULL || buffer_size == 0U || sink == NULL || sink->path == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (gmtime_r(&now, &tm_utc) == NULL) {
        return IJ_STATUS_SINK_ERROR;
    }

    path_len = strlen(sink->path);
    {
        unsigned long long next_sequence = (unsigned long long)sink->rotation_sequence + 1ULL;

        written = snprintf(buffer, buffer_size, "%s.%04d%02d%02dT%02d%02d%02dZ.%06llu", sink->path,
                           tm_utc.tm_year + 1900, tm_utc.tm_mon + 1, tm_utc.tm_mday, tm_utc.tm_hour,
                           tm_utc.tm_min, tm_utc.tm_sec, next_sequence);
    }
    if (written <= 0 || (size_t)written >= buffer_size || (size_t)written <= path_len) {
        return IJ_STATUS_SINK_ERROR;
    }

    return IJ_STATUS_OK;
}

static bool ij_file_sink_rotation_due(const ij_file_sink_t *sink, size_t next_payload_len)
{
    time_t now = time(NULL);

    if ((sink->active_backend == IJ_FILE_BACKEND_SYNC && sink->stream == NULL) ||
        (sink->active_backend == IJ_FILE_BACKEND_IO_URING && sink->fd < 0)) {
        return false;
    }
    if (sink->rotate_bytes > 0U && sink->bytes_written > 0U &&
        sink->bytes_written + next_payload_len > sink->rotate_bytes) {
        return true;
    }
    if (sink->rotate_interval_seconds > 0U && sink->bytes_written > 0U &&
        (uint64_t)(now - sink->last_rotation_epoch) >= sink->rotate_interval_seconds) {
        return true;
    }

    return false;
}

static const ij_file_sink_ops_t *ij_file_sink_ops(const ij_file_sink_t *sink)
{
    if (sink->active_backend == IJ_FILE_BACKEND_IO_URING) {
        return &IJ_FILE_SINK_IO_URING_OPS;
    }

    return &IJ_FILE_SINK_SYNC_OPS;
}

static ij_status_t ij_file_sink_rotate(ij_file_sink_t *sink)
{
    char rotated_path[4096];
    const ij_file_sink_ops_t *ops;
    ij_status_t status;

    if (sink == NULL || sink->path == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    ops = ij_file_sink_ops(sink);
    if (ij_file_sink_make_rotation_path(sink, rotated_path, sizeof(rotated_path)) != IJ_STATUS_OK) {
        return IJ_STATUS_SINK_ERROR;
    }

    status = ops->flush(sink);
    if (status != IJ_STATUS_OK) {
        return status;
    }
    ops->close(sink);
    if (rename(sink->path, rotated_path) != 0) {
        return IJ_STATUS_SINK_ERROR;
    }
    if (ij_file_sink_record_rotation(sink, rotated_path) != IJ_STATUS_OK) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    sink->rotation_sequence += 1U;
    sink->bytes_written = 0U;
    sink->last_rotation_epoch = time(NULL);
    ij_file_sink_apply_retention(sink);

    return ops->open(sink);
}

ij_status_t ij_file_sink_open(ij_file_sink_t *sink, const char *path)
{
    ij_status_t status;
    const ij_file_sink_ops_t *ops;
    ij_file_backend_t requested_backend;

    if (sink == NULL || path == NULL || path[0] == '\0') {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    requested_backend = sink->requested_backend;
    memset(sink, 0, sizeof(*sink));
    sink->fd = -1;
    sink->requested_backend = requested_backend;

    status = ij_file_sink_store_path(sink, path);
    if (status != IJ_STATUS_OK) {
        return status;
    }

    if (sink->requested_backend == IJ_FILE_BACKEND_IO_URING) {
        sink->active_backend = IJ_FILE_BACKEND_IO_URING;
        ops = ij_file_sink_ops(sink);
        status = ops->open(sink);
        if (status != IJ_STATUS_OK) {
            sink->active_backend = IJ_FILE_BACKEND_SYNC;
            sink->fd = -1;
            sink->uring = NULL;
            ops = ij_file_sink_ops(sink);
            status = ops->open(sink);
            if (status != IJ_STATUS_OK) {
                ij_file_sink_close(sink);
                return status;
            }
        }
    } else {
        sink->active_backend = IJ_FILE_BACKEND_SYNC;
        ops = ij_file_sink_ops(sink);
        status = ops->open(sink);
        if (status != IJ_STATUS_OK) {
            ij_file_sink_close(sink);
            return status;
        }
    }

    sink->bytes_written = 0U;
    sink->rotation_sequence = 0U;
    sink->last_rotation_epoch = time(NULL);

    return IJ_STATUS_OK;
}

ij_status_t ij_file_sink_write(ij_file_sink_t *sink, const char *payload, size_t payload_len)
{
    ij_status_t status;
    const ij_file_sink_ops_t *ops;

    if (sink == NULL || payload == NULL || payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    ops = ij_file_sink_ops(sink);
    if (ij_file_sink_rotation_due(sink, payload_len)) {
        status = ij_file_sink_rotate(sink);
        if (status != IJ_STATUS_OK) {
            return status;
        }
    }

    status = ops->write(sink, payload, payload_len);
    if (status == IJ_STATUS_OK) {
        sink->bytes_written += payload_len;
    }

    return status;
}

ij_status_t ij_file_sink_flush(ij_file_sink_t *sink)
{
    if (sink == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    return ij_file_sink_ops(sink)->flush(sink);
}

void ij_file_sink_close(ij_file_sink_t *sink)
{
    if (sink == NULL) {
        return;
    }

    ij_file_sink_ops(sink)->close(sink);
    ij_file_sink_reset_rotation_cache(sink);
    free(sink->path);
    sink->path = NULL;
}
