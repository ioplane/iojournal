/*
 * iojournal -- io_uring file sink backend
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#if IOJOURNAL_HAS_LIBURING

#    include <liburing.h>

#    include <fcntl.h>
#    include <limits.h>
#    include <stdlib.h>
#    include <unistd.h>

static int ij_file_sink_uring_drain(struct io_uring *ring, unsigned wait_for)
{
    struct io_uring_cqe *cqe = NULL;
    int rc;

    if (ring == NULL) {
        return -1;
    }

    rc = io_uring_submit_and_wait(ring, wait_for);
    if (rc < 0) {
        return rc;
    }

    rc = io_uring_wait_cqe(ring, &cqe);
    if (rc < 0) {
        return rc;
    }

    rc = cqe->res;
    io_uring_cqe_seen(ring, cqe);
    return rc;
}

ij_status_t ij_file_sink_uring_open(ij_file_sink_t *sink)
{
    struct io_uring *ring;
    int fd;
    int rc;

    if (sink == NULL || sink->path == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    ring = calloc(1U, sizeof(*ring));
    if (ring == NULL) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    rc = io_uring_queue_init(8U, ring, 0U);
    if (rc < 0) {
        free(ring);
        return IJ_STATUS_SINK_ERROR;
    }

    fd = open(sink->path, O_CREAT | O_APPEND | O_WRONLY | O_CLOEXEC, 0644);
    if (fd < 0) {
        io_uring_queue_exit(ring);
        free(ring);
        return IJ_STATUS_SINK_ERROR;
    }

    sink->fd = fd;
    sink->uring = ring;
    return IJ_STATUS_OK;
}

ij_status_t ij_file_sink_uring_write(ij_file_sink_t *sink, const char *payload, size_t payload_len)
{
    struct io_uring_sqe *sqe;
    unsigned payload_len_u32;
    int rc;

    if (sink == NULL || sink->uring == NULL || sink->fd < 0 || payload == NULL ||
        payload_len == 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (payload_len > UINT_MAX) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    sqe = io_uring_get_sqe(sink->uring);
    if (sqe == NULL) {
        return IJ_STATUS_SINK_ERROR;
    }

    payload_len_u32 = (unsigned)payload_len;
    io_uring_prep_write(sqe, sink->fd, payload, payload_len_u32, UINT64_MAX);
    rc = ij_file_sink_uring_drain(sink->uring, 1U);
    if (rc < 0 || (size_t)rc != payload_len) {
        return IJ_STATUS_SINK_ERROR;
    }

    return IJ_STATUS_OK;
}

ij_status_t ij_file_sink_uring_flush(ij_file_sink_t *sink)
{
    if (sink == NULL || sink->uring == NULL || sink->fd < 0) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    return IJ_STATUS_OK;
}

void ij_file_sink_uring_close(ij_file_sink_t *sink)
{
    if (sink == NULL) {
        return;
    }

    if (sink->fd >= 0) {
        (void)close(sink->fd);
        sink->fd = -1;
    }
    if (sink->uring != NULL) {
        io_uring_queue_exit(sink->uring);
        free(sink->uring);
        sink->uring = NULL;
    }
}

#else

ij_status_t ij_file_sink_uring_open(ij_file_sink_t *sink)
{
    (void)sink;
    return IJ_STATUS_SINK_ERROR;
}

ij_status_t ij_file_sink_uring_write(ij_file_sink_t *sink, const char *payload, size_t payload_len)
{
    (void)sink;
    (void)payload;
    (void)payload_len;
    return IJ_STATUS_SINK_ERROR;
}

ij_status_t ij_file_sink_uring_flush(ij_file_sink_t *sink)
{
    (void)sink;
    return IJ_STATUS_SINK_ERROR;
}

void ij_file_sink_uring_close(ij_file_sink_t *sink)
{
    (void)sink;
}

#endif
