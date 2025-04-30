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

#ifndef __PTREE_SUR_MUX_H__
#define __PTREE_SUR_MUX_H__
#include "ptree_sur.h"
#include "ptree_packet.h"

enum PTREE_SUR_MUX_Type_e
{
    E_PTREE_SUR_MUX_TYPE_TS,
    E_PTREE_SUR_MUX_TYPE_MP4,
};

typedef struct PTREE_SUR_MUX_Info_s
{
    PTREE_SUR_Info_t base;
    unsigned int     bitRateV;
    unsigned int     bitRateA;
    unsigned int     audioFrameSize;
    unsigned int     format;
    char             fileName[64];
} PTREE_SUR_MUX_Info_t;

typedef struct PTREE_SUR_MUX_InInfo_s
{
    PTREE_SUR_InInfo_t base;
} PTREE_SUR_MUX_InInfo_t;

typedef struct PTREE_SUR_MUX_OutInfo_s
{
    PTREE_SUR_OutInfo_t base;
} PTREE_SUR_MUX_OutInfo_t;

#endif //__PTREE_MOD_SCL_H__
