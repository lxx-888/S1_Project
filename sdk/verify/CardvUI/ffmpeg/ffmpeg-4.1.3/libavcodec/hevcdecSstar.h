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

#ifndef AVCODEC_SSHEVCDEC_H
#define AVCODEC_SSHEVCDEC_H

#include "avcodec.h"
#include "h2645_parse.h"
#include "internal.h"

// sstar sdk lib
#include "mi_vdec.h"
#include "mi_sys.h"

typedef struct SsHevcContext
{
    AVCodecContext *avctx;
    H2645Packet     pkt;
    HEVCSEI         sei;
    HEVCParamSets   ps;
    uint8_t         decoder_initialized;
    int             is_nalff;        ///< this flag is != 0 if bitstream is encapsulated as a format defined in 14496-15
    int             nal_length_size; ///< Number of bytes used for nal length (1, 2 or 4)
    MI_VDEC_CHN     s32VdecChn;
    MI_S32          s32Fd;
    MI_U32          u32SendCnt;
    MI_U32          u32RecvCnt;
    MI_U16          u16DispW;
    MI_U16          u16DispH;
} SsHevcContext;

#endif
