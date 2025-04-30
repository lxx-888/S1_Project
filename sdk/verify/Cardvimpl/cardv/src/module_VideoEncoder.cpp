/*
 * module_VideoEncoder.cpp- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

using namespace std;

#include <algorithm>
#include <math.h>
#include <stdlib.h>

#include "mid_common.h"
#include "mi_venc.h"
#include "mi_vdisp.h"
#include "mi_scl.h"
#include "mi_jpd.h"

#include "mid_VideoEncoder.h"
#include "mid_mux.h"
#include "module_config.h"
#include "module_video.h"
#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
#include "module_ipc.h"
#endif
#if (CARDV_RTSP_INPUT_ENABLE)
#include "module_rtspclient.h"
#endif
#if (CARDV_WS_INPUT_ENABLE || CARDV_WS_OUTPUT_ENABLE)
#include "module_ws.h"
#endif

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define TEST_FIXQP           (0) // Only for test large bitrate.
#define IMI_JPEG_LIMIT_WIDTH (4096)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static MI_S32 g_s32VencChnCreated[MI_VENC_DEV_ID_JPEG_0 + 1] = {0};

extern MI_VideoEncoder *g_videoEncoderArray[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_Venc_StopChannel(MI_VENC_DEV VencDev, MI_VENC_CHN VencChn)
{
    MI_S32 s32Ret;
    MI_U8  u8Retry = 0;

    do
    {
        s32Ret = MI_VENC_StopRecvPic(VencDev, VencChn);
    } while (s32Ret == MI_ERR_VENC_BUSY && u8Retry++ < 10 && usleep(10000) == 0);
    if (s32Ret != MI_SUCCESS)
        return s32Ret;

    return MI_VENC_ResetChn(VencDev, VencChn);
}

void *Stream_Task(void *argv)
{
    MI_VideoEncoder * pVideoEncoder = (MI_VideoEncoder *)argv;
    MI_VENC_Stream_t  stStream;
    MI_VENC_Pack_t    stPack[STREAM_PACK_CNT];
    MI_VENC_ChnStat_t stStat;
    MI_S32            s32Ret = MI_SUCCESS;

    CARDV_THREAD();
    CARDV_INFO("Venc%u Enter\n", pVideoEncoder->getVencChn());

    while (pVideoEncoder->getStreamStatus())
    {
#ifdef DUAL_OS
    RETRY_GET_STREAM:
#endif
        if (pVideoEncoder->pollingStream() < 0)
        {
            continue;
        }

        memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
        s32Ret = pVideoEncoder->Query(&stStat);
        if (MI_SUCCESS != s32Ret || stStat.u32CurPacks == 0)
        {
            CARDV_ERR("Venc%u Query Fail\n", pVideoEncoder->getVencChn());
            usleep(10000); // sleep 10 ms
            continue;
        }

        memset(&stStream, 0, sizeof(stStream));
        memset(&stPack[0], 0, sizeof(stPack[0]) * STREAM_PACK_CNT);
        stStream.pstPack      = &stPack[0];
        stStream.u32PackCount = stStat.u32CurPacks;
        if (stStat.u32CurPacks > STREAM_PACK_CNT)
            CARDV_ERR("CurPacks [%d] > STREAM_PACK_CNT\n", stStat.u32CurPacks);

        s32Ret = pVideoEncoder->GetStream(&stStream, 0);
        if (MI_SUCCESS == s32Ret)
        {
            pVideoEncoder->handlerStream(&stStream);
            pVideoEncoder->ReleaseStream(&stStream);
        }
        else
        {
            CARDV_ERR("Venc%u Get Stream Fail %x\n", pVideoEncoder->getVencChn(), s32Ret);
        }

        pVideoEncoder->handlerStreamPost();

#ifdef DUAL_OS
        if (stStat.u32LeftStreamFrames > 1)
        {
            CARDV_INFO("Venc%u Left Frames %d\n", pVideoEncoder->getVencChn(), stStat.u32LeftStreamFrames);
            goto RETRY_GET_STREAM;
        }
#endif
    }

    CARDV_INFO("Venc%u Exit\n", pVideoEncoder->getVencChn());
    pthread_exit(NULL);
}

#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
void IpcServerConnect(MI_S32 s32SvrSock, void *pArg, MI_U32 u32ArgLen)
{
    CarDV_StreamLinkNode_t *pLinkNode = (CarDV_StreamLinkNode_t *)pArg;
    pLinkNode->s32Sock                = s32SvrSock;
#if (CARDV_RTSP_INPUT_ENABLE)
    CarDV_RtspClientAddLink(pLinkNode);
#elif (CARDV_WS_INPUT_ENABLE)
    CarDV_WsClientAddLink(pLinkNode);
#endif
}

void IpcServerDisConnect(void *pArg, MI_U32 u32ArgLen)
{
    CarDV_StreamLinkNode_t *pLinkNode = (CarDV_StreamLinkNode_t *)pArg;
#if (CARDV_RTSP_INPUT_ENABLE)
    CarDV_RtspClientRemoveLink(pLinkNode);
#elif (CARDV_WS_INPUT_ENABLE)
    CarDV_WsClientRemoveLink(pLinkNode);
#endif
    FREEIF(pLinkNode);
}
#endif

S32 MI_VideoEncoder::initVideoEncoder()
{
    MI_VENC_ChnAttr_t stChnAttr;
#if !(TEST_FIXQP)
    MI_VENC_RcParam_t       stRcParam;
    MI_VENC_SuperFrameCfg_t stSuperFrameCfg;
    MI_VENC_RcPriority_e    eRcPriority;
#endif
    MI_U32 u32BufSize = 0;
    MI_U32 u32Num, u32Den, u32Gcd;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));

#ifdef DUAL_OS
    MI_VENC_DupChn(m_s32VencDev, m_s32VencChn);
    if (m_stVencAttr.eVideoType == VIDEO_STREAM_CAPTURE)
    {
        // in rtos, JPE stream is on. only stop capture, don't stop thumbnail.
        CarDV_Venc_StopChannel(m_s32VencDev, m_s32VencChn);
    }
#endif

    CARDVCHECKRESULT(internalInit());

    if (m_stVencAttr.bPassVenc)
    {
#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
        CarDV_IpcParam_t        stParam;
        CarDV_StreamLinkNode_t *pLinkNode = (CarDV_StreamLinkNode_t *)MALLOC(sizeof(CarDV_StreamLinkNode_t));
        if (pLinkNode == NULL)
        {
            return -1;
        }
        pLinkNode->u8Chn       = m_stVencAttr.u8ExtStreamChn;
        pLinkNode->pEncoder    = this;
        pLinkNode->bStart      = FALSE;
        stParam.pSvrConnect    = IpcServerConnect;
        stParam.pSvrDisConnect = IpcServerDisConnect;
        stParam.pArg           = pLinkNode;
        stParam.u32ArgLen      = sizeof(CarDV_StreamLinkNode_t);
        m_s32VencFd            = IpcClientConnect(&stParam);
        if (m_s32VencFd < 0)
        {
            printf("venc[%d] get fd fail\n", m_s32VencChn);
            return -1;
        }
#endif
        return 0;
    }

    if (E_MI_MODULE_ID_VENC != m_stVencAttr.stVencInBindParam.stChnPort.eModId)
    {
        CARDVCHECKRESULT(VideoInitResolution(&m_stVencAttr.stVencInBindParam.stChnPort, &m_stVencAttr.u32Width,
                                             &m_stVencAttr.u32Height));
    }

    if (E_MI_MODULE_ID_SCL == m_stVencAttr.stVencInBindParam.stChnPort.eModId
        && m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_HW_RING
        && g_s32VencChnCreated[MI_VENC_DEV_ID_H264_H265_0] > 0 && m_stVencAttr.u32Width > IMI_JPEG_LIMIT_WIDTH)
    {
        // 8838G Sram size is 270KB. IMI JPEG Capture 4K width need 128KB, and H26X device need 120KB.
        MI_SYS_WindowSize_t stPortSize;
        stPortSize.u16Width  = IMI_JPEG_LIMIT_WIDTH;
        stPortSize.u16Height = IMI_JPEG_LIMIT_WIDTH * m_stVencAttr.u32Height / m_stVencAttr.u32Width;
        CARDVCHECKRESULT(VideoSetInputResolution(&m_stVencAttr.stVencInBindParam.stChnPort, stPortSize.u16Width,
                                                 stPortSize.u16Height));
        m_stVencAttr.u32Width  = stPortSize.u16Width;
        m_stVencAttr.u32Height = stPortSize.u16Height;
    }

    if (m_stVencAttr.u32BufSize == 0)
    {
        if (m_stVencAttr.u8EncoderType == VE_H264 || m_stVencAttr.u8EncoderType == VE_H265)
            u32BufSize = m_stVencAttr.u32MaxWidth * m_stVencAttr.u32MaxHeight >> 1;
        else
            u32BufSize = m_stVencAttr.u32MaxWidth * m_stVencAttr.u32MaxHeight >> 2;

        if (u32BufSize <= (640 * 480 >> 1))
            u32BufSize = u32BufSize << 1;
    }
    else
    {
        u32BufSize = m_stVencAttr.u32BufSize;
    }

    u32Num = m_stVencAttr.u32VencFrameRate & 0xffff;
    u32Den = (m_stVencAttr.u32VencFrameRate >> 16) & 0xffff;
    if (0x0 == u32Den)
        u32Den = 1;
    else
    {
        u32Gcd = __gcd(u32Num, u32Den);
        if (u32Gcd > 1)
        {
            u32Num /= u32Gcd;
            u32Den /= u32Gcd;
        }
    }

    switch (m_stVencAttr.u8EncoderType)
    {
    case VE_H264:
        stChnAttr.stVeAttr.stAttrH264e.u32Profile      = 2; // 0: baseline  1: main profile  2: high profile
        stChnAttr.stVeAttr.stAttrH264e.u32PicWidth     = ALIGN_8xUP(m_stVencAttr.u32Width);
        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight    = ALIGN_2xUP(m_stVencAttr.u32Height);
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth  = ALIGN_8xUP(m_stVencAttr.u32MaxWidth);
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = ALIGN_2xUP(m_stVencAttr.u32MaxHeight);
        stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum    = 2;
        stChnAttr.stVeAttr.stAttrH264e.bByFrame        = TRUE;
        stChnAttr.stVeAttr.stAttrH264e.u32BufSize      = u32BufSize;
#if !(TEST_FIXQP)
        if (m_stVencAttr.u8VBR)
        {
            stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H264VBR;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate    = m_stVencAttr.u32BitRate;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp         = m_stVencAttr.u32MaxPQp;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp         = m_stVencAttr.u32MinPQp;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop           = m_stVencAttr.u32Gop;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = u32Num;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = u32Den;
        }
        else
        {
            stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H264CBR;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate       = m_stVencAttr.u32BitRate;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop           = m_stVencAttr.u32Gop;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = u32Num;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = u32Den;
        }
#else
        stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_H264FIXQP;
        stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp           = 16;
        stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp           = 16;
        stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop           = m_stVencAttr.u32Gop;
        stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = u32Num;
        stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = u32Den;
#endif
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
        break;

    case VE_H265:
        stChnAttr.stVeAttr.stAttrH265e.u32PicWidth     = ALIGN_8xUP(m_stVencAttr.u32Width);
        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight    = ALIGN_2xUP(m_stVencAttr.u32Height);
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth  = ALIGN_8xUP(m_stVencAttr.u32MaxWidth);
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = ALIGN_2xUP(m_stVencAttr.u32MaxHeight);
        stChnAttr.stVeAttr.stAttrH265e.bByFrame        = TRUE;
        stChnAttr.stVeAttr.stAttrH265e.u32BufSize      = u32BufSize;
#if !(TEST_FIXQP)
        if (m_stVencAttr.u8VBR)
        {
            stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate    = m_stVencAttr.u32BitRate;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp         = m_stVencAttr.u32MaxPQp;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp         = m_stVencAttr.u32MinPQp;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop           = m_stVencAttr.u32Gop;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = u32Num;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = u32Den;
        }
        else
        {
            stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H265CBR;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate       = m_stVencAttr.u32BitRate;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop           = m_stVencAttr.u32Gop;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = u32Num;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = u32Den;
        }
#else
        stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_H265FIXQP;
        stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp           = 14;
        stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp           = 14;
        stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop           = m_stVencAttr.u32Gop;
        stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = u32Num;
        stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = u32Den;
#endif
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
        break;

    case VE_JPG:
        stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth            = ALIGN_8xUP(m_stVencAttr.u32Width);
        stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight           = ALIGN_2xUP(m_stVencAttr.u32Height);
        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth         = ALIGN_8xUP(m_stVencAttr.u32MaxWidth);
        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight        = ALIGN_2xUP(m_stVencAttr.u32MaxHeight);
        stChnAttr.stVeAttr.stAttrJpeg.bByFrame               = TRUE;
        stChnAttr.stVeAttr.stAttrJpeg.u32BufSize             = u32BufSize;
        stChnAttr.stRcAttr.eRcMode                           = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor       = m_stVencAttr.u32Qfactor;
        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = u32Num;
        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = u32Den;
        stChnAttr.stVeAttr.eType                             = E_MI_VENC_MODTYPE_JPEGE;
        break;

    default:
        CARDV_ERR("error encoder type:%d\n", m_stVencAttr.u8EncoderType);
        break;
    }

    if (g_s32VencChnCreated[m_s32VencDev] == 0)
    {
        MI_VENC_InitParam_t stInitParam = {0};
        if (m_s32VencDev == MI_VENC_DEV_ID_H264_H265_0)
        {
            stInitParam.u32MaxWidth  = H26X_MAX_ENCODE_WIDTH;
            stInitParam.u32MaxHeight = H26X_MAX_ENCODE_HEIGHT;
        }
        else if (m_s32VencDev == MI_VENC_DEV_ID_JPEG_0)
        {
            stInitParam.u32MaxWidth  = JPEG_MAX_ENCODE_WIDTH;
            stInitParam.u32MaxHeight = JPEG_MAX_ENCODE_HEIGHT;
        }
        CARDVCHECKRESULT_OS(MI_VENC_CreateDev(m_s32VencDev, &stInitParam));

        if (m_s32VencDev == MI_VENC_DEV_ID_H264_H265_0 && gstVencPrivPoolCfg.bConfigPrivPool)
        {
            MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
            memset(&stPrivPoolCfg, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
            stPrivPoolCfg.bCreate                                         = TRUE;
            stPrivPoolCfg.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule      = E_MI_MODULE_ID_VENC;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = m_s32VencDev;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = gstVencPrivPoolCfg.u16PrivPoolMaxWidth;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = gstVencPrivPoolCfg.u16PrivPoolMaxHeight;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = gstVencPrivPoolCfg.u16PrivPoolRingLine;
            CARDVCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg));
        }
    }

    CARDVCHECKRESULT_OS(MI_VENC_CreateChn(m_s32VencDev, m_s32VencChn, &stChnAttr));
    g_s32VencChnCreated[m_s32VencDev]++;

    if (m_stVencAttr.u8EncoderType == VE_H264 || m_stVencAttr.u8EncoderType == VE_H265)
    {
        MI_VENC_ParamRef_t stParamRef;
        stParamRef.u32Base     = 1;
        stParamRef.bEnablePred = FALSE;
        stParamRef.u32Enhance  = 0;
        // for insertting skip frame, only use this ref setting.
        ExecFunc_OS(MI_VENC_SetRefParam(m_s32VencDev, m_s32VencChn, &stParamRef), MI_SUCCESS);

#if !(TEST_FIXQP)
        if (m_stVencAttr.stVencInBindParam.u32BindParam == E_MI_SYS_BIND_TYPE_FRAME_BASE)
        {
            stSuperFrameCfg.eSuperFrmMode       = E_MI_VENC_SUPERFRM_REENCODE;
            stSuperFrameCfg.u32SuperIFrmBitsThr = (u32BufSize << 3) * 3 / 5;  // more than 60% buffer size
            stSuperFrameCfg.u32SuperPFrmBitsThr = (u32BufSize << 3) * 4 / 10; // more than 40% buffer size
            stSuperFrameCfg.u32SuperBFrmBitsThr = 2;                          // debug level
            ExecFunc_OS(MI_VENC_SetSuperFrameCfg(m_s32VencDev, m_s32VencChn, &stSuperFrameCfg), MI_SUCCESS);
        }

        eRcPriority = E_MI_VENC_RC_PRIORITY_FRAMEBITS_FIRST;
        ExecFunc_OS(MI_VENC_SetRcPriority(m_s32VencDev, m_s32VencChn, &eRcPriority), MI_SUCCESS);

        memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
        ExecFunc(MI_VENC_GetRcParam(m_s32VencDev, m_s32VencChn, &stRcParam), MI_SUCCESS);
        switch (m_stVencAttr.u8EncoderType)
        {
        case VE_H264:
            if (m_stVencAttr.u8VBR)
            {
                stRcParam.stParamH264VBR.u32MaxIQp    = m_stVencAttr.u32MaxIQp;
                stRcParam.stParamH264VBR.u32MinIQp    = m_stVencAttr.u32MinIQp;
                stRcParam.stParamH264VBR.s32IPQPDelta = m_stVencAttr.s32IPQPDelta;
                stRcParam.stParamH264VBR.s32ChangePos = m_stVencAttr.s32ChangePos;
            }
            else
            {
                stRcParam.stParamH264Cbr.u32MaxQp     = m_stVencAttr.u32MaxPQp;
                stRcParam.stParamH264Cbr.u32MinQp     = m_stVencAttr.u32MinPQp;
                stRcParam.stParamH264Cbr.u32MaxIQp    = m_stVencAttr.u32MaxIQp;
                stRcParam.stParamH264Cbr.u32MinIQp    = m_stVencAttr.u32MinIQp;
                stRcParam.stParamH264Cbr.s32IPQPDelta = m_stVencAttr.s32IPQPDelta;
            }
            break;

        case VE_H265:
            if (m_stVencAttr.u8VBR)
            {
                stRcParam.stParamH265Vbr.u32MaxIQp    = m_stVencAttr.u32MaxIQp;
                stRcParam.stParamH265Vbr.u32MinIQp    = m_stVencAttr.u32MinIQp;
                stRcParam.stParamH265Vbr.s32IPQPDelta = m_stVencAttr.s32IPQPDelta;
                stRcParam.stParamH265Vbr.s32ChangePos = m_stVencAttr.s32ChangePos;
            }
            else
            {
                stRcParam.stParamH265Cbr.u32MaxQp     = m_stVencAttr.u32MaxPQp;
                stRcParam.stParamH265Cbr.u32MinQp     = m_stVencAttr.u32MinPQp;
                stRcParam.stParamH265Cbr.u32MaxIQp    = m_stVencAttr.u32MaxIQp;
                stRcParam.stParamH265Cbr.u32MinIQp    = m_stVencAttr.u32MinIQp;
                stRcParam.stParamH265Cbr.s32IPQPDelta = m_stVencAttr.s32IPQPDelta;
            }
            break;
        default:
            break;
        }
        ExecFunc_OS(MI_VENC_SetRcParam(m_s32VencDev, m_s32VencChn, &stRcParam), MI_SUCCESS);
#endif

        if (m_stVencAttr.u8EncoderType == VE_H264)
        {
            MI_VENC_ParamH264Vui_t stH264Vui;
            ExecFunc_OS(MI_VENC_GetH264Vui(m_s32VencDev, m_s32VencChn, &stH264Vui), MI_SUCCESS);
            stH264Vui.stVuiTimeInfo.u32NumUnitsInTick = 1000;
            stH264Vui.stVuiTimeInfo.u32TimeScale      = 2000 / u32Den * u32Num;
            ExecFunc_OS(MI_VENC_SetH264Vui(m_s32VencDev, m_s32VencChn, &stH264Vui), MI_SUCCESS);
        }
        else if (m_stVencAttr.u8EncoderType == VE_H265)
        {
            MI_VENC_ParamH265Vui_t stH265Vui;
            ExecFunc_OS(MI_VENC_GetH265Vui(m_s32VencDev, m_s32VencChn, &stH265Vui), MI_SUCCESS);
            stH265Vui.stVuiTimeInfo.u32NumUnitsInTick = 1000;
            stH265Vui.stVuiTimeInfo.u32TimeScale      = 1000 / u32Den * u32Num;
            ExecFunc_OS(MI_VENC_SetH265Vui(m_s32VencDev, m_s32VencChn, &stH265Vui), MI_SUCCESS);
        }
    }

    if (m_stVencAttr.u8EncoderType == VE_H265)
    {
        MI_VENC_ParamH265Dblk_t stH265Dblk;
        stH265Dblk.disable_deblocking_filter_idc = 0;
        stH265Dblk.slice_beta_offset_div2        = 3;
        stH265Dblk.slice_tc_offset_div2          = 3;
        ExecFunc(MI_VENC_SetH265Dblk(m_s32VencDev, m_s32VencChn, &stH265Dblk), MI_SUCCESS);
    }
    else if (m_stVencAttr.u8EncoderType == VE_H264)
    {
        MI_VENC_ParamH264Dblk_s stH264Dblk;
        stH264Dblk.disable_deblocking_filter_idc = 0;
        stH264Dblk.slice_alpha_c0_offset_div2    = 6;
        stH264Dblk.slice_beta_offset_div2        = 6;
        ExecFunc(MI_VENC_SetH264Dblk(m_s32VencDev, m_s32VencChn, &stH264Dblk), MI_SUCCESS);
    }

    if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_REALTIME
        && m_stVencAttr.stVencInBindParam.stChnPort.eModId == E_MI_MODULE_ID_SCL)
    {
        // IMI JPEG 48M pixels cause frame rate issue, disable SCL output port until venc channel start.
        CARDVCHECKRESULT(VideoEnableInput(&m_stVencAttr.stVencInBindParam.stChnPort, FALSE));
    }
    else
    {
        // IMI JPEG NOT bind until venc channel start.
        bindVideoEncoder(TRUE);
    }

    m_s32VencFd = MI_VENC_GetFd(m_s32VencDev, m_s32VencChn);
    if (m_s32VencFd < 0)
    {
        CARDV_ERR("venc[%d] get fd fail\n", m_s32VencChn);
        return -1;
    }

#if (CARDV_WS_OUTPUT_ENABLE)
    if (m_stVencAttr.eVideoType == VIDEO_STREAM_USER)
    {
        CarDV_WsServerStart(m_s32VencChn, this);
    }
#endif

    return 0;
}

S32 MI_VideoEncoder::uninitVideoEncoder()
{
#if (CARDV_WS_OUTPUT_ENABLE)
    if (m_stVencAttr.eVideoType == VIDEO_STREAM_USER)
    {
        CarDV_WsServerStop(m_s32VencChn);
    }
#endif

    stopGetStreamThread();
    CARDVCHECKRESULT(internalDeInit());

    if (m_stVencAttr.bPassVenc)
    {
#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
        CARDVCHECKRESULT(IpcClientDisConnect(m_s32VencFd));
#endif
        return 0;
    }

    MI_VENC_CloseFd(m_s32VencDev, m_s32VencChn);
    bindVideoEncoder(FALSE);
    CARDVCHECKRESULT(MI_VENC_DestroyChn(m_s32VencDev, m_s32VencChn));
    g_s32VencChnCreated[m_s32VencDev]--;
    if (g_s32VencChnCreated[m_s32VencDev] == 0)
    {
        if (m_s32VencDev == MI_VENC_DEV_ID_H264_H265_0 && gstVencPrivPoolCfg.bConfigPrivPool)
        {
            MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
            memset(&stPrivPoolCfg, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
            stPrivPoolCfg.bCreate                                     = FALSE;
            stPrivPoolCfg.eConfigType                                 = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule  = E_MI_MODULE_ID_VENC;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = m_s32VencDev;
            CARDVCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg));
        }

        CARDVCHECKRESULT(MI_VENC_DestroyDev(m_s32VencDev));
    }

    return 0;
}

S32 MI_VideoEncoder::bindVideoEncoder(MI_BOOL bBind)
{
    if (bBind == m_bBindEncoder)
        return 0;

    CarDV_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x00, sizeof(CarDV_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId    = m_stVencAttr.stVencInBindParam.stChnPort.eModId;
    stBindInfo.stSrcChnPort.u32DevId  = m_stVencAttr.stVencInBindParam.stChnPort.u32DevId;
    stBindInfo.stSrcChnPort.u32ChnId  = m_stVencAttr.stVencInBindParam.stChnPort.u32ChnId;
    stBindInfo.stSrcChnPort.u32PortId = m_stVencAttr.stVencInBindParam.stChnPort.u32PortId;
    stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId  = m_s32VencDev;
    stBindInfo.stDstChnPort.u32ChnId  = m_s32VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate          = m_stVencAttr.stVencInBindParam.u32SrcFrmrate;
    stBindInfo.u32DstFrmrate          = m_stVencAttr.stVencInBindParam.u32DstFrmrate;
    stBindInfo.eBindType              = m_stVencAttr.stVencInBindParam.eBindType;
    stBindInfo.u32BindParam           = m_stVencAttr.stVencInBindParam.u32BindParam;

    if (bBind)
    {
        CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
        m_bBindEncoder = TRUE;
    }
    else
    {
        CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
        m_bBindEncoder = FALSE;
    }

    return 0;
}

S32 MI_VideoEncoder::startVideoEncoder(BOOL bStartThread)
{
    if (m_bEncoding)
        return 0;

    if (m_stVencAttr.bPassVenc)
    {
#if (CARDV_RTSP_INPUT_ENABLE)
        CarDV_RtspClientStartEncoder(m_stVencAttr.u8ExtStreamChn, this);
#elif (CARDV_WS_INPUT_ENABLE)
        CarDV_WsClientStartEncoder(m_stVencAttr.u8ExtStreamChn, this);
#endif
        goto PASS_VENC_START;
    }

    if (VE_JPG == m_stVencAttr.u8EncoderType)
    {
        if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_FRAME_BASE)
        {
            // Wait IMI jpeg done
            WaitImiJpegDone();
        }
        else if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_REALTIME)
        {
            // Wait frame jpeg done
            WaitFrameJpegDone();
        }
    }
    else
    {
        MI_VENC_InputSourceConfig_t stInputSourceConfig;
        stInputSourceConfig.eInputSrcBufferMode = m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_HW_RING
                                                      ? E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA
                                                      : E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE;

        if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_HW_RING
            && m_stVencAttr.stVencInBindParam.stChnPort.eModId == E_MI_MODULE_ID_SCL)
        {
            CARDVCHECKRESULT_OS(MI_VENC_SetInputSourceConfig(m_s32VencDev, m_s32VencChn, &stInputSourceConfig));
        }
    }

    CARDVCHECKRESULT_OS(MI_VENC_StartRecvPic(m_s32VencDev, m_s32VencChn));

    if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_REALTIME
        && m_stVencAttr.stVencInBindParam.stChnPort.eModId == E_MI_MODULE_ID_SCL)
    {
        CARDVCHECKRESULT(VideoEnableInput(&m_stVencAttr.stVencInBindParam.stChnPort, TRUE));

        // unbind all frame jpeg channel.
        for (MI_U32 i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (g_videoEncoderArray[i] && g_videoEncoderArray[i]->getCodec() == VE_JPG
                && g_videoEncoderArray[i]->getInBindParam().eBindType == E_MI_SYS_BIND_TYPE_FRAME_BASE)
            {
                g_videoEncoderArray[i]->bindVideoEncoder(FALSE);
            }
        }
    }

    bindVideoEncoder(TRUE);

PASS_VENC_START:
    if (bStartThread)
    {
        startGetStreamThread();
    }

    m_bEncoding = TRUE;
    return 0;
}

S32 MI_VideoEncoder::stopVideoEncoder()
{
    if (!m_bEncoding)
        return 0;

    if (VE_JPG != m_stVencAttr.u8EncoderType)
    {
        // jpeg thread always exist, until deinit encoder.
        stopGetStreamThread();
    }

    if (m_stVencAttr.bPassVenc)
    {
#if (CARDV_RTSP_INPUT_ENABLE)
        CarDV_RtspClientStopEncoder(m_stVencAttr.u8ExtStreamChn, this);
#elif (CARDV_WS_INPUT_ENABLE)
        CarDV_WsClientStopEncoder(m_stVencAttr.u8ExtStreamChn, this);
#endif
        goto PASS_VENC_STOP;
    }

    if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_REALTIME
        && m_stVencAttr.stVencInBindParam.stChnPort.eModId == E_MI_MODULE_ID_SCL)
    {
        bindVideoEncoder(FALSE);
        CARDVCHECKRESULT(VideoEnableInput(&m_stVencAttr.stVencInBindParam.stChnPort, FALSE));
    }

    CARDVCHECKRESULT(CarDV_Venc_StopChannel(m_s32VencDev, m_s32VencChn));

    if (VE_JPG == m_stVencAttr.u8EncoderType)
    {
        if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_FRAME_BASE)
        {
            // Trigger frame jpeg done
            TriggerFrameJpegDone();
        }
        else if (m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_REALTIME)
        {
            // Trigger IMI jpeg done
            TriggerImiJpegDone();
        }
    }

PASS_VENC_STOP:
    m_bEncoding = FALSE;
    return 0;
}

MI_S32 MI_VideoEncoder::Query(MI_VENC_ChnStat_t *pstStat)
{
    if (!m_stVencAttr.bPassVenc)
    {
        return MI_VENC_Query(m_s32VencDev, m_s32VencChn, pstStat);
    }

#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
    pstStat->u32CurPacks = 1;
#endif
    return 0;
}

MI_S32 MI_VideoEncoder::GetStream(MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec)
{
    if (!m_stVencAttr.bPassVenc)
    {
        return MI_VENC_GetStream(m_s32VencDev, m_s32VencChn, pstStream, s32MilliSec);
    }
    else
    {
        MI_S32 s32Ret = 0;
#if (CARDV_RTSP_INPUT_ENABLE)
        CARDVCHECKRESULT(CarDV_RtspClientGetStream(m_stVencAttr.u8ExtStreamChn, m_s32VencFd, pstStream));
#elif (CARDV_WS_INPUT_ENABLE)
        CARDVCHECKRESULT(CarDV_WsClientGetStream(m_stVencAttr.u8ExtStreamChn, m_s32VencFd, pstStream));
#endif
        return s32Ret;
    }
}

MI_S32 MI_VideoEncoder::ReleaseStream(MI_VENC_Stream_t *pstStream)
{
    if (!m_stVencAttr.bPassVenc)
    {
        return MI_VENC_ReleaseStream(m_s32VencDev, m_s32VencChn, pstStream);
    }

    return 0;
}

MI_S32 MI_VideoEncoder::requestIDR(MI_BOOL bInstant)
{
    if (!m_stVencAttr.bPassVenc)
    {
        CARDVCHECKRESULT(MI_VENC_RequestIdr(m_s32VencDev, m_s32VencChn, bInstant));
        return 0;
    }

#if (CARDV_RTSP_INPUT_ENABLE)
    CARDVCHECKRESULT(CarDV_RtspClientRequestIDR(m_stVencAttr.u8ExtStreamChn, this));
#elif (CARDV_WS_INPUT_ENABLE)
    CARDVCHECKRESULT(CarDV_WsClientRequestIDR(m_stVencAttr.u8ExtStreamChn, this));
#endif
    return 0;
}

S32 MI_VideoEncoder::setResolution(MI_U32 u32Width, MI_U32 u32Height)
{
    MI_VENC_ChnAttr_t   stVencChnAttr;
    MI_SYS_WindowSize_t stPortSize;

    if (m_stVencAttr.bPassVenc)
    {
        goto PASS_VENC;
    }

    stPortSize.u16Width  = u32Width;
    stPortSize.u16Height = u32Height;

    if (E_MI_MODULE_ID_SCL == m_stVencAttr.stVencInBindParam.stChnPort.eModId
        && m_stVencAttr.stVencInBindParam.eBindType == E_MI_SYS_BIND_TYPE_REALTIME
        && g_s32VencChnCreated[MI_VENC_DEV_ID_H264_H265_0] > 0 && u32Width > IMI_JPEG_LIMIT_WIDTH)
    {
        // 8838G Sram size is 270KB. IMI JPEG Capture 4K width need 128KB, and H26X device need 120KB.
        stPortSize.u16Width  = IMI_JPEG_LIMIT_WIDTH;
        stPortSize.u16Height = IMI_JPEG_LIMIT_WIDTH * u32Height / u32Width;
        printf("IMI JPEG Exceed Sram Size, Chn%u Set [%dx%d] to [%dx%d]\n", m_s32VencChn, u32Width, u32Height,
               stPortSize.u16Width, stPortSize.u16Height);
    }

    CARDVCHECKRESULT(
        VideoSetInputResolution(&m_stVencAttr.stVencInBindParam.stChnPort, stPortSize.u16Width, stPortSize.u16Height));

    memset(&stVencChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    ExecFunc(MI_VENC_GetChnAttr(m_s32VencDev, m_s32VencChn, &stVencChnAttr), MI_SUCCESS);

    switch (m_stVencAttr.u8EncoderType)
    {
    case VE_H264:
        stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth  = ALIGN_8xUP(u32Width);
        stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = ALIGN_2xUP(u32Height);
        break;

    case VE_H265:
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = ALIGN_8xUP(u32Width);
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = ALIGN_2xUP(u32Height);
        break;

    case VE_JPG:
        stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth  = ALIGN_8xUP(u32Width);
        stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = ALIGN_2xUP(u32Height);
        break;

    default:
        CARDV_ERR("init video error, error video encoder type\n");
        break;
    }

    ExecFunc(MI_VENC_SetChnAttr(m_s32VencDev, m_s32VencChn, &stVencChnAttr), MI_SUCCESS);

PASS_VENC:
    m_stVencAttr.u32Width  = u32Width;
    m_stVencAttr.u32Height = u32Height;

    return 0;
}

S32 MI_VideoEncoder::setFrameRate(MI_U32 u32FrameRate, MI_U32 bChangeBitrate)
{
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_RcMode_e  eRcModeType         = E_MI_VENC_RC_MODE_MAX;
    MI_VENC_ModType_e enType              = E_MI_VENC_MODTYPE_VENC;
    MI_U32            u32VencOldFrameRate = m_stVencAttr.u32VencFrameRate;
    MI_U32            u32Num, u32Den, u32Gcd;

    if (m_stVencAttr.bPassVenc)
    {
        return 0;
    }

    m_stVencAttr.u32VencFrameRate = u32FrameRate;
    u32Num                        = m_stVencAttr.u32VencFrameRate & 0xffff;
    u32Den                        = (m_stVencAttr.u32VencFrameRate >> 16) & 0xffff;
    if (0x0 == u32Den)
        u32Den = 1;
    else
    {
        u32Gcd = __gcd(u32Num, u32Den);
        if (u32Gcd > 1)
        {
            u32Num /= u32Gcd;
            u32Den /= u32Gcd;
        }
    }

    if (m_stVencAttr.u32VencFrameRate != u32VencOldFrameRate && m_stVencAttr.u8EncoderType != VE_JPG)
    {
        MI_U32 u32OldNum, u32OldDen, u32OldGcd;

        bindVideoEncoder(FALSE);
        bindVideoEncoder(TRUE);

        u32OldNum = u32VencOldFrameRate & 0xffff;
        u32OldDen = (u32VencOldFrameRate >> 16) & 0xffff;
        if (0x0 == u32OldDen)
            u32OldDen = 1;
        else
        {
            u32OldGcd = __gcd(u32Num, u32OldDen);
            if (u32OldGcd > 1)
            {
                u32OldNum /= u32OldGcd;
                u32OldDen /= u32OldGcd;
            }
        }

        m_u64CheckIInterval = m_u64CheckIInterval * u32OldNum * u32Den / u32OldDen / u32Num;
    }

    if (m_stVencAttr.u8EncoderType == VE_H264)
    {
        MI_VENC_ParamH264Vui_t stH264Vui;
        ExecFunc(MI_VENC_GetH264Vui(m_s32VencDev, m_s32VencChn, &stH264Vui), MI_SUCCESS);
        stH264Vui.stVuiTimeInfo.u32NumUnitsInTick = 1000;
        stH264Vui.stVuiTimeInfo.u32TimeScale      = 2000 / u32Den * u32Num;
        ExecFunc(MI_VENC_SetH264Vui(m_s32VencDev, m_s32VencChn, &stH264Vui), MI_SUCCESS);
    }
    else if (m_stVencAttr.u8EncoderType == VE_H265)
    {
        MI_VENC_ParamH265Vui_t stH265Vui;
        ExecFunc(MI_VENC_GetH265Vui(m_s32VencDev, m_s32VencChn, &stH265Vui), MI_SUCCESS);
        stH265Vui.stVuiTimeInfo.u32NumUnitsInTick = 1000;
        stH265Vui.stVuiTimeInfo.u32TimeScale      = 1000 / u32Den * u32Num;
        ExecFunc(MI_VENC_SetH265Vui(m_s32VencDev, m_s32VencChn, &stH265Vui), MI_SUCCESS);
    }

    if (bChangeBitrate)
    {
        memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
        ExecFunc(MI_VENC_GetChnAttr(m_s32VencDev, m_s32VencChn, &stChnAttr), MI_SUCCESS);

        eRcModeType = stChnAttr.stRcAttr.eRcMode;
        enType      = stChnAttr.stVeAttr.eType;
        switch (enType)
        {
        case E_MI_VENC_MODTYPE_H264E:
            if (E_MI_VENC_RC_MODE_H264CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = u32Num;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = u32Den;
            }
            else if (E_MI_VENC_RC_MODE_H264VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = u32Num;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = u32Den;
            }
            else
            {
                CARDV_ERR("not init rcmode type, eRcModeType:%d\n", eRcModeType);
            }
            break;

        case E_MI_VENC_MODTYPE_H265E:
            if (E_MI_VENC_RC_MODE_H265CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = u32Num;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = u32Den;
            }
            else if (E_MI_VENC_RC_MODE_H265VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = u32Num;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = u32Den;
            }
            else
            {
                CARDV_ERR("not init rcmode type, eRcModeType:%d\n", eRcModeType);
            }
            break;

        case E_MI_VENC_MODTYPE_JPEGE:
            break;

        default:
            CARDV_ERR("not init video type, error video encoder type:%d\n", enType);
            break;
        }

        MI_U32 u32BitRate;
        MI_U32 u32TmpFrameRate = m_stVencAttr.u32VencFrameRate;
        float  fNewFrameRate   = 0.0;
        if (0 != (u32TmpFrameRate & 0xffff0000))
        {
            fNewFrameRate = (u32TmpFrameRate & 0xffff) * 1.0 / ((u32TmpFrameRate & 0xffff0000) >> 16);
        }
        else
        {
            fNewFrameRate = u32TmpFrameRate * 1.0;
        }

        if (E_MI_VENC_MODTYPE_H264E == enType)
        {
            if (E_MI_VENC_RC_MODE_H264CBR == eRcModeType)
            {
                u32BitRate = stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate;
                if (0 == (u32VencOldFrameRate & 0xffff0000))
                {
                    u32BitRate *= fNewFrameRate / u32VencOldFrameRate;
                }
                else
                {
                    u32BitRate *= fNewFrameRate
                                  / ((u32VencOldFrameRate & 0xffff) * 1.0 / ((u32VencOldFrameRate & 0xffff0000) >> 16));
                }
                stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = u32BitRate;
            }
            else if (E_MI_VENC_RC_MODE_H264VBR == eRcModeType)
            {
                u32BitRate = stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate;
                if (0 == (u32VencOldFrameRate & 0xffff0000))
                {
                    u32BitRate *= fNewFrameRate / u32VencOldFrameRate;
                }
                else
                {
                    u32BitRate *= fNewFrameRate
                                  / ((u32VencOldFrameRate & 0xffff) * 1.0 / ((u32VencOldFrameRate & 0xffff0000) >> 16));
                }
                stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = u32BitRate;
            }
            else
            {
                CARDV_ERR("not init rcmode type, eRcModeType:%d\n", eRcModeType);
                return 0;
            }
        }
        else if (E_MI_VENC_MODTYPE_H265E == enType)
        {
            if (E_MI_VENC_RC_MODE_H265CBR == eRcModeType)
            {
                u32BitRate = stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate;
                if (0 == (u32VencOldFrameRate & 0xffff0000))
                {
                    u32BitRate *= fNewFrameRate / u32VencOldFrameRate;
                }
                else
                {
                    u32BitRate *= fNewFrameRate
                                  / ((u32VencOldFrameRate & 0xffff) * 1.0 / ((u32VencOldFrameRate & 0xffff0000) >> 16));
                }
                stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32BitRate;
            }
            else if (E_MI_VENC_RC_MODE_H265VBR == eRcModeType)
            {
                u32BitRate = stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate;
                if (0 == (u32VencOldFrameRate & 0xffff0000))
                {
                    u32BitRate *= fNewFrameRate / u32VencOldFrameRate;
                }
                else
                {
                    u32BitRate *= fNewFrameRate
                                  / ((u32VencOldFrameRate & 0xffff) * 1.0 / ((u32VencOldFrameRate & 0xffff0000) >> 16));
                }
                stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate = u32BitRate;
            }
            else
            {
                CARDV_ERR("not init rcmode type, eRcModeType:%d\n", eRcModeType);
                return 0;
            }
        }
        else if (E_MI_VENC_MODTYPE_JPEGE == enType)
        {
            return 0;
        }
        else
        {
            CARDV_ERR("not init video type, enType:%d\n", enType);
            return 0;
        }

        ExecFunc(MI_VENC_SetChnAttr(m_s32VencDev, m_s32VencChn, &stChnAttr), MI_SUCCESS);
        m_stVencAttr.u32BitRate = u32BitRate;
    }

    return 0;
}

MI_S32 MI_VideoEncoder::setBitrate(U32 u32BitRate)
{
    MI_VENC_ChnAttr_t stChnAttr;

    if (m_stVencAttr.bPassVenc)
    {
        goto PASS_VENC;
    }

    ExecFunc(MI_VENC_GetChnAttr(m_s32VencDev, m_s32VencChn, &stChnAttr), MI_SUCCESS);

    switch (stChnAttr.stVeAttr.eType)
    {
    case E_MI_VENC_MODTYPE_H265E:
        if (stChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265CBR)
        {
            stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32BitRate;
        }
        else if (stChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265VBR)
        {
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate = u32BitRate;
        }
        else
        {
            CARDV_ERR("not support crtl = %d\n", stChnAttr.stRcAttr.eRcMode);
        }
        break;

    case E_MI_VENC_MODTYPE_H264E:
        if (stChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264CBR)
        {
            stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = u32BitRate;
        }
        else if (stChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264VBR)
        {
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = u32BitRate;
        }
        else
        {
            CARDV_ERR("not support crtl = %d\n", stChnAttr.stRcAttr.eRcMode);
        }
        break;

    case E_MI_VENC_MODTYPE_JPEGE:
        if (stChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_MJPEGCBR)
        {
            stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate = u32BitRate;
        }
        else
        {
            CARDV_ERR("not support crtl = %d\n", stChnAttr.stRcAttr.eRcMode);
        }
        break;

    default:
        break;
    }

    ExecFunc(MI_VENC_SetChnAttr(m_s32VencDev, m_s32VencChn, &stChnAttr), MI_SUCCESS);

PASS_VENC:
    m_stVencAttr.u32BitRate = u32BitRate;
    return 0;
}

MI_S32 MI_VideoEncoder::setGop(U32 u32Gop)
{
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_RcMode_e  eRcModeType = E_MI_VENC_RC_MODE_MAX;
    MI_VENC_ModType_e enType      = E_MI_VENC_MODTYPE_VENC;

    if (m_stVencAttr.bPassVenc)
    {
        return 0;
    }

    ExecFunc(MI_VENC_GetChnAttr(m_s32VencDev, m_s32VencChn, &stChnAttr), MI_SUCCESS);
    eRcModeType = stChnAttr.stRcAttr.eRcMode;
    enType      = stChnAttr.stVeAttr.eType;
    switch (enType)
    {
    case E_MI_VENC_MODTYPE_H264E:
        if (E_MI_VENC_RC_MODE_H264CBR == eRcModeType)
        {
            stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = u32Gop;
        }
        else if (E_MI_VENC_RC_MODE_H264VBR == eRcModeType)
        {
            stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = u32Gop;
        }
        else if (E_MI_VENC_RC_MODE_H264FIXQP == eRcModeType)
        {
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = u32Gop;
        }
        break;

    case E_MI_VENC_MODTYPE_H265E:
        if (E_MI_VENC_RC_MODE_H265CBR == eRcModeType)
        {
            stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = u32Gop;
        }
        else if (E_MI_VENC_RC_MODE_H265VBR == eRcModeType)
        {
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = u32Gop;
        }
        else if (E_MI_VENC_RC_MODE_H265FIXQP == eRcModeType)
        {
            stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop = u32Gop;
        }
        break;

    default:
        return 0;
    }

    ExecFunc(MI_VENC_SetChnAttr(m_s32VencDev, m_s32VencChn, &stChnAttr), MI_SUCCESS);
    return 0;
}

S32 MI_VideoEncoder::SetRoiCfg(MI_VENC_RoiCfg_t *pstRoiCfg)
{
    MI_VENC_RoiCfg_t stvencRoiCfg;
    MI_U32           index     = -1;
    MI_U32           u32Left   = 0;
    MI_U32           u32Top    = 0;
    MI_U32           u32Width  = 0;
    MI_U32           u32Height = 0;

    if (m_stVencAttr.bPassVenc)
    {
        return 0;
    }

    if (VE_H264 == m_stVencAttr.u8EncoderType)
    {
        u32Top    = ALIGN_16xUP(pstRoiCfg->stRect.u32Top);
        u32Left   = ALIGN_16xUP(pstRoiCfg->stRect.u32Left);
        u32Width  = ALIGN_16xUP(pstRoiCfg->stRect.u32Width);
        u32Height = ALIGN_16xUP(pstRoiCfg->stRect.u32Height);
    }
    else if (VE_H265 == m_stVencAttr.u8EncoderType)
    {
        u32Top    = ALIGN_32xUP(pstRoiCfg->stRect.u32Top);
        u32Left   = ALIGN_32xUP(pstRoiCfg->stRect.u32Left);
        u32Width  = ALIGN_32xUP(pstRoiCfg->stRect.u32Width);
        u32Height = ALIGN_32xUP(pstRoiCfg->stRect.u32Height);
    }
    else
    {
        return -1;
    }

    CarDV_API_ISVALID_POINT(pstRoiCfg);
    memset(&stvencRoiCfg, 0, sizeof(MI_VENC_RoiCfg_t));
    index = pstRoiCfg->u32Index;

    ExecFunc(MI_VENC_GetRoiCfg(m_s32VencDev, m_s32VencChn, index, &stvencRoiCfg), MI_SUCCESS);

    stvencRoiCfg.bEnable          = pstRoiCfg->bEnable;
    stvencRoiCfg.bAbsQp           = pstRoiCfg->bAbsQp;
    stvencRoiCfg.s32Qp            = pstRoiCfg->s32Qp;
    stvencRoiCfg.stRect.u32Left   = u32Left;
    stvencRoiCfg.stRect.u32Top    = u32Top;
    stvencRoiCfg.stRect.u32Width  = u32Width;
    stvencRoiCfg.stRect.u32Height = u32Height;
    stvencRoiCfg.u32Index         = index;
    ExecFunc(MI_VENC_SetRoiCfg(m_s32VencDev, m_s32VencChn, &stvencRoiCfg), MI_SUCCESS);

    return MI_SUCCESS;
}