/*
 * mid_utils.h- Sigmastar
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
#ifndef _MID_UTILS_H_
#define _MID_UTILS_H_

#include "mid_common.h"

#define ALIGN_2xUP(x)  (((x) + 0x1) & (~0x1))
#define ALIGN_8xUP(x)  (((x) + 0x7) & (~0x7))
#define ALIGN_16xUP(x) (((x) + 0xF) & (~0xF))
#define ALIGN_32xUP(x) (((x) + 0x1F) & (~0x1F))
#define ALIGN64(x)     (((x) + 0x3F) & (~0x3F))
#define ALIGN128(x)    (((x) + 0x7F) & (~0x7F))
#define ALIGN256(x)    (((x) + 0xFF) & (~0xFF))
#define ALIGN4K(x)     (((x) + 0xFFF) & (~0xFFF))

#define ALIGN_16xDOWN(x)       (x & 0xFFFFFFF0)
#define ALIGN_32xDOWN(x)       (x & 0xFFFFFFE0)
#define ISADDRESSALIGNTO(x, y) (!((x) & ((y)-1)))
#define MAX_ARGC               (20)

typedef struct CARDV_FIFO_CMD_s
{
    char *pu8CmdName;
    MI_U8 u8MaxPara;
    void (*fpCmdHandle)(MI_U8, MI_U8, char **);
} CARDV_FIFO_CMD_t;

MI_U8  CarDVParseStringsStart(char *pStr, char **argv, char **ppNextcmd);
void   CarDVParseStringsEnd(MI_U8 argc, char **argv);
void   CarDVInitCmdHandler();
void   CarDVAddCmdHandler(char *u8Cmd, MI_U8 u8MaxPara, void (*fpCmdHandle)(MI_U8, MI_U8, char **));
void   CarDVRemoveCmdHandler(void);
MI_S32 CarDVExecCmdHandler(char **argv, MI_U8 argc);

#endif //_MID_UTILS_H_
