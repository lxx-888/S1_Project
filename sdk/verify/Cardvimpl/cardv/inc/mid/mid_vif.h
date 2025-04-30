/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include "mid_common.h"
#include "mi_sys.h"

typedef void (*GetRawCB)(MI_U8, MI_U8 *, MI_U32);

MI_S32 CarDV_GetRawEnable(MI_U8 u8CamId, MI_SYS_ChnPort_t *pstChnPort);
MI_S32 CarDV_GetRawDisable(MI_U8 u8CamId);
MI_S32 CarDV_GetRaw(MI_U8 u8CamId, MI_U32 u32ShutterUs);
void   CarDV_SetRawCallback(GetRawCB pFunc);