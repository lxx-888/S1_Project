/*
 * drv_pnl_os_header.h- Sigmastar
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

#ifndef _DRV_PNL_OS_HEADER_H_
#define _DRV_PNL_OS_HEADER_H_

#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <stdlib.h>

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PNL_IO_OFFSET (0UL)

#define PNL_IO_ADDRESS(x) ((x) + PNL_IO_OFFSET)
//#define __io_address(n)       ((void __iomem __force *)PNL_IO_ADDRESS(n))

/* read register by byte */
#define pnl_readb(a) (*(volatile unsigned char *)PNL_IO_ADDRESS(a))

/* read register by word */
#define pnl_readw(a) (*(volatile unsigned short *)PNL_IO_ADDRESS(a))

/* read register by long */
#define pnl_readl(a) (*(volatile unsigned int *)PNL_IO_ADDRESS(a))

/* write register by byte */
#define pnl_writeb(v, a) (*(volatile unsigned char *)PNL_IO_ADDRESS(a) = (v))

/* write register by word */
#define pnl_writew(v, a) (*(volatile unsigned short *)PNL_IO_ADDRESS(a) = (v))

/* write register by long */
#define pnl_writel(v, a) (*(volatile unsigned int *)PNL_IO_ADDRESS(a) = (v))

#define READ_BYTE(x)     pnl_readb(x)
#define READ_WORD(x)     pnl_readw(x)
#define READ_LONG(x)     pnl_readl(x)
#define WRITE_BYTE(x, y) pnl_writeb((u8)(y), x)
#define WRITE_WORD(x, y) pnl_writew((u16)(y), x)
#define WRITE_LONG(x, y) pnl_writel((u32)(y), x)

#define CamOsPrintf printf
typedef struct
{
    void *pFile;
} DrvPnlOsFileConfig_t;

#endif
