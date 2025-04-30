/*
 * IPC_msg.h - Sigmastar
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

#ifndef _IPC_MSG_H_
#define _IPC_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define IPC_MSG_KEY             0x111
#define IPC_MAX_MSG_BUFFER_SIZE 128

//==============================================================================
//
//                              ENUM & STRUCT DEFINES
//
//==============================================================================

typedef enum _IPC_MSG_ID
{
    IPC_MSG_UI_UPDATE_ALL = 0x01,
    IPC_MSG_UI_SD_MOUNT,
    IPC_MSG_UI_SD_PRE_MOUNT,
    IPC_MSG_UI_SD_UNMOUNT,
    IPC_MSG_UI_SD_NEED_FORMAT,
    IPC_MSG_UI_FORMAT_SD_OK,
    IPC_MSG_UI_FORMAT_SD_FAILED,
    IPC_MSG_UI_SPEECH_POWEROFF,
    IPC_MSG_UI_SETTING_UPDATE,
    IPC_MSG_UI_SD_NOCARD,
    IPC_MSG_UI_STOP,
    IPC_MSG_EXIT = 0xFF,
} IPC_MSG_ID;

struct IPCMsgBuf
{
    long         mtype;
    IPC_MSG_ID   eIPCMsgId;
    unsigned int u32ParamLen;
    char         s8ParamBuf[IPC_MAX_MSG_BUFFER_SIZE];
};

typedef void (*IPC_MSG_HANDLER)(struct IPCMsgBuf *);

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int IPC_MsgToUI_Init(void);
int IPC_MsgToUI_DeInit(void);
int IPC_MsgToUI_CreateThread(void);
int IPC_MsgToUI_DestroyThread(void);
int IPC_MsgToUI_RegisterMsgHandler(IPC_MSG_HANDLER pMsgHandler);
int IPC_MsgToUI_SendMsg(IPC_MSG_ID cmdId, char *param, unsigned int paramLen);

#ifdef __cplusplus
}
#endif

#endif // _IPC_MSG_H_
