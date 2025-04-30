/*
 * module_dcf.h- Sigmastar
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
#ifndef _MODULE_DCF_H_
#define _MODULE_DCF_H_

BOOL DCF_CheckDeleteFilesForSpace(int DB);
void DCF_CheckDeleteFilesForNum(int DB, int nFileNumLimit);
void DCF_CheckDeleteFilesForFormatFreeReserved(int DB, int nReservedCnt);

#endif //#define _MODULE_DCF_H_
