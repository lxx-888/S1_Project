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

/*
 * H.26L/H.264/AVC/JVT/14496-10/... encoder/decoder
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_SSH264DEC_H
#define AVCODEC_SSH264DEC_H

#include "avcodec.h"
#include "h2645_parse.h"
#include "h264_ps.h"
#include "internal.h"

// sstar sdk lib
#include "mi_vdec.h"
#include "mi_sys.h"
/**
 * H264Context
 */
typedef struct SsH264Context
{
    AVCodecContext *avctx;
    H2645Packet     pkt;
    H264ParamSets   ps;
    uint8_t         decoder_initialized;
    int             is_avc;          ///< this flag is != 0 if codec is avc1
    int             nal_length_size; ///< Number of bytes used for nal length (1, 2 or 4)
    MI_VDEC_CHN     s32VdecChn;
    MI_S32          s32Fd;
    MI_U32          u32SendCnt;
    MI_U32          u32RecvCnt;
    MI_U16          u16DispW;
    MI_U16          u16DispH;
} SsH264Context;
#endif /* AVCODEC_SSH264DEC_H */