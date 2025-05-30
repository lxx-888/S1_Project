/*
 * rgn_sysfs.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef _RGN_SYSFS_H_
#define _RGN_SYSFS_H_

#ifdef _RGN_SYSFS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void DrvRgnSysfsInit(struct device *device);

#undef INTERFACE

#endif
