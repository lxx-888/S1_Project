/*
 * module_ie.cpp- Sigmastar
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

#include "module_common.h"
#include "module_config.h"

#if CARDV_VDF_ENABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "mid_common.h"
#include "module_vdf.h"
#include "module_md.h"

CarDV_VdfModAttr_t gstVdfModule = {0};

int vdf_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    CarDV_VdfModAttr_t *    pstVdfModule = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;
    MI_U32                  u32VdfChn;

    switch (id)
    {
    case CMD_VDF_OPEN: {
        if (pstVdfModule->bUsed == TRUE && pstVdfModule->bCreate == FALSE)
        {
            MI_VDF_Init();
            pstVdfModule->bCreate = TRUE;
        }
    }
    break;

    case CMD_VDF_CLOSE:
        if (pstVdfModule->bCreate == TRUE)
        {
            for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
            {
                pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
                if (pstVdfChnAttr->bUsed && pstVdfChnAttr->bCreate == TRUE)
                {
                    return 0;
                }
            }
            MI_VDF_Uninit();
            pstVdfModule->bCreate = FALSE;
        }
        break;

    case CMD_VDF_MD_OPEN:
        if (pstVdfModule->bCreate == TRUE)
        {
            MD_Initial(1);
        }
        break;

    case CMD_VDF_MD_CLOSE:
        if (pstVdfModule->bCreate == TRUE)
        {
            MD_Uninitial();
        }
        break;

        // case CMD_VDF_MD_CHANGE:
        //     if (pstVdfChnAttr->bCreate == TRUE)
        //     {
        //         MD_Param_Change(param, paramLen);
        //     }
        //     break;

    default:
        break;
    }

    return 0;
}
#endif