/*
 * iojournal -- version helpers
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

const char *ij_version(void)
{
    return IJ_VERSION_STRING;
}

int ij_version_num(void)
{
    return (IJ_VERSION_MAJOR << 16) | (IJ_VERSION_MINOR << 8) | IJ_VERSION_PATCH;
}
