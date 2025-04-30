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
#ifndef __SSAPP_GLASSES_INDEMSGTASK_H
#define __SSAPP_GLASSES_INDEMSGTASK_H

#include "mi_common_datatype.h"
#include "st_common_ai_glasses_protrcol.h"

MI_S32 SSAPP_GLASSES_INDEMSGTASK_HandleMsg(ST_Protrcol_Msg_t *pstProtrcolMsg);
MI_S32 SSAPP_GLASSES_INDEMSGTASK_SendMsg(SS_INDEMSG_t *pstMsg);
#endif
