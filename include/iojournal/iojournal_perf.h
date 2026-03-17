/*
 * iojournal -- performance and backend selection definitions
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_IOJOURNAL_PERF_H
#define IOJOURNAL_IOJOURNAL_PERF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IJ_FILE_BACKEND_AUTO = 0,
    IJ_FILE_BACKEND_SYNC = 1,
    IJ_FILE_BACKEND_IO_URING = 2
} ij_file_backend_t;

#ifdef __cplusplus
}
#endif

#endif /* IOJOURNAL_IOJOURNAL_PERF_H */
