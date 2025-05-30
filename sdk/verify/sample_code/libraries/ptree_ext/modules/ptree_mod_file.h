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

#ifndef __PTREE_MOD_FILE_H__
#define __PTREE_MOD_FILE_H__

#include "ptree_mod.h"

void PTREE_MOD_FILE_SetLeftFrame(PTREE_MOD_InObj_t* inMod, unsigned int frameCnt);
int  PTREE_MOD_FILE_WaitFrame(PTREE_MOD_InObj_t* inMod, unsigned int waitMs);

#endif //__PTREE_MOD_FILE_H__
