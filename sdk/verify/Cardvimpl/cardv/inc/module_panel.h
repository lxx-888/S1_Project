/*
 * module_panel.h- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
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
#ifndef _MODULE_PANEL_H_
#define _MODULE_PANEL_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_panel.h"
#include "module_spi.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define SPI_MODE  (0)       // CPOL=0 CPHA=0
#define SPI_BITS  (9)       // 9 bits, MSB first
#define SPI_SPEED (1000000) // 1MHz

#define REGFLAG_DELAY        (0xFFFE)
#define REGFLAG_END_OF_TABLE (0xFFFF) // END OF REGISTERS MARKER

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    MI_U16 *pu16CmdBuf;
    MI_U16  u16CmdBufSize;
} stPnlInitCmd_t;

typedef struct
{
    unsigned int  cmd;
    unsigned char count;
    unsigned char para_list[64];
} stPnlSpiCmdTable_t;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_PanelModuleInit(MI_DISP_Interface_e eDispIf);
MI_S32 CarDV_PanelModuleUnInit(MI_DISP_Interface_e eDispIf);

#endif //#define _MODULE_PANEL_H_