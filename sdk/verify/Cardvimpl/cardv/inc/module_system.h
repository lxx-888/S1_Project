/*
 * module_system.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _MODULE_SYSTEM_H_
#define _MODULE_SYSTEM_H_

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void cardv_send_to_fifo(const void *cmd, size_t size);
void cardv_update_status(const char *status_name, const char *status, size_t size);
int  cardv_get_status(const char *status_name);
void cardv_record_boot_time(int line);

#endif //#define _MODULE_SYSTEM_H_
