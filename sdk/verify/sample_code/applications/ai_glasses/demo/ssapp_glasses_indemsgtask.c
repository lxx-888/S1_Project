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

#include <stdio.h>
#include <string.h>

#include "cam_os_wrapper.h"
#include "ssapp_glasses_factory.h"
#include "ssapp_glasses_indemsgtask.h"

/* Users need to parse and process indefinite length messages on own. Note that the pstProtcolMsg will be released
 * after this function returns */
MI_S32 SSAPP_GLASSES_INDEMSGTASK_HandleMsg(ST_Protrcol_Msg_t *pstProtrcolMsg)
{
    SS_INDEMSG_t *pstIndeMsg = NULL;
    if (!pstProtrcolMsg || !pstProtrcolMsg->msg)
    {
        DEBUG_ERROR("pstProtrcolMsg:%lx or msg:%lx is NULL\r\n", (unsigned long)pstProtrcolMsg,
                    (unsigned long)(!pstProtrcolMsg ? 0x0 : pstProtrcolMsg->msg));
        return -1;
    }

    if (pstProtrcolMsg->length == sizeof(SS_INDEMSG_t))
    {
        pstIndeMsg = (SS_INDEMSG_t *)pstProtrcolMsg->msg;
        if (pstIndeMsg->eIndeMsgType == E_INDEF_TYPE_SIGMASTAR_HELLO)
        {
            DEBUG_INFO("the voice from tws:\r\n");
            printf("\e[35m %s \e[0m\r\n", pstIndeMsg->stIndefLenMsg.sigmaStar);
        }
        else if (pstIndeMsg->eIndeMsgType == E_INDEF_TYPE_TIMESTAMP_SYNC)
        {
            DEBUG_INFO("timeStamp sync\r\n");
        }
        else
        {
            DEBUG_ERROR("unknown indef type:%d\r\n", pstIndeMsg->eIndeMsgType);
            return -1;
        }
    }
    else
    {
        DEBUG_ERROR("unknown indef msg length:%d\r\n", pstProtrcolMsg->length);
        DEBUG_ERROR("You may need to parse this data yourself\r\n");
    }

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_INDEMSGTASK_SendMsg(SS_INDEMSG_t *pstMsg)
{
    int                ret        = MI_SUCCESS;
    ST_Protrcol_Task_t stProtTask = {0};

    if (!pstMsg)
    {
        DEBUG_ERROR("type:%d pstMsg:%lx\r\n", pstMsg->eIndeMsgType, (unsigned long)pstMsg);
        return -1;
    }
    stProtTask.eTaskCmd   = E_INDEF_LEN_MSG;
    stProtTask.pstProtMsg = CamOsMemAlloc(sizeof(ST_Protrcol_Msg_t));
    if (!stProtTask.pstProtMsg)
    {
        DEBUG_ERROR("inde alloc pstProtMsg failed\r\n");
        return -1;
    }
    memset(stProtTask.pstProtMsg, 0x0, sizeof(ST_Protrcol_Msg_t));
    stProtTask.pstProtMsg->isIndeLengMsg = TRUE;
    stProtTask.pstProtMsg->length        = sizeof(SS_INDEMSG_t);
    stProtTask.pstProtMsg->msg           = (char *)pstMsg;
    ret = ST_Common_AiGlasses_Prot_Make_Cmd_To_Send(&stProtTask, (void *)SSAPP_GLASSES_FACTORY_ProtSend);
    CamOsMemRelease(stProtTask.pstProtMsg);
    return ret;
}
