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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#ifdef LINUX_OS
#include "mi_common_datatype.h"
#include "st_common_ai_glasses_protrcol.h"
#include "cam_os_wrapper.h"
#else
#include "os_utils.h"
#include "inc/st_common_ai_glasses_protrcol.h"
#endif

char protHeader[] = {0x5A, 0x5A, 0x5A, 0x5A};
char protTail[]   = {0xA5, 0xA5, 0xA5, 0xA5};

/* the length of msg_buf must be SS_TASK_CMD_PROTRCOL */
MI_S32 ST_Common_AiGlasses_Prot_Make_Cmd_To_Send(ST_Protrcol_Task_t *pstProtTask, MI_S32 (*uartSendFun)(char *, MI_U32))
{
    char *protBuf      = NULL;
    int   u8ProtBufLen = SS_TASK_CMD_PROTRCOL + 1;
    int   ret          = 0;

    if (!uartSendFun)
    {
        DEBUG_ERROR("uartSendFun is NULL\r\n");
        return -1;
    }
    if (!pstProtTask)
    {
        DEBUG_ERROR("pstProtTask is NULL\r\n");
        return -1;
    }
    if (pstProtTask->eCmdType >= E_CMD_TYPE_ERR)
    {
        DEBUG_ERROR("eCmdType:%d err\r\n", pstProtTask->eCmdType);
        return -1;
    }
    if (pstProtTask->eTaskCmd >= E_TASK_ERR)
    {
        DEBUG_ERROR("eTaskCmd:%d err\r\n", pstProtTask->eTaskCmd);
        return -1;
    }
    if (pstProtTask->eSocState >= E_STATE_UNKOWN)
    {
        DEBUG_ERROR("eSocState:%d err\r\n", pstProtTask->eSocState);
        return -1;
    }

    if (pstProtTask->eTaskCmd == E_INDEF_LEN_MSG)
    {
        if (pstProtTask->pstProtMsg)
        {
            u8ProtBufLen += pstProtTask->pstProtMsg->length;
        }
        else
        {
            DEBUG_ERROR("pstProtMsg is NULL, but eTaskCmd:%d\r\n", pstProtTask->eTaskCmd);
        }
    }
#ifdef SS_USE_TWS_LIGHT_SENSOR
    else if (pstProtTask->eTaskCmd == E_TASK_PHOTO || pstProtTask->eTaskCmd == E_TASK_START_REC)
    {
        u8ProtBufLen += sizeof(MI_U16);
    }
#endif
    protBuf = (char *)CamOsMemAlloc(u8ProtBufLen * sizeof(char));
    if (!protBuf)
    {
        DEBUG_ERROR("alloc protBuf failed\r\n");
        return -1;
    }
    memset(protBuf, 0x0, u8ProtBufLen);

    memcpy(protBuf, protHeader, sizeof(protHeader));
    protBuf[4] = pstProtTask->eCmdType;
    protBuf[5] = pstProtTask->eTaskCmd;
    protBuf[6] = pstProtTask->isReject;
    protBuf[7] = pstProtTask->eSocState;
    memcpy(protBuf + (8 * sizeof(char)), &pstProtTask->u16UserDefine, sizeof(pstProtTask->u16UserDefine));
    if (pstProtTask->eTaskCmd == E_INDEF_LEN_MSG && pstProtTask->pstProtMsg)
    {
        protBuf[9] = pstProtTask->pstProtMsg->length;
        memcpy(protBuf + (8 * sizeof(char)) + sizeof(pstProtTask->u16UserDefine), pstProtTask->pstProtMsg->msg,
               pstProtTask->pstProtMsg->length);
    }
#ifdef SS_USE_TWS_LIGHT_SENSOR
    else if (pstProtTask->eTaskCmd == E_TASK_PHOTO || pstProtTask->eTaskCmd == E_TASK_START_REC)
    {
        memcpy(protBuf + (8 * sizeof(char)) + sizeof(pstProtTask->u16UserDefine), &pstProtTask->u16LuxValue,
               sizeof(pstProtTask->u16LuxValue));
    }
#endif
    memcpy(protBuf + ((u8ProtBufLen - 5) * sizeof(char)), protTail, sizeof(protTail));

    protBuf[u8ProtBufLen - 1] = 0xD; // enter CR !!

    ret = uartSendFun(protBuf, u8ProtBufLen);
    CamOsMemRelease(protBuf);
    return ret;
}

