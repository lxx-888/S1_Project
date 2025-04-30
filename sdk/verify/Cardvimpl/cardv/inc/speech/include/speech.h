/*
* speech.h- Sigmastar
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

#ifndef _SPEECH_H_
#define _SPEECH_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
//#include <sys/time.h>

#include "mi_common_datatype.h"
#include "mi_aio_datatype.h"
#include "mi_ai_datatype.h"
#define AUDIO_PT_NUMBER_FRAME (1024)

//toUI
#define MSG_TYPE_SMARTMIC_START 0x100
#define MSG_TYPE_SMARTMIC_STOP 0x101
#define MSG_TYPE_SMARTMIC_MACTCH 0x102

#define MSG_TYPE_FACEREGISTER_CTRL  0x103
#define MSG_TYPE_DISP_DETECT_FACE   0x104
#define MSG_TYPE_REMOTE_CALL_ME     0x105
#define MSG_UI_CALLER_HANGUP        0x106
#define MSG_UI_CALLED_HANGUP        0x107
#define MSG_UI_CREATE_MONITOR_WIN   0x108


//toAPP
#define MSG_TYPE_CONFIRM_REGSITER_FACE  0x200
#define MSG_TYPE_START_SEND_VIDEO       0x201
#define MSG_TYPE_STOP_SEND_VIDEO        0x202
#define MSG_TYPE_CREATE_XMLCFG          0x203
#define MSG_TYPE_PRASE_XMLCFG           0x204

#define MSG_TYPE_LOCAL_VIDEO_DISP       0x205
#define MSG_TYPE_LOCAL_START_MONITOR    0x206
#define MSG_TYPE_LOCAL_STOP_MONITOR     0x207

#define MSG_TYPE_RECV_MONITOR_CMD       0x208 //msg[1] == cmd  msg[2] == Start/Stop msg[3] == RemoteIPaddr
#define MSG_TYPE_RECV_IDLE_CMD          0x209 //msg[1] == cmd  msg[2] == ServerIp reserve msg[3] == sendsocket

#define MSG_TYPE_LOCAL_CALL_ROOM        0x20A //msg[1] == roomid msg[2]=msg[3]=reserve
#define MSG_TYPE_RECV_REMOTE_CALL_ROOM  0x20B //msg[1] == cmd  msg[2] == reserve msg[3] == RemoteIPaddr
#define MSG_TYPE_RECV_ROOM_BUSY         0x20C

#define MSG_TYPE_TALK_CALLED_HOLDON     0x20D
#define MSG_TYPE_TALK_CALLED_HANGUP     0x20E
#define MSG_TYPE_TALK_CALLER_HANGUP     0x20F

#define MSG_NET_RECV_ROOM_MONTACK_CMD   0x212 //msg[1] == cmd  msg[2] == ServerIp reserve msg[3] == sendsocket

#define MSG_NET_TALK_CALLED_HOLDON      0x213
#define MSG_NET_TALK_CALLED_HANGUP      0x214
#define MSG_NET_TALK_CALLER_HANGUP      0x215
#define MSG_TIMEOUT_NO_HOLDON_HANGUP    0x216 //not recv remote holdon
#define MSG_TIMEOUT_HOLDON_HANGUP       0x217 //talking timeout

//SocketProcess
#define MSG_TYPE_SOCKET_RECV_CONNECT 0x300
#define MSG_TYPE_SOCKET_SEND_PACKET 0x301
#define MSG_TYPE_SOCKET_REMOTE_HANGUP 0x302
#define MSG_TYPE_SOCKET_TIMEOUT_HANGUP 0x303
#define MSG_TYPE_SOCKET_RECV_NORMAL_PACK 0x304
#define MSG_TYPE_SOCKET_RECV_EXT_PACK 0x305

//Local call
#define MSG_TYPE_SOCKET_CONNECT_REMOTE 0x400
#define MSG_TYPE_SOCKET_SEND_LOCAL_CALL_CMD 0x401
#define MSG_TYPE_SOCKET_ANSWER_REMOTE_CMD 0x402

#define CSPOTTER_PATH_PREFIX        "/customer/CSpotter"
#define CSPOTTER_LIB_PATH           "/customer/CSpotter/lib"
#define CSPOTTER_DATA_PATH          "/customer/CSpotter/data"
#define CSPOTTER_DATA_TO_FILE       0
#define CSPOTTER_DATA_FROM_FILE     0

#define MAX_FRAME_QUEUE_DEPTH       64

typedef void (*VoiceAnalyzeprocessCB)(MI_U8);
typedef void (*ActivateStatusNotifyCB)(MI_S32);

MI_S32 Speech_VoiceAnalyzeStart();
MI_S32 Speech_VoiceAnalyzeStop();
MI_BOOL Speech_VoiceIsAnalyzeStart();

MI_S32 Speech_VoiceLearnStart();
MI_S32 Speech_VoiceLearnStop();

MI_S32 Speech_VoiceAnalyzeInit();
MI_S32 Speech_VoiceAnalyzeDeInit();

void   Speech_VoiceSendAudioFrame(MI_AI_Data_t *pstAudioFrame);

//txz callback
void Speech_VoiceAnalyzeprocessCallback(VoiceAnalyzeprocessCB pFunc);
void Speech_txzActivateStatusNotifyCallback(ActivateStatusNotifyCB pFunc);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif //_SPEECH_H_
