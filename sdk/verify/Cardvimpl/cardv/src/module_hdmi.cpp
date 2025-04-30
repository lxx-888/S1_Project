/*
 * module_hdmi.cpp - Sigmastar
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

#include "mid_common.h"
#include "module_config.h"
#include "module_hdmi.h"

#if (CARDV_HDMI_ENABLE)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

MI_BOOL bHdmiStart = FALSE;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static MI_S32 Hdmi_callback(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
    case E_MI_HDMI_EVENT_HOTPLUG:
        printf("HDMI In\n");
        MI_HDMI_Start(eHdmi);
        bHdmiStart = TRUE;
        break;
    case E_MI_HDMI_EVENT_NO_PLUG:
        printf("HDMI Out\n");
        MI_HDMI_Stop(eHdmi);
        bHdmiStart = FALSE;
        break;
    default:
        printf("Unsupport event.\n");
        break;
    }
    return MI_SUCCESS;
}

MI_S32 CarDV_HdmiModuleSwitchTiming(MI_HDMI_TimingType_e eHdmiTiming)
{
    MI_S32         s32Ret = MI_SUCCESS;
    MI_HDMI_Attr_t stAttr;
    if (bHdmiStart)
    {
        CARDVCHECKRESULT(MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, TRUE));
        CARDVCHECKRESULT(MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr));

        stAttr.stVideoAttr.eTimingType = eHdmiTiming;

        CARDVCHECKRESULT(MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr));
        CARDVCHECKRESULT(MI_HDMI_SetAvMute(E_MI_HDMI_ID_0, FALSE));
    }
    return s32Ret;
}

MI_S32 CarDV_HdmiModuleInit(MI_HDMI_TimingType_e eHdmiTiming)
{
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t      stAttr;
    memset(&stInitParam, 0, sizeof(MI_HDMI_InitParam_t));
    stInitParam.pCallBackArgs        = NULL;
    stInitParam.pfnHdmiEventCallback = Hdmi_callback;
    CARDVCHECKRESULT(MI_HDMI_InitDev(&stInitParam));
    CARDVCHECKRESULT(MI_HDMI_Open(E_MI_HDMI_ID_0));

    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    stAttr.stEnInfoFrame.bEnableAudInfoFrame = FALSE;
    stAttr.stEnInfoFrame.bEnableAviInfoFrame = FALSE;
    stAttr.stEnInfoFrame.bEnableSpdInfoFrame = FALSE;
    stAttr.stVideoAttr.bEnableVideo          = TRUE;
    stAttr.stVideoAttr.eColorType            = E_MI_HDMI_COLOR_TYPE_YCBCR444;
    stAttr.stVideoAttr.eDeepColorMode        = E_MI_HDMI_DEEP_COLOR_MAX;
    stAttr.stVideoAttr.eTimingType           = eHdmiTiming;
    stAttr.stVideoAttr.eOutputMode           = E_MI_HDMI_OUTPUT_MODE_HDMI;
#if (CARDV_HDMI_AUDIO_ENABLE)
    stAttr.stAudioAttr.bEnableAudio    = TRUE;
    stAttr.stAudioAttr.bIsMultiChannel = 0;
    stAttr.stAudioAttr.eBitDepth       = E_MI_HDMI_BIT_DEPTH_16;
    stAttr.stAudioAttr.eCodeType       = E_MI_HDMI_ACODE_PCM;
    stAttr.stAudioAttr.eSampleRate     = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
#endif
    CARDVCHECKRESULT(MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr));
    CARDVCHECKRESULT(MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr));
    if (stAttr.bConnect)
    {
        CARDVCHECKRESULT(MI_HDMI_Start(E_MI_HDMI_ID_0));
        bHdmiStart = TRUE;
    }
    return 0;
}

MI_S32 CarDV_HdmiModuleUnInit()
{
    if (bHdmiStart)
    {
        CARDVCHECKRESULT(MI_HDMI_Stop(E_MI_HDMI_ID_0));
        bHdmiStart = FALSE;
    }
    CARDVCHECKRESULT(MI_HDMI_Close(E_MI_HDMI_ID_0));
    CARDVCHECKRESULT(MI_HDMI_DeInitDev());
    return 0;
}
#endif