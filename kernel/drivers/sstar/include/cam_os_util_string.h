/*
 * cam_os_util_string.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

#ifndef __CAM_OS_UTIL_STRING_H__
#define __CAM_OS_UTIL_STRING_H__

#ifndef KERN_SOH
#define KERN_SOH "\001" /* ASCII Start Of Header */
#endif

#ifndef KERN_EMERG
#define KERN_EMERG KERN_SOH "0" /* system is unusable */
#endif

#ifndef KERN_ALERT
#define KERN_ALERT KERN_SOH "1" /* action must be taken immediately */
#endif

#ifndef KERN_CRIT
#define KERN_CRIT KERN_SOH "2" /* critical conditions */
#endif

#ifndef KERN_ERR
#define KERN_ERR KERN_SOH "3" /* error conditions */
#endif

#ifndef KERN_WARNING
#define KERN_WARNING KERN_SOH "4" /* warning conditions */
#endif

#ifndef KERN_NOTICE
#define KERN_NOTICE KERN_SOH "5" /* normal but significant condition */
#endif

#ifndef KERN_INFO
#define KERN_INFO KERN_SOH "6" /* informational */
#endif

#ifndef KERN_DEBUG
#define KERN_DEBUG KERN_SOH "7" /* debug-level messages */
#endif

#endif //__CAM_OS_UTIL_STRING_H__
