/*
 * iojournal -- level filter helpers
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

bool ij_level_is_enabled(ij_level_t min_level, ij_level_t event_level)
{
    if (!ij_level_is_valid(min_level) || !ij_level_is_valid(event_level)) {
        return false;
    }

    return event_level >= min_level;
}

const char *ij_level_to_text(ij_level_t level)
{
    switch (level) {
    case IJ_LEVEL_TRACE:
        return "trace";
    case IJ_LEVEL_DEBUG:
        return "debug";
    case IJ_LEVEL_INFO:
        return "info";
    case IJ_LEVEL_WARN:
        return "warn";
    case IJ_LEVEL_ERROR:
        return "error";
    case IJ_LEVEL_FATAL:
        return "fatal";
    default:
        return "unknown";
    }
}
