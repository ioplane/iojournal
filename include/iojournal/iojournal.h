/*
 * iojournal -- main public API header
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_IOJOURNAL_H
#define IOJOURNAL_IOJOURNAL_H

#include <iojournal/iojournal_file.h>
#include <iojournal/iojournal_syslog.h>
#include <iojournal/iojournal_types.h>

#ifdef __cplusplus
extern "C" {
#endif

[[nodiscard]] const char *ij_version(void);
[[nodiscard]] int ij_version_num(void);

/*
 * Create one logger instance from caller-owned configuration data.
 * Any state required after return is copied into logger-owned storage.
 */
[[nodiscard]] ij_status_t ij_logger_init(ij_logger_t **out_logger,
                                         const ij_logger_config_t *config);

/*
 * Release logger-owned resources.
 * Passing NULL is allowed and returns IJ_STATUS_OK.
 */
[[nodiscard]] ij_status_t ij_logger_shutdown(ij_logger_t *logger);

/*
 * Flush buffered RC sinks.
 * The current Sprint 05 build surface keeps this as a bounded no-op for
 * initialized loggers until sink workers are introduced.
 */
[[nodiscard]] ij_status_t ij_logger_flush(ij_logger_t *logger);

/*
 * Validate and submit one immutable event.
 * The full bounded queue and encoder path is implemented in later Sprint 05
 * tasks; the Task 1 build surface only validates logger state and arguments.
 */
[[nodiscard]] ij_status_t ij_logger_log(ij_logger_t *logger, const ij_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* IOJOURNAL_IOJOURNAL_H */
