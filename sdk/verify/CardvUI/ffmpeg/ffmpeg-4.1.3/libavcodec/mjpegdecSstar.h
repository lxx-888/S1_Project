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

/**
 * @file
 * MJPEG codec.
 * @author jeff.li <jeff.li@sigmastar.com.cn>
 */

#ifndef AVCODEC_SSMJPEGDEC_H
#define AVCODEC_SSMJPEGDEC_H

#include "avcodec.h"
#include "libavcodec/internal.h"
//sstar sdk lib
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_jpd.h"
#include "mi_scl.h"

typedef struct SsMjpegContext {
    AVFrame *picture_ptr;
    MI_JPD_DEV JpdDev;
    MI_JPD_CHN JpdChn;
    MI_SCL_DEV SclDev;
    MI_SCL_CHANNEL SclChn;
    MI_SCL_PORT SclPort;
    MI_S32 s32Fd;
} SsMjpegContext;
#endif