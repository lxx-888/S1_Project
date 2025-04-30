/*
 * module_lightdetect.h- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
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

#ifndef __MODULE_LD_H__
#define __MODULE_LD_H__

#include "module_config.h"
#include "mi_sys.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define LD_DBG_MSG(enable, format, args...) \
    do                                      \
    {                                       \
        if (enable)                         \
            printf(format, ##args);         \
    } while (0)
#define LD_DBG_ERR(enable, format, args...) \
    do                                      \
    {                                       \
        if (1)                              \
            printf(format, ##args);         \
    } while (0)

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern MI_SYS_ChnPort_t gstLDSrcModule;

#endif /* __MODULE_LD_H__ */