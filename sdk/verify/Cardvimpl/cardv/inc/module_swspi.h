/*
 * module_swspi.h- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
 *
 * Author: jiahong.xiong <jiahong.xiong@sigmastar.com.cn>
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
#ifndef _MODULE_SWSPI_H_
#define _MODULE_SWSPI_H_

void *cardv_swspi_open(unsigned char u8CsGpioNum, unsigned char u8SckGpioNum, unsigned char u8SdiGpioNum);
void  cardv_swspi_close(void *handler);
void  cardv_swspi_write(void *handler, unsigned int u32Val, unsigned int u32Bits);

#endif //#define _MODULE_SWSPI_H_
