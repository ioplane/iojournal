/*
 * iojournal -- public syslog constants for the RC surface
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IOJOURNAL_IOJOURNAL_SYSLOG_H
#define IOJOURNAL_IOJOURNAL_SYSLOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IJ_SYSLOG_RFC5424_VERSION  1U
#define IJ_SYSLOG_UDP_PORT_DEFAULT 514U
#define IJ_SYSLOG_TCP_PORT_DEFAULT 514U
#define IJ_SYSLOG_UDP_TARGET_MAX   2048U

#ifdef __cplusplus
}
#endif

#endif /* IOJOURNAL_IOJOURNAL_SYSLOG_H */
