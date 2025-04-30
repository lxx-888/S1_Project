/*
 * main.cpp- Sigmastar
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

#include "mid_common.h"
#include "module_common.h"
#include "module_config.h"

#include "IPC_msg.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define MSG_TYPE_HIGH_PRIO     (1)
#define MSG_TYPE_LOW_PRIO      (2)
#define MSG_TYPE_WAIT_ALL      (-2)
#define MSG_TYPE_ACK           (3)
#define MSG_MAX_Q_NUM_LOW_PRIO (16)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

struct CarDVMsgBuf
{
    long       mtype;
    CarDVCmdId cmdId;
    MI_S32     s32Wait;
    MI_S32     paramLen;
    MI_S8      paramBuf[CARDV_MAX_MSG_BUFFER_SIZE];
};

struct CarDVMsgAck
{
    long       mtype;
    CarDVCmdId cmdId;
    MI_S32     s32Ret;
};

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static int       g_msgQueueId = -1;
static pthread_t pthread_message;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static int _send_message(struct CarDVMsgBuf *qbuf, bool bHighPriority)
{
    if (bHighPriority == FALSE)
    {
        struct msqid_ds buf;
        if (msgctl(g_msgQueueId, IPC_STAT, &buf) == -1)
        {
            printf("msgctl err[%02d][%s]\n", errno, strerror(errno));
            return -1;
        }

        if (buf.msg_qnum > MSG_MAX_Q_NUM_LOW_PRIO)
            return -1;
    }

    if (msgsnd(g_msgQueueId, (void *)qbuf,
               sizeof(qbuf->cmdId) + sizeof(qbuf->s32Wait) + sizeof(qbuf->paramLen) + qbuf->paramLen, 0)
        == -1)
    {
        printf("msg[%x]err[%02d][%s]\n", qbuf->cmdId, errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int _read_message(struct CarDVMsgBuf *qbuf)
{
    size_t msgsz = sizeof(qbuf->cmdId) + sizeof(qbuf->s32Wait) + sizeof(qbuf->paramLen) + CARDV_MAX_MSG_BUFFER_SIZE;

    if (msgrcv(g_msgQueueId, (void *)qbuf, msgsz, MSG_TYPE_HIGH_PRIO, IPC_NOWAIT) == -1)
    {
        // No high priority msg, than recv all msg.
        if (msgrcv(g_msgQueueId, (void *)qbuf, msgsz, MSG_TYPE_WAIT_ALL, 0) == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int _send_ack(struct CarDVMsgAck *qbuf)
{
    if (msgsnd(g_msgQueueId, (void *)qbuf, sizeof(qbuf->cmdId) + sizeof(qbuf->s32Ret), 0) == -1)
    {
        printf("msg[%x]err[%02d][%s]\n", qbuf->cmdId, errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int _wait_ack(struct CarDVMsgAck *qbuf)
{
    if (msgrcv(g_msgQueueId, (void *)qbuf, sizeof(qbuf->cmdId) + sizeof(qbuf->s32Ret), MSG_TYPE_ACK, 0) == -1)
    {
        return -1;
    }

    return 0;
}

void *Msg_Task(void *argu)
{
    CarDVMsgBuf msgBuf;
    int         ret   = 0;
    int         retry = 0;

    CARDV_THREAD();

RETRY:
    while ((ret = _read_message(&msgBuf)) != -1)
    {
        retry = 0;
        switch (msgBuf.cmdId & CARDV_CMD_TYPE)
        {
        case CMD_SYSTEM:
            ret = system_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;

        case CMD_PIPE:
            ret = video_pipe_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;

        case CMD_VIDEO:
            ret = video_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;

        case CMD_AUDIO:
            ret = audio_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;

#if CARDV_OSD_ENABLE
        case CMD_OSD:
            ret = osd_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

#if CARDV_DISPLAY_ENABLE
        case CMD_DISP:
            ret = display_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

#if CARDV_ISP_ENABLE
        case CMD_ISP:
            ret = isp_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

#if CARDV_VDF_ENABLE
        case CMD_VDF:
            ret = vdf_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

#if CARDV_UVC_ENABLE
        case CMD_UVC:
            ret = uvc_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

        case CMD_MUX:
            ret = mux_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;

#if CARDV_DMS_ENABLE
        case CMD_DMS:
            ret = dms_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

#if CARDV_ADAS_ENABLE
        case CMD_ADAS:
            ret = adas_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif

#if CARDV_LD_ENABLE
        case CMD_LD:
            ret = lightdetect_process_cmd(msgBuf.cmdId, msgBuf.paramBuf, msgBuf.paramLen);
            break;
#endif
        }

        if (msgBuf.s32Wait)
        {
            CarDVMsgAck msgAck;
            msgAck.mtype  = MSG_TYPE_ACK;
            msgAck.cmdId  = msgBuf.cmdId;
            msgAck.s32Ret = ret;
            _send_ack(&msgAck);
        }

        if (CMD_SYSTEM_EXIT_QUEUE == msgBuf.cmdId)
        {
            break;
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

int cardv_send_cmd(CarDVCmdId cmdId, MI_S8 *param, MI_S32 paramLen)
{
    CarDVMsgBuf msgBuf;

    if (paramLen > CARDV_MAX_MSG_BUFFER_SIZE)
    {
        printf("Send cmd failed. cmd=%x, paramLen=%d\n", cmdId, paramLen);
        return -1;
    }

    msgBuf.mtype    = MSG_TYPE_LOW_PRIO;
    msgBuf.cmdId    = cmdId;
    msgBuf.s32Wait  = 0;
    msgBuf.paramLen = paramLen;
    memcpy(msgBuf.paramBuf, param, paramLen);
    _send_message(&msgBuf, FALSE);

    return 0;
}

int cardv_send_cmd_HP(CarDVCmdId cmdId, MI_S8 *param, MI_S32 paramLen)
{
    CarDVMsgBuf msgBuf;

    if (paramLen > CARDV_MAX_MSG_BUFFER_SIZE)
    {
        printf("Send cmd failed. cmd=%x, paramLen=%d\n", cmdId, paramLen);
        return -1;
    }

    msgBuf.mtype    = MSG_TYPE_HIGH_PRIO;
    msgBuf.cmdId    = cmdId;
    msgBuf.s32Wait  = 0;
    msgBuf.paramLen = paramLen;
    memcpy(msgBuf.paramBuf, param, paramLen);
    _send_message(&msgBuf, TRUE);

    return 0;
}

int cardv_send_cmd_and_wait(CarDVCmdId cmdId, MI_S8 *param, MI_S32 paramLen)
{
    CarDVMsgBuf msgBuf;
    CarDVMsgAck msgAck;

    if (paramLen > CARDV_MAX_MSG_BUFFER_SIZE)
    {
        printf("Send cmd failed. cmd=%x, paramLen=%d\n", cmdId, paramLen);
        return -1;
    }

    msgBuf.mtype    = MSG_TYPE_HIGH_PRIO;
    msgBuf.cmdId    = cmdId;
    msgBuf.s32Wait  = 1;
    msgBuf.paramLen = paramLen;
    memcpy(msgBuf.paramBuf, param, paramLen);
    if (_send_message(&msgBuf, TRUE) == 0)
    {
        _wait_ack(&msgAck);
        return msgAck.s32Ret;
    }

    return -1;
}

int cardv_message_queue_init()
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_IFORD)
    if (IPC_MsgToUI_Init() == -1)
    {
        printf("IPC_Msg_Init FAIL\n");
        return -1;
    }
#endif

    if ((g_msgQueueId = msgget(IPC_PRIVATE, IPC_CREAT | 0660)) == -1)
    {
        printf("msgget failed errno.%02d is: %s\n", errno, strerror(errno));
        return -1;
    }

    pthread_create(&pthread_message, NULL, Msg_Task, NULL);
    pthread_setname_np(pthread_message, "Msg_Task");

    return 0;
}

int cardv_message_queue_uninit()
{
    cardv_send_cmd_HP(CMD_SYSTEM_EXIT_QUEUE, NULL, 0);
    pthread_join(pthread_message, NULL);

    /* Remove the queue */
    if (g_msgQueueId != -1)
    {
        msgctl(g_msgQueueId, IPC_RMID, 0);
    }

#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_IFORD)
    IPC_MsgToUI_DeInit();
#endif

    return 0;
}
