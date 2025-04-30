/*
 * module_display.h- Sigmastar
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "module_common.h"

#if (CARDV_DISPLAY_ENABLE)

#include "module_display.h"
#if (CARDV_PANEL_ENABLE)
#include "module_panel.h"
#endif
#include "module_hdmi.h"
#include "module_scl.h"
#include "mi_sys.h"
#include "iniparser.h"
#include "IPC_cardvInfo.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define DISP_TO_SYS_CHN_ID(layerid, portid)                      \
    ({                                                           \
        int chnid = 0;                                           \
        if ((layerid == 0 || layerid == 2) && portid <= 15)      \
        {                                                        \
            chnid = portid;                                      \
        }                                                        \
        else if ((layerid == 1 || layerid == 3) && portid == 0)  \
        {                                                        \
            chnid = 16;                                          \
        }                                                        \
        else                                                     \
        {                                                        \
            printf("invalid layer%d port%d\n", layerid, portid); \
            chnid = 0;                                           \
        }                                                        \
        chnid;                                                   \
    })

#define DISP_WBC_ID (0)

typedef struct stInterfaceArray_s
{
    char                desc[32];
    MI_DISP_Interface_e eIntfType;
} stInterfaceArray_t;

typedef struct stTimingArray_s
{
    char                   desc[32];
    MI_DISP_OutputTiming_e eIntfSync;
    MI_HDMI_TimingType_e   eHdmiTiming;
} stTimingArray_t;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

stInterfaceArray_t astInterfaceArray[] = {
    {"MIPI", E_MI_DISP_INTF_MIPIDSI}, {"TTL_SPI", E_MI_DISP_INTF_TTL_SPI_IF},
    {"TTL", E_MI_DISP_INTF_TTL},      {"MCU_NOFLM", E_MI_DISP_INTF_MCU_NOFLM},
    {"MCU", E_MI_DISP_INTF_MCU},      {"SRGB", E_MI_DISP_INTF_SRGB},
    {"HDMI", E_MI_DISP_INTF_HDMI},    {"VGA", E_MI_DISP_INTF_VGA},
    {"CVBS", E_MI_DISP_INTF_CVBS},
};

stTimingArray_t astTimingArray[] = {
    {"user", E_MI_DISP_OUTPUT_USER, E_MI_HDMI_TIMING_MAX},
    {"PAL", E_MI_DISP_OUTPUT_PAL, E_MI_HDMI_TIMING_576_50P},
    {"NTSC", E_MI_DISP_OUTPUT_NTSC, E_MI_HDMI_TIMING_480_60P},
    {"480p60", E_MI_DISP_OUTPUT_480P60, E_MI_HDMI_TIMING_480_60P},
    {"576p50", E_MI_DISP_OUTPUT_576P50, E_MI_HDMI_TIMING_576_50P},
    {"720p2997", E_MI_DISP_OUTPUT_1280x720_2997, E_MI_HDMI_TIMING_1280x720_2997P},
    {"720p50", E_MI_DISP_OUTPUT_720P50, E_MI_HDMI_TIMING_720_50P},
    {"720p5994", E_MI_DISP_OUTPUT_1280x720_5994, E_MI_HDMI_TIMING_1280x720_5994P},
    {"720p60", E_MI_DISP_OUTPUT_720P60, E_MI_HDMI_TIMING_720_60P},
    {"1024x768_60", E_MI_DISP_OUTPUT_1024x768_60, E_MI_HDMI_TIMING_1024x768_60P},
    {"1080p24", E_MI_DISP_OUTPUT_1080P24, E_MI_HDMI_TIMING_1080_24P},
    {"1080p25", E_MI_DISP_OUTPUT_1080P25, E_MI_HDMI_TIMING_1080_25P},
    {"1080p2997", E_MI_DISP_OUTPUT_1920x1080_2997, E_MI_HDMI_TIMING_1920x1080_2997P},
    {"1080p30", E_MI_DISP_OUTPUT_1080P30, E_MI_HDMI_TIMING_1080_30P},
    {"1080p50", E_MI_DISP_OUTPUT_1080P50, E_MI_HDMI_TIMING_1080_50P},
    {"1080p5994", E_MI_DISP_OUTPUT_1920x1080_5994, E_MI_HDMI_TIMING_1920x1080_5994P},
    {"1080p60", E_MI_DISP_OUTPUT_1080P60, E_MI_HDMI_TIMING_1080_60P},
    {"1280x800_60", E_MI_DISP_OUTPUT_1280x800_60, E_MI_HDMI_TIMING_1280x800_60P},
    {"1280x1024_60", E_MI_DISP_OUTPUT_1280x1024_60, E_MI_HDMI_TIMING_1280x1024_60P},
    {"1366x768_60", E_MI_DISP_OUTPUT_1366x768_60, E_MI_HDMI_TIMING_1366x768_60P},
    {"1440x900_60", E_MI_DISP_OUTPUT_1440x900_60, E_MI_HDMI_TIMING_1440x900_60P},
    {"1680x1050_60", E_MI_DISP_OUTPUT_1680x1050_60, E_MI_HDMI_TIMING_1680x1050_60P},
    {"1600x1200_60", E_MI_DISP_OUTPUT_1600x1200_60, E_MI_HDMI_TIMING_1600x1200_60P},
    {"2560x1440_30", E_MI_DISP_OUTPUT_2560x1440_30, E_MI_HDMI_TIMING_1440_30P},
    {"3840x2160_30", E_MI_DISP_OUTPUT_3840x2160_30, E_MI_HDMI_TIMING_4K2K_30P},
    {"3840x2160_60", E_MI_DISP_OUTPUT_3840x2160_60, E_MI_HDMI_TIMING_4K2K_60P},
};

CarDV_DispModAttr_t  gstDispModule   = {0};
CarDV_DispBindAttr_t gstDispBindAttr = {0};

static MI_U32 u32SwitchIdx = 0;

extern "C" {
extern void RotateYuv420sp90(MI_U8 *pu8YDst, MI_U8 *pu8YSrc, MI_U8 *pu8UVDst, MI_U8 *pu8UVSrc, MI_U16 u16OrigW,
                             MI_U16 u16OrigH);
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_DISP_Interface_e CarDV_DispModuleParserIntfType(char *str)
{
    MI_U32              i         = 0;
    MI_DISP_Interface_e eIntfType = E_MI_DISP_INTF_MAX;

    if (str)
    {
        for (i = 0; i < sizeof(astInterfaceArray) / sizeof(stInterfaceArray_t); i++)
        {
            if (!strcmp(str, astInterfaceArray[i].desc))
            {
                eIntfType = astInterfaceArray[i].eIntfType;
                break;
            }
        }
    }

    return eIntfType;
}

MI_DISP_OutputTiming_e CarDV_DispModuleParserIntfSync(char *str)
{
    MI_U32                 i         = 0;
    MI_DISP_OutputTiming_e eIntfSync = E_MI_DISP_OUTPUT_MAX;

    if (str)
    {
        for (i = 0; i < sizeof(astTimingArray) / sizeof(stTimingArray_t); i++)
        {
            if (!strcmp(str, astTimingArray[i].desc))
            {
                eIntfSync = astTimingArray[i].eIntfSync;
                break;
            }
        }
    }

    return eIntfSync;
}

#if (CARDV_HDMI_ENABLE)
MI_HDMI_SinkInfo_t   stSinkInfo;
MI_HDMI_TimingType_e CarDV_DispToHdmiTiming(MI_DISP_OutputTiming_e eIntfSync)
{
    MI_U32               i           = 0;
    MI_HDMI_TimingType_e eHdmiTiming = E_MI_HDMI_TIMING_MAX;

    for (i = 0; i < sizeof(astTimingArray) / sizeof(stTimingArray_t); i++)
    {
        if (eIntfSync == astTimingArray[i].eIntfSync)
        {
            eHdmiTiming = astTimingArray[i].eHdmiTiming;
            break;
        }
    }

    return eHdmiTiming;
}

MI_HDMI_TimingType_e CarDV_DispModuleParserHdmiTiming(char *str)
{
    MI_U32                 i           = 0;
    MI_HDMI_TimingType_e   eHdmiTiming = E_MI_HDMI_TIMING_MAX;
    MI_DISP_OutputTiming_e eIntfSync   = E_MI_DISP_OUTPUT_MAX;
    if (str)
    {
        for (i = 0; i < sizeof(astTimingArray) / sizeof(stTimingArray_t); i++)
        {
            if (!strcmp(str, astTimingArray[i].desc))
            {
                eHdmiTiming = astTimingArray[i].eHdmiTiming;
                eIntfSync   = astTimingArray[i].eIntfSync;
                break;
            }
        }
    }

    for (MI_S32 s32DispDev = 0; s32DispDev < CARDV_MAX_DISP_DEV_NUM; s32DispDev++)
    {
        CarDV_DispDevAttr_t *pstDispDevAttr = &gstDispModule.stDispDevAttr[s32DispDev];
        if (TRUE == pstDispDevAttr->bUsed && FALSE == pstDispDevAttr->bCreate)
        {
            if (pstDispDevAttr->eIntfType == E_MI_DISP_INTF_HDMI)
            {
                MI_HDMI_GetSinkInfo(E_MI_HDMI_ID_0, &stSinkInfo);
                if (TRUE == stSinkInfo.bConnected)
                {
                    if (stSinkInfo.abVideoFmtSupported[eIntfSync] == 1)
                        pstDispDevAttr->eIntfSync = eIntfSync;
                }
                MI_DISP_PubAttr_t stDispPubAttr;
                memcpy(&stDispPubAttr, pstDispDevAttr, sizeof(MI_DISP_PubAttr_t));
                MI_DISP_SetPubAttr(s32DispDev, &stDispPubAttr);
                MI_DISP_Enable(s32DispDev);
            }
        }
    }
    return eHdmiTiming;
}

#endif

MI_S32 CarDV_DispModuleInit(void)
{
    // CARDVCHECKRESULT_OS(MI_DISP_InitDev(NULL));

    for (MI_S32 s32DispDev = 0; s32DispDev < CARDV_MAX_DISP_DEV_NUM; s32DispDev++)
    {
        CarDV_DispDevAttr_t *pstDispDevAttr = &gstDispModule.stDispDevAttr[s32DispDev];
        if (TRUE == pstDispDevAttr->bUsed && FALSE == pstDispDevAttr->bCreate)
        {
#if (CARDV_HDMI_ENABLE)
            if (pstDispDevAttr->eIntfType == E_MI_DISP_INTF_HDMI)
            {
                MI_HDMI_GetSinkInfo(E_MI_HDMI_ID_0, &stSinkInfo);
                if (TRUE == stSinkInfo.bConnected)
                {
                    if (stSinkInfo.abVideoFmtSupported[E_MI_HDMI_TIMING_1080_60P] == 1)
                        pstDispDevAttr->eIntfSync = E_MI_DISP_OUTPUT_1080P60;
                    if (stSinkInfo.abVideoFmtSupported[E_MI_HDMI_TIMING_1080_30P] == 1)
                        pstDispDevAttr->eIntfSync = E_MI_DISP_OUTPUT_1080P30;
                }
            }
#endif
            MI_DISP_LAYER     s32Layer = 0;
            MI_DISP_INPUTPORT u32Port  = 0;
            MI_DISP_PubAttr_t stDispPubAttr;
            memset(&stDispPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
            stDispPubAttr.eIntfSync  = pstDispDevAttr->eIntfSync;
            stDispPubAttr.eIntfType  = pstDispDevAttr->eIntfType;
            stDispPubAttr.u32BgColor = YUYV_BLACK;
            CARDVCHECKRESULT_OS(MI_DISP_SetPubAttr(s32DispDev, &stDispPubAttr));
            CARDVCHECKRESULT_OS(MI_DISP_Enable(s32DispDev));
            pstDispDevAttr->bCreate = TRUE;

#if (CARDV_PANEL_ENABLE)
            if (pstDispDevAttr->eIntfType == E_MI_DISP_INTF_LCD || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_TTL
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_TTL_SPI_IF
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_MIPIDSI
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_MCU_NOFLM
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_MCU || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_SRGB)
            {
                CARDVCHECKRESULT(CarDV_PanelModuleInit(pstDispDevAttr->eIntfType));
            }
#endif
#if (CARDV_HDMI_ENABLE)
            if (pstDispDevAttr->eIntfType == E_MI_DISP_INTF_HDMI || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_VGA)
            {
                CARDVCHECKRESULT(CarDV_HdmiModuleInit(CarDV_DispToHdmiTiming(pstDispDevAttr->eIntfSync)));
            }
#endif

            CarDV_DispLayerAttr_t *pstDispLayerAttr = NULL;
            for (s32Layer = 0; s32Layer < CARDV_MAX_DISP_LAYER_NUM; s32Layer++)
            {
                pstDispLayerAttr = &pstDispDevAttr->stDispLayerAttr[s32Layer];
                if (TRUE == pstDispLayerAttr->bUsed && FALSE == pstDispLayerAttr->bCreate)
                {
                    MI_DISP_VideoLayerAttr_t stDispLayerAttr;
                    memset(&stDispLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
                    stDispLayerAttr.stVidLayerDispWin.u16X      = pstDispLayerAttr->stDispLayerWin.u16X;
                    stDispLayerAttr.stVidLayerDispWin.u16Y      = pstDispLayerAttr->stDispLayerWin.u16Y;
                    stDispLayerAttr.stVidLayerDispWin.u16Width  = pstDispLayerAttr->stDispLayerWin.u16Width;
                    stDispLayerAttr.stVidLayerDispWin.u16Height = pstDispLayerAttr->stDispLayerWin.u16Height;
                    stDispLayerAttr.stVidLayerSize.u16Width     = pstDispLayerAttr->u16Width;
                    stDispLayerAttr.stVidLayerSize.u16Height    = pstDispLayerAttr->u16Height;
                    CARDVCHECKRESULT_OS(MI_DISP_BindVideoLayer(pstDispLayerAttr->s32LayerId, s32DispDev));
                    CARDVCHECKRESULT_OS(MI_DISP_SetVideoLayerAttr(pstDispLayerAttr->s32LayerId, &stDispLayerAttr));
                    CARDVCHECKRESULT_OS(MI_DISP_EnableVideoLayer(pstDispLayerAttr->s32LayerId));
                    pstDispLayerAttr->bCreate = TRUE;

                    MI_U32 PortNum = (s32Layer == 0) ? CARDV_MAX_DISP_LAYER0_PORT_NUM : CARDV_MAX_DISP_LAYER1_PORT_NUM;
                    MI_DISP_InputPortAttr_t stDispInputPortAttr;
                    CarDV_DispPortAttr_t *  pstDispPortAttr = NULL;
                    for (u32Port = 0; u32Port < PortNum; u32Port++)
                    {
                        pstDispPortAttr = &pstDispLayerAttr->stDispPortAttr[u32Port];
                        if (TRUE == pstDispPortAttr->bUsed && FALSE == pstDispPortAttr->bCreate)
                        {
                            memset(&stDispInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
                            stDispInputPortAttr.u16SrcWidth         = pstDispPortAttr->u16SrcWidth;
                            stDispInputPortAttr.u16SrcHeight        = pstDispPortAttr->u16SrcHeight;
                            stDispInputPortAttr.stDispWin.u16X      = pstDispPortAttr->stDispPortWin.u16X;
                            stDispInputPortAttr.stDispWin.u16Y      = pstDispPortAttr->stDispPortWin.u16Y;
                            stDispInputPortAttr.stDispWin.u16Width  = pstDispPortAttr->stDispPortWin.u16Width;
                            stDispInputPortAttr.stDispWin.u16Height = pstDispPortAttr->stDispPortWin.u16Height;
                            CARDVCHECKRESULT_OS(
                                MI_DISP_SetInputPortAttr(pstDispLayerAttr->s32LayerId, u32Port, &stDispInputPortAttr));
                            pstDispPortAttr->bCreate = TRUE;
                            if (pstDispPortAttr->stDispZoomInWin.u16Width != 0
                                && pstDispPortAttr->stDispZoomInWin.u16Height != 0)
                            {
                                CARDVCHECKRESULT_OS(MI_DISP_SetZoomInWindow(pstDispLayerAttr->s32LayerId, u32Port,
                                                                            &pstDispPortAttr->stDispZoomInWin));
                            }
                        }
                    }
                }
            }
        }
    }

    // Wbc
    CarDV_DispWbcAttr_t *pstDispWbcAttr = &gstDispModule.stDispWbcAttr;
    if (TRUE == pstDispWbcAttr->bUsed && FALSE == pstDispWbcAttr->bCreate)
    {
        MI_DISP_WBC_Source_t stWbcSource;
        MI_DISP_WBC_Attr_t   stWbcAttr;

        stWbcSource.eSourceType = pstDispWbcAttr->eSourceType;
        stWbcSource.u32SourceId = pstDispWbcAttr->u32SourceId;
        CARDVCHECKRESULT_OS(MI_DISP_SetWBCSource(DISP_WBC_ID, &stWbcSource));

        stWbcAttr.stTargetSize.u32Width  = pstDispWbcAttr->u32Width;
        stWbcAttr.stTargetSize.u32Height = pstDispWbcAttr->u32Height;
        stWbcAttr.ePixFormat             = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        CARDVCHECKRESULT_OS(MI_DISP_SetWBCAttr(DISP_WBC_ID, &stWbcAttr));
        CARDVCHECKRESULT_OS(MI_DISP_EnableWBC(DISP_WBC_ID));
        pstDispWbcAttr->bCreate = TRUE;
    }

    CARDVCHECKRESULT(CarDV_DispModuleInput(TRUE));

    return 0;
}

MI_S32 CarDV_DispModuleUnInit(void)
{
    CARDVCHECKRESULT(CarDV_DispModuleInput(FALSE));

    // Wbc
    CarDV_DispWbcAttr_t *pstDispWbcAttr = &gstDispModule.stDispWbcAttr;
    if (TRUE == pstDispWbcAttr->bUsed && TRUE == pstDispWbcAttr->bCreate)
    {
        CARDVCHECKRESULT(MI_DISP_DisableWBC(DISP_WBC_ID));
        pstDispWbcAttr->bCreate = FALSE;
    }

    for (MI_S32 s32DispDev = CARDV_MAX_DISP_DEV_NUM - 1; s32DispDev >= 0; s32DispDev--)
    {
        CarDV_DispDevAttr_t *pstDispDevAttr = &gstDispModule.stDispDevAttr[s32DispDev];
        if (TRUE == pstDispDevAttr->bUsed && TRUE == pstDispDevAttr->bCreate)
        {
            MI_DISP_LAYER          s32Layer         = 0;
            MI_DISP_INPUTPORT      u32Port          = 0;
            CarDV_DispLayerAttr_t *pstDispLayerAttr = NULL;

            for (s32Layer = 0; s32Layer < CARDV_MAX_DISP_LAYER_NUM; s32Layer++)
            {
                pstDispLayerAttr = &pstDispDevAttr->stDispLayerAttr[s32Layer];
                if (TRUE == pstDispLayerAttr->bUsed && TRUE == pstDispLayerAttr->bCreate)
                {
                    MI_U32 u32PortNum =
                        (0 == s32Layer) ? CARDV_MAX_DISP_LAYER0_PORT_NUM : CARDV_MAX_DISP_LAYER1_PORT_NUM;
                    CarDV_DispPortAttr_t *pstDispPortAttr = NULL;
                    for (u32Port = 0; u32Port < u32PortNum; u32Port++)
                    {
                        pstDispPortAttr = &pstDispLayerAttr->stDispPortAttr[u32Port];
                        if (TRUE == pstDispPortAttr->bUsed && TRUE == pstDispPortAttr->bCreate)
                        {
                            pstDispPortAttr->bCreate = FALSE;
                        }
                    }
                    CARDVCHECKRESULT(MI_DISP_DisableVideoLayer(pstDispLayerAttr->s32LayerId));
                    CARDVCHECKRESULT(MI_DISP_UnBindVideoLayer(pstDispLayerAttr->s32LayerId, s32DispDev));
                    pstDispLayerAttr->bCreate = FALSE;
                }
            }

#if (CARDV_HDMI_ENABLE)
            if (pstDispDevAttr->eIntfType == E_MI_DISP_INTF_HDMI || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_VGA)
            {
                CARDVCHECKRESULT(CarDV_HdmiModuleUnInit());
            }
#endif
#if (CARDV_PANEL_ENABLE)
            if (pstDispDevAttr->eIntfType == E_MI_DISP_INTF_LCD || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_TTL
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_TTL_SPI_IF
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_MIPIDSI
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_MCU_NOFLM
                || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_MCU || pstDispDevAttr->eIntfType == E_MI_DISP_INTF_SRGB)
            {
                CARDVCHECKRESULT(CarDV_PanelModuleUnInit(pstDispDevAttr->eIntfType));
            }
#endif

            CARDVCHECKRESULT(MI_DISP_Disable(s32DispDev));
            pstDispDevAttr->bCreate = FALSE;
        }
    }

    // CARDVCHECKRESULT(MI_DISP_DeInitDev());
    return 0;
}

MI_S32 CarDV_DispModuleBind(MI_BOOL bBind)
{
    for (MI_S32 s32DispDev = 0; s32DispDev < CARDV_MAX_DISP_DEV_NUM; s32DispDev++)
    {
        CarDV_DispDevAttr_t *pstDispDevAttr = &gstDispModule.stDispDevAttr[s32DispDev];
        if (TRUE == pstDispDevAttr->bUsed && TRUE == pstDispDevAttr->bCreate)
        {
            CarDV_DispLayerAttr_t *pstDispLayerAttr = NULL;
            for (MI_S32 s32Layer = 0; s32Layer < CARDV_MAX_DISP_LAYER_NUM; s32Layer++)
            {
                pstDispLayerAttr = &pstDispDevAttr->stDispLayerAttr[s32Layer];
                if (TRUE == pstDispLayerAttr->bUsed && TRUE == pstDispLayerAttr->bCreate)
                {
                    MI_U32 PortNum = (s32Layer == 0) ? CARDV_MAX_DISP_LAYER0_PORT_NUM : CARDV_MAX_DISP_LAYER1_PORT_NUM;
                    CarDV_DispPortAttr_t *pstDispPortAttr = NULL;
                    for (MI_U32 u32Port = 0; u32Port < PortNum; u32Port++)
                    {
                        pstDispPortAttr = &pstDispLayerAttr->stDispPortAttr[u32Port];
                        if (TRUE == pstDispPortAttr->bUsed && TRUE == pstDispPortAttr->bCreate
                            && pstDispPortAttr->bInject == FALSE)
                        {
                            if (pstDispPortAttr->stDispInBindParam.stChnPort.eModId != E_MI_MODULE_ID_MAX)
                            {
                                CarDV_Sys_BindInfo_T stBindInfo;
                                memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
                                stBindInfo.stSrcChnPort.eModId = pstDispPortAttr->stDispInBindParam.stChnPort.eModId;
                                stBindInfo.stSrcChnPort.u32DevId =
                                    pstDispPortAttr->stDispInBindParam.stChnPort.u32DevId;
                                stBindInfo.stSrcChnPort.u32ChnId =
                                    pstDispPortAttr->stDispInBindParam.stChnPort.u32ChnId;
                                stBindInfo.stSrcChnPort.u32PortId =
                                    pstDispPortAttr->stDispInBindParam.stChnPort.u32PortId;
                                stBindInfo.stDstChnPort.eModId   = E_MI_MODULE_ID_DISP;
                                stBindInfo.stDstChnPort.u32DevId = s32DispDev;
                                stBindInfo.stDstChnPort.u32ChnId =
                                    DISP_TO_SYS_CHN_ID(pstDispLayerAttr->s32LayerId, u32Port);
                                stBindInfo.stDstChnPort.u32PortId = 0;
                                stBindInfo.u32SrcFrmrate          = pstDispPortAttr->stDispInBindParam.u32SrcFrmrate;
                                stBindInfo.u32DstFrmrate          = pstDispPortAttr->stDispInBindParam.u32DstFrmrate;
                                stBindInfo.eBindType              = pstDispPortAttr->stDispInBindParam.eBindType;
                                if (bBind && pstDispPortAttr->bBind == FALSE)
                                {
                                    CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
                                    pstDispPortAttr->bBind = TRUE;
                                }
                                else if (bBind == FALSE && pstDispPortAttr->bBind)
                                {
                                    CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
                                    pstDispPortAttr->bBind = FALSE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

static void *_DispInjectThread(void *args)
{
    CarDV_DispPortAttr_t *pstDispPortAttr = (CarDV_DispPortAttr_t *)args;
    MI_SYS_ChnPort_t *    pstSrcChnPort   = &pstDispPortAttr->stDispInBindParam.stChnPort;
    MI_SYS_ChnPort_t      stDstChnPort;
    MI_SYS_BufInfo_t      stOutputBufInfo, stInputBufInfo;
    MI_SYS_BufConf_t      stBufConf;
    MI_SYS_BUF_HANDLE     hOutputHandle, hInputHandle;
    MI_S32                s32Ret = 0;
    MI_S32                s32Fd  = -1;
    fd_set                read_fds;
    struct timeval        TimeoutVal;

    stDstChnPort.eModId    = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId  = 0;
    stDstChnPort.u32ChnId  = DISP_TO_SYS_CHN_ID(0, 0);
    stDstChnPort.u32PortId = 0;

    MI_SYS_GetFd(pstSrcChnPort, &s32Fd);

    while (!pstDispPortAttr->bThreadExit)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret <= 0)
        {
            continue;
        }

        if (FD_ISSET(s32Fd, &read_fds))
        {
            s32Ret = MI_SYS_ChnOutputPortGetBuf(pstSrcChnPort, &stOutputBufInfo, &hOutputHandle);
            if (MI_SUCCESS == s32Ret)
            {
                memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
                stBufConf.eBufType                  = E_MI_SYS_BUFDATA_FRAME;
                stBufConf.u64TargetPts              = 0;
                stBufConf.stFrameCfg.u16Width       = pstDispPortAttr->u16SrcWidth;
                stBufConf.stFrameCfg.u16Height      = pstDispPortAttr->u16SrcHeight;
                stBufConf.stFrameCfg.eFormat        = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

                if (MI_SUCCESS
                    == MI_SYS_ChnInputPortGetBuf(&stDstChnPort, &stBufConf, &stInputBufInfo, &hInputHandle, -1))
                {
                    stInputBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                    stInputBufInfo.stFrameData.eFieldType    = E_MI_SYS_FIELDTYPE_NONE;
                    stInputBufInfo.stFrameData.eTileMode     = E_MI_SYS_FRAME_TILE_MODE_NONE;

                    RotateYuv420sp90((MI_U8 *)stInputBufInfo.stFrameData.pVirAddr[0],
                                     (MI_U8 *)stOutputBufInfo.stFrameData.pVirAddr[0],
                                     (MI_U8 *)stInputBufInfo.stFrameData.pVirAddr[1],
                                     (MI_U8 *)stOutputBufInfo.stFrameData.pVirAddr[1], pstDispPortAttr->u16SrcHeight,
                                     pstDispPortAttr->u16SrcWidth);

                    MI_SYS_ChnInputPortPutBuf(hInputHandle, &stInputBufInfo, FALSE);
                }

                MI_SYS_ChnOutputPortPutBuf(hOutputHandle);
            }
        }
    }

    MI_SYS_CloseFd(s32Fd);
    return NULL;
}

MI_S32 CarDV_DispModuleInput(MI_BOOL bEnable)
{
    for (MI_S32 s32DispDev = 0; s32DispDev < CARDV_MAX_DISP_DEV_NUM; s32DispDev++)
    {
        CarDV_DispDevAttr_t *pstDispDevAttr = &gstDispModule.stDispDevAttr[s32DispDev];
        if (TRUE == pstDispDevAttr->bUsed && TRUE == pstDispDevAttr->bCreate)
        {
            for (MI_DISP_LAYER s32Layer = 0; s32Layer < CARDV_MAX_DISP_LAYER_NUM; s32Layer++)
            {
                CarDV_DispLayerAttr_t *pstDispLayerAttr = &pstDispDevAttr->stDispLayerAttr[s32Layer];
                if (TRUE == pstDispLayerAttr->bUsed && TRUE == pstDispLayerAttr->bCreate)
                {
                    MI_U32 u32PortNum =
                        (0 == s32Layer) ? CARDV_MAX_DISP_LAYER0_PORT_NUM : CARDV_MAX_DISP_LAYER1_PORT_NUM;
                    CarDV_DispPortAttr_t *pstDispPortAttr = NULL;
                    for (MI_DISP_INPUTPORT u32Port = 0; u32Port < u32PortNum; u32Port++)
                    {
                        pstDispPortAttr = &pstDispLayerAttr->stDispPortAttr[u32Port];
                        if (TRUE == pstDispPortAttr->bUsed && TRUE == pstDispPortAttr->bCreate)
                        {
                            if (bEnable)
                            {
                                CARDVCHECKRESULT_OS(MI_DISP_EnableInputPort(pstDispLayerAttr->s32LayerId, u32Port));

                                if (pstDispPortAttr->bInject)
                                {
                                    pstDispPortAttr->bThreadExit = FALSE;
                                    pthread_create(&pstDispPortAttr->pInjectThread, NULL, _DispInjectThread,
                                                   (void *)pstDispPortAttr);
                                    pthread_setname_np(pstDispPortAttr->pInjectThread, "disp_task");
                                }
                            }
                            else
                            {
                                if (pstDispPortAttr->pInjectThread)
                                {
                                    pstDispPortAttr->bThreadExit = TRUE;
                                    pthread_join(pstDispPortAttr->pInjectThread, NULL);
                                    pstDispPortAttr->pInjectThread = 0;
                                }

                                CARDVCHECKRESULT(MI_DISP_DisableInputPort(pstDispLayerAttr->s32LayerId, u32Port));
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

MI_S32 CarDV_DispPipeBind(MI_U32 u32BindType)
{
    CarDV_DispBindAttr_t *pstDispBindAttr = &gstDispBindAttr;
    MI_U32                u32BindMap;
    MI_U8                 u8BindInfo;

    if (u32BindType >= pstDispBindAttr->u32BindMapNum)
        u32BindType = 0;

    u32BindMap = pstDispBindAttr->u32BindMap[u32BindType];
    for (u8BindInfo = 0; u8BindInfo < CARDV_MAX_DISP_BIND_INFO && pstDispBindAttr->u32BindMapNum; u8BindInfo++)
    {
        if (1 << u8BindInfo & u32BindMap)
        {
            CarDV_Sys_BindInfo_T *pstBindInfo = &pstDispBindAttr->stBindInfo[u8BindInfo];
            CARDVCHECKRESULT(CarDV_Sys_Bind(pstBindInfo));
            pstDispBindAttr->bBind[u8BindInfo] = TRUE;
        }
    }

    return 0;
}

MI_S32 CarDV_DispPipeUnBind()
{
    CarDV_DispBindAttr_t *pstDispBindAttr = &gstDispBindAttr;
    MI_U8                 u8BindInfo;

    for (u8BindInfo = 0; u8BindInfo < CARDV_MAX_DISP_BIND_INFO && pstDispBindAttr->u32BindMapNum; u8BindInfo++)
    {
        if (pstDispBindAttr->bBind[u8BindInfo])
        {
            CarDV_Sys_BindInfo_T *pstBindInfo = &pstDispBindAttr->stBindInfo[u8BindInfo];
            CARDVCHECKRESULT(CarDV_Sys_UnBind(pstBindInfo));
            pstDispBindAttr->bBind[u8BindInfo] = FALSE;
        }
    }

    return 0;
}

MI_S32 CarDV_DispPlayBackPipeBind()
{
    CarDV_DispBindAttr_t *pstDispBindAttr = &gstDispBindAttr;
    MI_U8                 u8BindInfo;

    for (u8BindInfo = 0; u8BindInfo < pstDispBindAttr->u32BindPbMapNum; u8BindInfo++)
    {
        if (pstDispBindAttr->bPlayBackBind[u8BindInfo] == FALSE)
        {
            CarDV_Sys_BindInfo_T *pstBindInfo = &pstDispBindAttr->stPlayBackBindInfo[u8BindInfo];
            CARDVCHECKRESULT(CarDV_Sys_Bind(pstBindInfo));
            pstDispBindAttr->bPlayBackBind[u8BindInfo] = TRUE;
        }
    }

    return 0;
}

MI_S32 CarDV_DispPlayBackPipeUnBind()
{
    CarDV_DispBindAttr_t *pstDispBindAttr = &gstDispBindAttr;
    MI_U8                 u8BindInfo;

    for (u8BindInfo = 0; u8BindInfo < pstDispBindAttr->u32BindPbMapNum; u8BindInfo++)
    {
        if (pstDispBindAttr->bPlayBackBind[u8BindInfo])
        {
            CarDV_Sys_BindInfo_T *pstBindInfo = &pstDispBindAttr->stPlayBackBindInfo[u8BindInfo];
            CARDVCHECKRESULT(CarDV_Sys_UnBind(pstBindInfo));
            pstDispBindAttr->bPlayBackBind[u8BindInfo] = FALSE;
        }
    }

    return 0;
}

MI_S32 display_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    CarDV_DispBindAttr_t *pstDispBindAttr = &gstDispBindAttr;

    if (id == CMD_DISP_PREVIEW_START && paramLen)
    {
        u32SwitchIdx = *param;
        if (u32SwitchIdx >= pstDispBindAttr->u32BindMapNum)
            u32SwitchIdx = 0;
    }

    printf("disp cmd [%d] mode [%d]\n", id, u32SwitchIdx);

    switch (id)
    {
    case CMD_DISP_PREVIEW_START:
        CARDVCHECKRESULT(CarDV_DispPipeBind(u32SwitchIdx));
        break;

    case CMD_DISP_PREVIEW_STOP:
        CARDVCHECKRESULT(CarDV_DispPipeUnBind());
        break;

    case CMD_DISP_PLAYBACK_START:
        CARDVCHECKRESULT(CarDV_DispPlayBackPipeBind());
        break;

    case CMD_DISP_PLAYBACK_STOP:
        CARDVCHECKRESULT(CarDV_DispPlayBackPipeUnBind());
        break;

    case CMD_DISP_SET_LCD_LUMA: {
        MI_U32             u32Luma = param[0];
        MI_DISP_LcdParam_t stLcdParam;
        for (MI_S32 s32DispDev = 0; s32DispDev < /* CARDV_MAX_DISP_DEV_NUM */ 1; s32DispDev++)
        {
            CARDVCHECKRESULT(MI_DISP_GetLcdParam(s32DispDev, &stLcdParam));
            stLcdParam.stCsc.u32Luma = u32Luma;
            CARDVCHECKRESULT(MI_DISP_SetLcdParam(s32DispDev, &stLcdParam));
        }
    }
    break;

    case CMD_DISP_SET_LCD_CONTRAST: {
        MI_U32             u32Contrast = param[0];
        MI_DISP_LcdParam_t stLcdParam;
        for (MI_S32 s32DispDev = 0; s32DispDev < /* CARDV_MAX_DISP_DEV_NUM */ 1; s32DispDev++)
        {
            CARDVCHECKRESULT(MI_DISP_GetLcdParam(s32DispDev, &stLcdParam));
            stLcdParam.stCsc.u32Contrast = u32Contrast;
            CARDVCHECKRESULT(MI_DISP_SetLcdParam(s32DispDev, &stLcdParam));
        }
    }
    break;

    case CMD_DISP_SET_LCD_HUE: {
        MI_U32             u32Hue = param[0];
        MI_DISP_LcdParam_t stLcdParam;
        for (MI_S32 s32DispDev = 0; s32DispDev < /* CARDV_MAX_DISP_DEV_NUM */ 1; s32DispDev++)
        {
            CARDVCHECKRESULT(MI_DISP_GetLcdParam(s32DispDev, &stLcdParam));
            stLcdParam.stCsc.u32Hue = u32Hue;
            CARDVCHECKRESULT(MI_DISP_SetLcdParam(s32DispDev, &stLcdParam));
        }
    }
    break;

    case CMD_DISP_SET_LCD_SATURATION: {
        MI_U32             u32Saturation = param[0];
        MI_DISP_LcdParam_t stLcdParam;
        for (MI_S32 s32DispDev = 0; s32DispDev < /* CARDV_MAX_DISP_DEV_NUM */ 1; s32DispDev++)
        {
            CARDVCHECKRESULT(MI_DISP_GetLcdParam(s32DispDev, &stLcdParam));
            stLcdParam.stCsc.u32Saturation = u32Saturation;
            CARDVCHECKRESULT(MI_DISP_SetLcdParam(s32DispDev, &stLcdParam));
        }
    }
    break;

#if (CARDV_HDMI_ENABLE)
    case CMD_DISP_SET_HDMI_TIMING: {
        MI_HDMI_TimingType_e eHdmiTiming;
        char                 szHdmiTiming[64];
        memset(szHdmiTiming, 0x00, sizeof(szHdmiTiming));
        memcpy(szHdmiTiming, param, paramLen);
        eHdmiTiming = CarDV_DispModuleParserHdmiTiming(szHdmiTiming);
        CarDV_HdmiModuleSwitchTiming(eHdmiTiming);
    }
    break;
#endif
    default:
        break;
    }

    return 0;
}
#endif
