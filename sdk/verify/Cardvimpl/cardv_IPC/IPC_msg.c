/*
 * IPC_msg.c - Sigmastar
 *
 * Copyright (C) 2019 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 * Desc: for Inter-Process Communication
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "IPC_msg.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

int             g_msgToUI = -1;
pthread_t       g_msgToUI_thread;
IPC_MSG_HANDLER g_pMsgHandler = NULL;

//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)

static int _send_message(struct IPCMsgBuf *qbuf)
{
    if (-1 == g_msgToUI)
        return -1;

    if (msgsnd(g_msgToUI, (struct msgbuf *)qbuf,
               sizeof(qbuf->eIPCMsgId) + sizeof(qbuf->u32ParamLen) + qbuf->u32ParamLen, 0)
        == -1)
    {
        printf("msg[%x]err[%02d][%s]\n", qbuf->eIPCMsgId, errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int _read_message(struct IPCMsgBuf *qbuf)
{
    if (msgrcv(g_msgToUI, (struct msgbuf *)qbuf,
               sizeof(qbuf->eIPCMsgId) + sizeof(qbuf->u32ParamLen) + IPC_MAX_MSG_BUFFER_SIZE, qbuf->mtype, 0)
        == -1)
    {
        return -1;
    }

    return 0;
}

static void *MsgToUI_Task(void *argu)
{
    struct IPCMsgBuf msgBuf;
    int              ret   = 0;
    int              retry = 0;

    printf("%s: run!\n", __FUNCTION__);
RETRY:
    while ((ret = _read_message(&msgBuf)) != -1)
    {
        retry = 0;
        if (IPC_MSG_EXIT == msgBuf.eIPCMsgId)
        {
            break;
        }

        if (g_pMsgHandler)
        {
            g_pMsgHandler(&msgBuf);
        }
    }

    if (ret == -1)
    {
        printf("msg recv err[%02d][%s] retry[%d]\n", errno, strerror(errno), retry);
        if (retry < 5)
        {
            retry++;
            goto RETRY;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

#endif

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int IPC_MsgToUI_Init(void)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    key_t msgid;
    msgid = ftok("/tmp/UI_Recv", IPC_MSG_KEY);
    if ((g_msgToUI = msgget(msgid, IPC_CREAT | 0660)) == -1)
    {
        printf("msgget failed errno.%02d is: %s\n", errno, strerror(errno));
        return -1;
    }
#endif
    return 0;
}

int IPC_MsgToUI_DeInit(void)
{
    // #if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    //     if (g_msgToUI != -1)
    //     {
    //         msgctl(g_msgToUI, IPC_RMID, 0);
    //     }
    // #endif
    return 0;
}

int IPC_MsgToUI_CreateThread(void)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    pthread_create(&g_msgToUI_thread, NULL, MsgToUI_Task, NULL);
#endif
    return 0;
}

int IPC_MsgToUI_DestroyThread(void)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    IPC_MsgToUI_SendMsg(IPC_MSG_EXIT, NULL, 0);
    pthread_join(g_msgToUI_thread, NULL);
#endif
    return 0;
}

int IPC_MsgToUI_RegisterMsgHandler(IPC_MSG_HANDLER pMsgHandler)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    g_pMsgHandler = pMsgHandler;
#endif
    return 0;
}

int IPC_MsgToUI_SendMsg(IPC_MSG_ID eIPCMsgId, char *param, unsigned int paramLen)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    struct IPCMsgBuf stMsgBuf;

    if (paramLen > IPC_MAX_MSG_BUFFER_SIZE)
    {
        printf("Send cmd failed. cmd=%x, paramLen=%d, param=0x%x\n", eIPCMsgId, paramLen, (int)param);
        return -1;
    }

    stMsgBuf.mtype       = 1;
    stMsgBuf.eIPCMsgId   = eIPCMsgId;
    stMsgBuf.u32ParamLen = paramLen;
    memcpy(stMsgBuf.s8ParamBuf, param, paramLen);
    _send_message(&stMsgBuf);
#endif
    return 0;
}
