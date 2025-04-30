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
#ifndef _ST_GLASSES_PROTCOL_H
#define _ST_GLASSES_PROTCOL_H

#include "../../../applications/ai_glasses/mcu/Inc/ss_datatype.h"

#ifndef LINUX_OS
typedef unsigned char  MI_U8;  // 1 byte
typedef unsigned short MI_U16; // 2 bytes
typedef signed int     MI_S32; // 4 bytes
typedef unsigned int   MI_U32; // 4 bytes

#define MI_SUCCESS      (0)
#define CamOsMemAlloc   malloc
#define CamOsMemRelease free
#endif
typedef struct ST_Protrcol_Msg_s
{
    BOOL  isIndeLengMsg;
    char *msg;
    MI_U8 length;
} ST_Protrcol_Msg_t;

typedef struct ST_Protrcol_Task_s
{
    SS_CMD_TYPE_e eCmdType;
    SS_TASK_e     eTaskCmd;
    BOOL          isReject;
    SS_STATE_e    eSocState;
    MI_U16        u16UserDefine; /* [0 - 7][8 - 15] */
                                 /* taskId photoCnt */
#ifdef SS_USE_TWS_LIGHT_SENSOR
    MI_U16        u16LuxValue;   /* only E_TASK_PHOTO*/
                                 /* E_TASK_START_REC need*/
#endif
    ST_Protrcol_Msg_t *pstProtMsg;
} ST_Protrcol_Task_t;

MI_S32 ST_Common_AiGlasses_Prot_Deal_Uart_Buffer(char *uartBuf, MI_U32 len, BOOL useDma,
                                                 MI_S32 (*protcolTaskHandler)(ST_Protrcol_Task_t *pstProtcolTask));
MI_S32 ST_Common_AiGlasses_Prot_Make_Cmd_To_Send(ST_Protrcol_Task_t *pstProtTask,
                                                 MI_S32 (*uartSendFun)(char *, MI_U32));
#endif