static MI_S32 _ST_Common_AiGlasses_Prot_Get_Task_From_Msg(char *msg_buf, ST_Protrcol_Task_t *pstProtTask)
{
    if (!msg_buf)
    {
        DEBUG_ERROR("msg_buf is NULL\r\n");
        return -1;
    }
    if (!pstProtTask)
    {
        DEBUG_ERROR("pstProtTask is NULL\r\n");
        return -1;
    }
    pstProtTask->eCmdType  = msg_buf[4];
    pstProtTask->eTaskCmd  = msg_buf[5];
    pstProtTask->isReject  = msg_buf[6];
    pstProtTask->eSocState = msg_buf[7];
    memcpy(&pstProtTask->u16UserDefine, &msg_buf[8], sizeof(pstProtTask->u16UserDefine));
#ifdef SS_USE_TWS_LIGHT_SENSOR
    if (pstProtTask->eTaskCmd == E_TASK_PHOTO || pstProtTask->eTaskCmd == E_TASK_START_REC)
    {
        memcpy(&pstProtTask->u16LuxValue, &msg_buf[8 + sizeof(pstProtTask->u16UserDefine)],
               sizeof(pstProtTask->u16LuxValue));
    }
#endif
    return MI_SUCCESS;
}

static MI_S32 _ST_Common_AiGlasses_Prot_Find_Msg(const char *buf, MI_U32 buf_size, ST_Protrcol_Msg_t *pstProtMsg,
                                                 BOOL useDma, char **reminBuf)
{
    char * start       = NULL;
    char   startBuf[5] = {0};
    int    value       = 0;
    MI_U32 msgLength   = 0;

    memcpy(startBuf, protHeader, sizeof(protHeader));
    startBuf[4] = '\0';

    start = strstr(buf, startBuf);
    if (start == NULL)
    {
        if (useDma)
        {
            DEBUG_ERROR("cat't find protHeader\r\n");
        }
        return -1;
    }

    if (start[5] == E_INDEF_LEN_MSG)
    {
        // get msg length from prot[user define]bit1
        msgLength = start[9];
        // Indefinite length
        if (msgLength == 0 || msgLength > buf_size - SS_TASK_CMD_PROTRCOL)
        {
            DEBUG_ERROR("msgLength:%d error, buf_size:%d\r\n", msgLength, buf_size);
            return -1;
        }
    }
#ifdef SS_USE_TWS_LIGHT_SENSOR
    else if (start[5] == E_TASK_PHOTO || start[5] == E_TASK_START_REC)
    {
        msgLength = sizeof(MI_U16);
    }
#endif
    value = memcmp(start + SS_TASK_CMD_PROTRCOL + msgLength - sizeof(protTail), protTail, sizeof(protTail));
    if (value != 0)
    {
        DEBUG_ERROR("cat't find protTail, buf:%lx start:%lx cmp:%d\r\n", (unsigned long)buf, (unsigned long)start,
                    value);
        DEBUG_ERROR("msgLength:%d\r\n", msgLength);
        // if you want continue to find, you can buf = start + 4 and goto strstr again
        return -1;
    }

    if (pstProtMsg)
    {
        int   reminLen     = 0;
        char *pstProtStart = NULL;
        if (start[5] == E_INDEF_LEN_MSG)
        {
            pstProtMsg->isIndeLengMsg = TRUE;
            pstProtMsg->length        = msgLength;
            pstProtStart              = start + SS_TASK_CMD_PROTRCOL - sizeof(protTail);
        }
        else
        {
            pstProtMsg->isIndeLengMsg = FALSE;
            pstProtMsg->length        = SS_TASK_CMD_PROTRCOL;
            pstProtStart              = start;
#ifdef SS_USE_TWS_LIGHT_SENSOR
            if (start[5] == E_TASK_PHOTO || start[5] == E_TASK_START_REC)
            {
                pstProtMsg->length += msgLength;
            }
#endif
        }
        pstProtMsg->msg = (char *)CamOsMemAlloc(pstProtMsg->length);
        if (!pstProtMsg->msg)
        {
            DEBUG_ERROR("alloc pstProtMsg fail\r\n");
            return -1;
        }
        memcpy(pstProtMsg->msg, pstProtStart, pstProtMsg->length);

        reminLen = (buf + buf_size) - (start + SS_TASK_CMD_PROTRCOL + msgLength);
        if (reminLen > 0)
        {
            *reminBuf = start + SS_TASK_CMD_PROTRCOL + msgLength;
        }
        else
        {
            *reminBuf = NULL;
        }
    }
    else
    {
        return -2;
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiGlasses_Prot_Deal_Uart_Buffer(char *uartBuf, MI_U32 len, BOOL useDma,
                                                 MI_S32 (*protcolTaskHandler)(ST_Protrcol_Task_t *pstProtcolTask))
{
    ST_Protrcol_Msg_t stProtrcolMsg = {0};
    char *            pstUartBuf    = uartBuf;
    char *            pstUartBufEnd = uartBuf + len;
    int               bufLen        = len;
    int               ret           = MI_SUCCESS;

    while (pstUartBuf && bufLen > 0)
    {
        MI_U16 cnt      = 0;
        char * reminBuf = NULL;

        ret = _ST_Common_AiGlasses_Prot_Find_Msg(pstUartBuf, bufLen, &stProtrcolMsg, useDma, &reminBuf);
        if (ret == MI_SUCCESS)
        {
            ST_Protrcol_Task_t stProtcolTask = {0};
            long               milliseconds  = 0;
#ifdef LINUX_OS
            struct timeval tv = {0};
            gettimeofday(&tv, NULL);
            milliseconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
            milliseconds = os_boot_time32();
#endif
            if (stProtrcolMsg.isIndeLengMsg == FALSE)
            {
                char protMsg[SS_TASK_CMD_PROTRCOL + sizeof(MI_U16)] = {0};

                UNUSED(cnt);
                UNUSED(milliseconds);
                memcpy(&protMsg, stProtrcolMsg.msg, stProtrcolMsg.length);
                DEBUG_INFO("[%ld]get mcu protMsg:\r\n", milliseconds);
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_INFO
                for (cnt = 0; cnt < (sizeof(protMsg) / sizeof(char)); cnt++)
                {
                    printf("%x ", protMsg[cnt]);
                }
                printf("\r\n");
#endif
                _ST_Common_AiGlasses_Prot_Get_Task_From_Msg((char *)&protMsg, &stProtcolTask);
                // handle msg to deal task stProtcolTask
                /* ret = ST_GLASSES_FACTORY_HandleTask(&stProtcolTask); */
            }
            else
            {
                DEBUG_INFO("[%ld]get mcu Indefinite legth Msg:%d\r\n", milliseconds, stProtrcolMsg.length);
                stProtcolTask.pstProtMsg = &stProtrcolMsg;
            }

            if (protcolTaskHandler)
            {
                ret = protcolTaskHandler(&stProtcolTask);
            }
            CamOsMemRelease(stProtrcolMsg.msg);
        }
        else if (ret == -1 && useDma)
        {
            DEBUG_ERROR("Received:%s\r\n", uartBuf);
            /*             while (cnt < len) */
            /* { */
            /*     DEBUG_INFO("received buf[%d]:%x\n", cnt, uartBuf[cnt]); */
            /*     cnt++; */
            /* } */
        }

        if (reminBuf)
        {
            bufLen = pstUartBufEnd - reminBuf;
            DEBUG_INFO("remin bufLen:%d reminBuf:%lx origin pstUartBuf:%lx pstUartBufEnd:%lx\r\n", bufLen,
                       (unsigned long)reminBuf, (unsigned long)pstUartBuf, (unsigned long)pstUartBufEnd);
            if (bufLen >= SS_TASK_CMD_PROTRCOL)
            {
                pstUartBuf = reminBuf;
                memset(&stProtrcolMsg, 0x0, sizeof(stProtrcolMsg));
            }
            else
            {
                pstUartBuf = NULL;
                return bufLen;
            }
        }
        else
        {
            bufLen     = 0;
            pstUartBuf = NULL;
        }
    }
    return ret;
}
