/*
 * drv_hdmitx_if.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#define _DRV_HDMITX_IF_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_hdmitx_os.h"
#include "mhal_common.h"
#include "hdmitx_debug.h"
#include "hal_hdmitx_chip.h"
#include "hal_hdmitx_st.h"
#include "hal_hdmitx_if.h"
#include "drv_hdmitx_ctx.h"
#include "mhal_hdmitx_datatype.h"
#include "drv_hdmitx_if.h"
#include "hal_hdmitx.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_HDMITX_IF_SPD_INFOFRAME_DATA_SIZE 25
#define DRV_HDMITX_IF_AVI_INFOFRAME_DATA_SIZE 13
#define DRV_HDMITX_IF_AUD_INFOFRAME_DATA_SIZE 7

#define DRV_HDMITX_CTX_LOCK_SEM(x)   DrvHdmitxOsObtainSemaphore(x)
#define DRV_HDMITX_CTX_UNLOCK_SEM(x) DrvHdmitxOsReleaseSemaphore(x)

//-------------------------------------------------------------------------------------------------
//  Enu
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_UNKNOWN,            ///< Unknown
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_DIGITAL_STB,        ///< Digital STB
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_DVD_PLAYER,         ///< DVD player
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_D_VHS,              ///< D-VHS
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_HDD_VIDEO_RECORDER, ///< HDD VideoRecorder
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_DVC,                ///< Digital Video Camera (DVC)
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_DSC,                ///< Digital Still Camera (DSC)
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_VIDEO_CD,           ///< Video CD
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_GAME,               ///< Game
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_PC_GENERAL,         ///< PC general
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_BULE_RAY_DISC,      ///< Blu-Ray Disc(BD)
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_SUPER_AUDIO_CD,     ///< Super Audio CD
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_HD_DVD,             ///< HD DVD
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_PMP,                ///< Portable Multimedia Player (PMP)
    E_DRV_HDMITX_IF_SPD_SOURCE_INFO_MAX,
} DrvHdmitxIfSpdSourceInfo_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
u32 g_u32HdmitxDbgLevel = 0;

u8 g_u8AudChStatus3[E_MHAL_HDMITX_AUDIO_SAMPLERATE_MAX] = {
    0x02, 0x03, 0x00, 0x02, 0x08, 0x0A, 0x0C, 0x0E,
};

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static HalHdmitxTimingResType_e _DrvHdmitxIfTransTimingResypeToHal(MhaHdmitxTimingResType_e enTiming)
{
    HalHdmitxTimingResType_e enHalTiming;

    enHalTiming = enTiming == E_MHAL_HDMITX_RES_1280X720P_24HZ      ? E_HAL_HDMITX_RES_720X480P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X720P_25HZ    ? E_HAL_HDMITX_RES_1280X720P_25HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X720P_2997HZ  ? E_HAL_HDMITX_RES_1280X720P_29D97HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X720P_30HZ    ? E_HAL_HDMITX_RES_1280X720P_30HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X720P_50HZ    ? E_HAL_HDMITX_RES_1280X720P_50HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X720P_5994HZ  ? E_HAL_HDMITX_RES_1280X720P_59D94HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X720P_60HZ    ? E_HAL_HDMITX_RES_1280X720P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_2398HZ ? E_HAL_HDMITX_RES_1920X1080P_23D98HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_24HZ   ? E_HAL_HDMITX_RES_1920X1080P_24HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_25HZ   ? E_HAL_HDMITX_RES_1920X1080P_25HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_2997HZ ? E_HAL_HDMITX_RES_1920X1080P_29D97HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_30HZ   ? E_HAL_HDMITX_RES_1920X1080P_30HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_50HZ   ? E_HAL_HDMITX_RES_1920X1080P_50HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_5994HZ ? E_HAL_HDMITX_RES_1920X1080P_59D94HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1080P_60HZ   ? E_HAL_HDMITX_RES_1920X1080P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_2560X1440P_30HZ   ? E_HAL_HDMITX_RES_2560X1440P_30HZ
                  : enTiming == E_MHAL_HDMITX_RES_2560X1440P_50HZ   ? E_HAL_HDMITX_RES_2560X1440P_50HZ
                  : enTiming == E_MHAL_HDMITX_RES_2560X1440P_60HZ   ? E_HAL_HDMITX_RES_2560X1440P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_3840X2160P_24HZ   ? E_HAL_HDMITX_RES_3840X2160P_24HZ
                  : enTiming == E_MHAL_HDMITX_RES_3840X2160P_25HZ   ? E_HAL_HDMITX_RES_3840X2160P_25HZ
                  : enTiming == E_MHAL_HDMITX_RES_3840X2160P_2997HZ ? E_HAL_HDMITX_RES_3840X2160P_29D97HZ
                  : enTiming == E_MHAL_HDMITX_RES_3840X2160P_30HZ   ? E_HAL_HDMITX_RES_3840X2160P_30HZ
                  : enTiming == E_MHAL_HDMITX_RES_3840X2160P_50HZ   ? E_HAL_HDMITX_RES_3840X2160P_50HZ
                  : enTiming == E_MHAL_HDMITX_RES_3840X2160P_60HZ   ? E_HAL_HDMITX_RES_3840X2160P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_640X480P_60HZ     ? E_HAL_HDMITX_RES_640X480P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_720X480P_60HZ     ? E_HAL_HDMITX_RES_720X480P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_720X576P_50HZ     ? E_HAL_HDMITX_RES_720X576P_50HZ
                  : enTiming == E_MHAL_HDMITX_RES_800X600P_60HZ     ? E_HAL_HDMITX_RES_800X600P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_848X480P_60HZ     ? E_HAL_HDMITX_RES_848X480P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1024X768P_60HZ    ? E_HAL_HDMITX_RES_1024X768P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X768P_60HZ    ? E_HAL_HDMITX_RES_1280X768P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X800P_60HZ    ? E_HAL_HDMITX_RES_1280X800P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X960P_60HZ    ? E_HAL_HDMITX_RES_1280X960P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1280X1024P_60HZ   ? E_HAL_HDMITX_RES_1280X1024P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1360X768P_60HZ    ? E_HAL_HDMITX_RES_1360X768P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1366X768P_60HZ    ? E_HAL_HDMITX_RES_1366X768P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1400X1050P_60HZ   ? E_HAL_HDMITX_RES_1400X1050P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1440X900P_60HZ    ? E_HAL_HDMITX_RES_1440X900P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1600X900P_60HZ    ? E_HAL_HDMITX_RES_1600X900P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1600X1200P_60HZ   ? E_HAL_HDMITX_RES_1600X1200P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1680X1050P_60HZ   ? E_HAL_HDMITX_RES_1680X1050P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1200P_60HZ   ? E_HAL_HDMITX_RES_1920X1200P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_1920X1440P_60HZ   ? E_HAL_HDMITX_RES_1920X1440P_60HZ
                  : enTiming == E_MHAL_HDMITX_RES_2560X1600P_60HZ   ? E_HAL_HDMITX_RES_2560X1600P_60HZ
                                                                    : E_HAL_HDMITX_RES_MAX;

    return enHalTiming;
}
static u8 _DrvHdmitxIfTransAviInfoFrame(MhalHdmitxAviInfoFrameConfig_t *pstAviCfg, u8 *pu8InfoData)
{
    u16 value;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d, %s A0=%d %s %d %s %s %s %s %s %s\n", __FUNCTION__, __LINE__,
               PARSING_DRV_AVI_INFO_SCAN_INFO(pstAviCfg->enScanInfo), pstAviCfg->A0Value,
               PARSING_DRV_COLOR_FMT(pstAviCfg->enColorType), pstAviCfg->bEnableAfdOverWrite,
               PARSING_DRV_AVI_INFO_AFD_RATIO(pstAviCfg->enAfdRatio),
               PARSING_DRV_AVI_INFO_ASPECTRATIO(pstAviCfg->enAspectRatio),
               PARSING_DRV_AVI_INFO_COLORIMETRY(pstAviCfg->enColorimetry),
               PARSING_DRV_AVI_INFO_EXT_COLORIMETRY(pstAviCfg->enExtColorimetry),
               PARSING_DRV_TIMING(pstAviCfg->enTimingType),
               PARSING_DRV_AVI_INFO_YCC_QUANT_RANE(pstAviCfg->enYccQuantRange));

    memset(pu8InfoData, 0, sizeof(u8) * DRV_HDMITX_IF_AVI_INFOFRAME_DATA_SIZE);

    /* total 13 bytes packet */
    // Y2, Y1, Y0
    value = pstAviCfg->enColorType == E_MHAL_HDMITX_COLOR_YUV422   ? (1 << 5)
            : pstAviCfg->enColorType == E_MHAL_HDMITX_COLOR_YUV444 ? (2 << 5)
            : pstAviCfg->enColorType == E_MHAL_HDMITX_COLOR_YUV420 ? (3 << 5)
                                                                   : (0 << 5);
    // A0 field
    value |= pstAviCfg->A0Value == 0x01 ? 0x10 : 0x00;
    // S1, S0 field
    value |= pstAviCfg->enScanInfo;
    pu8InfoData[0] = value & 0xFF;

    // C1, C0, M1, M0
    if (pstAviCfg->enTimingType >= E_MHAL_HDMITX_RES_720X480P_60HZ
        && pstAviCfg->enTimingType <= E_MHAL_HDMITX_RES_720X576P_50HZ)
    {
        value = 0x48;
        value |= pstAviCfg->enAspectRatio << 4;
    }
    else
    {
        value = 0xA8;
    }
    // R3, R2, R1, R0
    if (pstAviCfg->bEnableAfdOverWrite)
    {
        value |= pstAviCfg->enAfdRatio & 0x0F;
    }

    if (pstAviCfg->enColorimetry != E_MHAL_HDMITX_COLORIMETRY_MAX)
    {
        value = (value & 0x3F) | (((u8)pstAviCfg->enColorimetry & 0x03) << 6);
    }
    pu8InfoData[1] = value & 0XFF;

    // EC0, EC1, EC2
    value = pstAviCfg->enExtColorimetry;
    // BT2020 RGB & BT2020 YCbCr share same value 6; 7 is reserved;
    value          = (value > 6) ? 6 : value;
    pu8InfoData[2] = (value << 4) & 0x0070;

    if (pstAviCfg->enColorType == E_MHAL_HDMITX_COLOR_RGB444)
    {
        // Q1, Q0
        if (pstAviCfg->enYccQuantRange == E_MHAL_HDMITX_YCC_QUANTIZATION_LIMITED_RANGE)
        {
            value = 1;
        }
        else if (pstAviCfg->enYccQuantRange == E_MHAL_HDMITX_YCC_QUANTIZATION_FULL_RANGE)
        {
            value = 2;
        }
        else
        {
            value = 0;
        }
        pu8InfoData[2] |= (value << 2) & 0x0C;
        pu8InfoData[4] = 0x00 & 0xC0;
    }
    else
    {
        // YQ1, YQ0
        if (pstAviCfg->enYccQuantRange == E_MHAL_HDMITX_YCC_QUANTIZATION_LIMITED_RANGE)
        {
            value = 0;
        }
        else if (pstAviCfg->enYccQuantRange == E_MHAL_HDMITX_YCC_QUANTIZATION_FULL_RANGE)
        {
            value = 1;
        }
        else
        {
            value = 3;
        }
        pu8InfoData[2] |= 0x00 & 0x0C;
        pu8InfoData[4] = (value << 6) & 0xC0;
    }

    // VIC code: VIC code shoud +1 if aspect ration is 16:9
    value = HalHdmitxMapVideoTimingToVIC(_DrvHdmitxIfTransTimingResypeToHal(pstAviCfg->enTimingType));
    if (((pstAviCfg->enTimingType >= E_MHAL_HDMITX_RES_720X480P_60HZ)
         && (pstAviCfg->enTimingType <= E_MHAL_HDMITX_RES_720X576P_50HZ))
        && (pstAviCfg->enAspectRatio == E_MHAL_HDMITX_ASPECT_RATIO_16TO9))
    {
        value += 1;
    }
    else if (pstAviCfg->enAspectRatio == E_MHAL_HDMITX_ASPECT_RATIO_21TO9)
    {
        u8 a8AR21To9MappingTbl[14][2] = {{60, 65}, {61, 66}, {62, 67}, {19, 68}, {4, 69},  {41, 70}, {47, 71},
                                         {32, 72}, {33, 73}, {34, 74}, {31, 75}, {16, 76}, {64, 77}, {63, 78}};
        if ((value >= 93) && (value <= 97)) // 3840*2160p@24 ~ 3840*2160@60
        {
            value += 10;
        }
        else if ((value > 78) && (value <= 92))
        {
            // do nothing;
        }
        else
        {
            u8 i         = 0;
            u8 bValidVIC = FALSE;

            for (i = 0; i < 14; i++)
            {
                if (a8AR21To9MappingTbl[i][0] == value)
                {
                    value     = a8AR21To9MappingTbl[i][1];
                    bValidVIC = TRUE;
                    break;
                }
            }

            if (!bValidVIC)
            {
                HDMITX_ERR("%s %d:: Invalid VIC Code for 21:9 Aspect Ratio!!!\r\n", __FUNCTION__, __LINE__);
            }
        }
    }
    pu8InfoData[3] = value & 0x7F;

    // check repetition
    if (0) // 480i and 576i need to set repetition
    {
        pu8InfoData[4] = 0x01 & 0x0F;
    }
    else
    {
        pu8InfoData[4] = 0x00 & 0x0F;
    }
    return 1;
}

static u8 _DrvHdmitxIfTransSpdInfoFrame(MhalHdmitxSpdInfoFrameConfig_t *pstSpdCfg, u8 *pu8InfoData)
{
    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d, Vendor:%s, Product:%s\n", __FUNCTION__, __LINE__,
               pstSpdCfg->au8VendorName, pstSpdCfg->au8ProductDescription);

    memset(pu8InfoData, 0, sizeof(u8) * DRV_HDMITX_IF_SPD_INFOFRAME_DATA_SIZE);
    memcpy(pu8InfoData, pstSpdCfg->au8VendorName, 8);
    memcpy(pu8InfoData + 8, pstSpdCfg->au8ProductDescription, 16);
    pu8InfoData[DRV_HDMITX_IF_SPD_INFOFRAME_DATA_SIZE - 1] = E_DRV_HDMITX_IF_SPD_SOURCE_INFO_DIGITAL_STB; // Default
    return 1;
}

static u8 _DrvHdmixIfTransAudioInfoFrame(MhalHdmitxAudInfoFrameConfig_t *pstAudCfg, u8 *pu8InfoData)
{
    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d, Ch_%d, %s %s\n", __FUNCTION__, __LINE__, pstAudCfg->u32ChannelCount,
               PARSING_DRV_AUD_INFO_CODE(pstAudCfg->enAudioCodeType),
               PARSING_DRV_AUD_INFO_SAMPLE(pstAudCfg->enSampleRate));

    memset(pu8InfoData, 0, sizeof(u8) * DRV_HDMITX_IF_AUD_INFOFRAME_DATA_SIZE);

    pu8InfoData[0] = pstAudCfg->enAudioCodeType == E_MHAL_HDMITX_AUDIO_CODE_PCM ? 0x00 : 0x01;
    pu8InfoData[1] = 0;
    pu8InfoData[2] = pstAudCfg->u32ChannelCount << 4;
    pu8InfoData[3] = g_u8AudChStatus3[pstAudCfg->enSampleRate];
    pu8InfoData[4] = 0x00;
    pu8InfoData[5] = 0x00;
    return 1;
}

static HalHdmitxColorType_e _DrvHdmitxIfTransColorTypeToHal(MhalHdmitxColorType_e enColor)
{
    HalHdmitxColorType_e enHalColor;

    enHalColor = enColor == E_MHAL_HDMITX_COLOR_AUTO     ? E_HAL_HDMITX_COLOR_AUTO
                 : enColor == E_MHAL_HDMITX_COLOR_YUV444 ? E_HAL_HDMITX_COLOR_YUV444
                 : enColor == E_MHAL_HDMITX_COLOR_YUV422 ? E_HAL_HDMITX_COLOR_YUV422
                 : enColor == E_MHAL_HDMITX_COLOR_YUV420 ? E_HAL_HDMITX_COLOR_YUV420
                                                         : E_HAL_HDMITX_COLOR_RGB444;
    return enHalColor;
}

static HalHdmitxOutpuModeType_e _DrvHdmitxIfTransOutpuModeToHal(MhalHdmitxOutpuModeType_e enOutputMode)
{
    HalHdmitxOutpuModeType_e enHalOutputMode;

    enHalOutputMode = enOutputMode == E_MHAL_HDMITX_OUTPUT_MODE_DVI         ? E_HAL_HDMITX_OUTPUT_MODE_DVI
                      : enOutputMode == E_MHAL_HDMITX_OUTPUT_MODE_DVI_HDCP  ? E_HAL_HDMITX_OUTPUT_MODE_DVI_HDCP
                      : enOutputMode == E_MHAL_HDMITX_OUTPUT_MODE_HDMI      ? E_HAL_HDMITX_OUTPUT_MODE_HDMI
                      : enOutputMode == E_MHAL_HDMITX_OUTPUT_MODE_HDMI_HDCP ? E_HAL_HDMITX_OUTPUT_MODE_HDMI_HDCP
                                                                            : E_HAL_HDMITX_OUTPUT_MODE_AUTO;

    return enHalOutputMode;
}

static HalHdmitxColorDepthType_e _DrvHdmitxIfTransColorDepthToHal(MhalHdmitxColorDepthType_e enColorDepth)
{
    HalHdmitxColorDepthType_e enHalColorDepth;

    enHalColorDepth = enColorDepth == E_MHAL_HDMITX_CD_24_BITS   ? E_HAL_HDMITX_CD_24_BITS
                      : enColorDepth == E_MHAL_HDMITX_CD_30_BITS ? E_HAL_HDMITX_CD_30_BITS
                      : enColorDepth == E_MHAL_HDMITX_CD_36_BITS ? E_HAL_HDMITX_CD_36_BITS
                      : enColorDepth == E_MHAL_HDMITX_CD_48_BITS ? E_HAL_HDMITX_CD_48_BITS
                                                                 : E_HAL_HDMITX_CD_NO_ID;
    return enHalColorDepth;
}

static HalHdmitxAudioFreqType_e _DrvHdmitxIfTransAudioFreqToHal(MhalHdmitxAudioFreqType_e enAudioFreq)
{
    HalHdmitxAudioFreqType_e enHalAudioFreq;

    enHalAudioFreq = enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_32K    ? E_HAL_HDMITX_AUDIO_FREQ_32K
                     : enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_44K  ? E_HAL_HDMITX_AUDIO_FREQ_44K
                     : enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_48K  ? E_HAL_HDMITX_AUDIO_FREQ_48K
                     : enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_88K  ? E_HAL_HDMITX_AUDIO_FREQ_88K
                     : enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_96K  ? E_HAL_HDMITX_AUDIO_FREQ_96K
                     : enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_176K ? E_HAL_HDMITX_AUDIO_FREQ_176K
                     : enAudioFreq == E_MHAL_HDMITX_AUDIO_FREQ_192K ? E_HAL_HDMITX_AUDIO_FREQ_192K
                                                                    : E_HAL_HDMITX_AUDIO_FREQ_NO_SIG;

    return enHalAudioFreq;
}

static HalHdmitxAudioChannelType_e _DrvHdmitxIfTransAudioChTohal(MhalHdmitxAudioChannelType_e enAudioCh)
{
    HalHdmitxAudioChannelType_e enHalAudioCh;

    enHalAudioCh = enAudioCh == E_MHAL_HDMITX_AUDIO_CH_2 ? E_HAL_HDMITX_AUDIO_CH_2 : E_HAL_HDMITX_AUDIO_CH_8;
    return enHalAudioCh;
}

static HalHdmitxAudioCodingType_e _DrvHdmitxIfTransAudioCodingToHal(MhalHdmitxAudioCodingType_e enAudioCoding)
{
    HalHdmitxAudioCodingType_e enHalAudioCoding;

    enHalAudioCoding = enAudioCoding == E_MHAL_HDMITX_AUDIO_CODING_PCM ? E_HAL_HDMITX_AUDIO_CODING_PCM
                                                                       : E_HAL_HDMITX_AUDIO_CODING_NONPCM;
    return enHalAudioCoding;
}

static HalHdmitxAudioSourceFormat_e _DrvHdmitxIfTransAudioSrcFmtToHal(MhalHdmitxAudioSourceFormat_e enAudioSrcFmt)
{
    HalHdmitxAudioSourceFormat_e enHalAudioSrcFmt;

    enHalAudioSrcFmt = enAudioSrcFmt == E_MHAL_HDMITX_AUDIO_FORMAT_PCM   ? E_HAL_HDMITX_AUDIO_FORMAT_PCM
                       : enAudioSrcFmt == E_MHAL_HDMITX_AUDIO_FORMAT_DSD ? E_HAL_HDMITX_AUDIO_FORMAT_DSD
                       : enAudioSrcFmt == E_MHAL_HDMITX_AUDIO_FORMAT_HBR ? E_HAL_HDMITX_AUDIO_FORMAT_HBR
                                                                         : E_HAL_HDMITX_AUDIO_FORMAT_NA;

    return enHalAudioSrcFmt;
}

static HalHdmitxSinkInfoType_e _DrvHdmitxIfTransSinkInfoTypeToHal(MhalHdmitxSinkInfoType_e enSinkType)
{
    HalHdmitxSinkInfoType_e enType;

    enType = enSinkType == E_MHAL_HDMITX_SINK_INFO_EDID_DATA      ? E_HAL_HDMITX_SINK_INFO_EDID_DATA
             : enSinkType == E_MHAL_HDMITX_SINK_INFO_HDMI_SUPPORT ? E_HAL_HDMITX_SINK_INFO_HDMI_SUPPORT
             : enSinkType == E_MHAL_HDMITX_SINK_INFO_COLOR_FORMAT ? E_HAL_HDMITX_SINK_INFO_COLOR_FORMAT
             : enSinkType == E_MHAL_HDMITX_SINK_INFO_HPD_STATUS   ? E_HAL_HDMITX_SINK_INFO_HPD_STATUS
                                                                  : E_HAL_HDMITX_SINK_INFO_MAX;

    return enType;
}

static HalHdmitxMuteType_e _DrvHdmitxIfTransMuteTypeToHal(MhalHdmitxMuteType_e enMute)
{
    HalHdmitxMuteType_e enHalMute = E_HAL_HDMITX_MUTE_NONE;

    if (enMute & E_MHAL_HDMITX_MUTE_AUDIO)
    {
        enHalMute |= E_HAL_HDMITX_MUTE_AUDIO;
    }

    if (enMute & E_MHAL_HDMITX_MUTE_VIDEO)
    {
        enHalMute |= E_HAL_HDMITX_MUTE_VIDEO;
    }

    if (enMute & E_MHAL_HDMITX_MUTE_AVMUTE)
    {
        enHalMute |= E_HAL_HDMITX_MUTE_AVMUTE;
    }

    return enHalMute;
}

static u8 _DrvHdmitxIfTransInfoFrameToHal(MhalHdmitxInfoFrameConfig_t *pstMhalCfg,
                                          HalHdmitxInfoFrameConfig_t * pstHalCfg)
{
    u8 bRet           = 1;
    pstHalCfg->enType = pstMhalCfg->enType == E_MHAL_HDMITX_INFOFRAM_TYPE_AVI     ? E_HAL_HDMITX_INFOFRAM_TYPE_AVI
                        : pstMhalCfg->enType == E_MHAL_HDMITX_INFOFRAM_TYPE_SPD   ? E_HAL_HDMITX_INFOFRAM_TYPE_SPD
                        : pstMhalCfg->enType == E_MHAL_HDMITX_INFOFRAM_TYPE_AUDIO ? E_HAL_HDMITX_INFOFRAM_TYPE_AUDIO
                                                                                  : E_HAL_HDMITX_INFOFRAM_TYPE_MAX;
    pstHalCfg->bEn    = pstMhalCfg->bEn;

    if (pstMhalCfg->enType == E_MHAL_HDMITX_INFOFRAM_TYPE_AVI)
    {
        pstHalCfg->u8DataLen = DRV_HDMITX_IF_AVI_INFOFRAME_DATA_SIZE;
        bRet = _DrvHdmitxIfTransAviInfoFrame(&pstMhalCfg->stInfoFrame.stAviInfoFrame, pstHalCfg->au8Data);
    }
    else if (pstMhalCfg->enType == E_MHAL_HDMITX_INFOFRAM_TYPE_SPD)
    {
        pstHalCfg->u8DataLen = DRV_HDMITX_IF_SPD_INFOFRAME_DATA_SIZE;
        bRet = _DrvHdmitxIfTransSpdInfoFrame(&pstMhalCfg->stInfoFrame.stSpdInfoFrame, pstHalCfg->au8Data);
    }
    else if (pstMhalCfg->enType == E_MHAL_HDMITX_INFOFRAM_TYPE_AUDIO)
    {
        pstHalCfg->u8DataLen = DRV_HDMITX_IF_AUD_INFOFRAME_DATA_SIZE;
        bRet = _DrvHdmixIfTransAudioInfoFrame(&pstMhalCfg->stInfoFrame.stAudInfoFrame, pstHalCfg->au8Data);
    }
    else
    {
        bRet = 0;
        HDMITX_ERR("%s %d, Unknown InforFrame Type %d\n", __FUNCTION__, __LINE__, pstMhalCfg->enType);
    }

    return bRet;
}

static u8 _DrvHdmitxIfSetClkOn(void *pCtx)
{
    u8                    bRet                            = 1;
    DrvHdmitxCtxConfig_t *pstHdmitxCtxCfg                 = NULL;
    u32                   au32ClkRate[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_RATE_SETTING;
    u8                    abClkEn[HAL_HDMITX_CLK_NUM]     = HAL_HDMITX_CLK_ON_SETTING;
    u32                   au32CurClkRate[HAL_HDMITX_CLK_NUM];
    u8                    abCurClkEn[HAL_HDMITX_CLK_NUM];

    pstHdmitxCtxCfg = (DrvHdmitxCtxConfig_t *)pCtx;

    DrvHdmitxIfGetClk(pCtx, abCurClkEn, au32CurClkRate, HAL_HDMITX_CLK_NUM);
    if (DrvHdmitxOsSetClkOn(abCurClkEn, au32ClkRate, HAL_HDMITX_CLK_NUM) != TRUE)
    {
        if (DrvHdmitxIfSetClk((void *)pstHdmitxCtxCfg, abClkEn, au32ClkRate, HAL_HDMITX_CLK_NUM)
            != E_MHAL_HDMITX_RET_SUCCESS)
        {
            bRet = 0;
            HDMITX_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
        }
    }

    return bRet;
}

static u8 _DrvHdmitxIfSetClkOff(void *pCtx)
{
    u8                    bRet                            = 1;
    DrvHdmitxCtxConfig_t *pstHdmitxCtxCfg                 = NULL;
    u32                   au32ClkRate[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_RATE_SETTING;
    u8                    abClkEn[HAL_HDMITX_CLK_NUM]     = HAL_HDMITX_CLK_OFF_SETTING;
    u32                   au32CurClkRate[HAL_HDMITX_CLK_NUM];
    u8                    abCurClkEn[HAL_HDMITX_CLK_NUM];

    pstHdmitxCtxCfg = (DrvHdmitxCtxConfig_t *)pCtx;

    DrvHdmitxIfGetClk(pCtx, abCurClkEn, au32CurClkRate, HAL_HDMITX_CLK_NUM);
    if (DrvHdmitxOsSetClkOff(abCurClkEn, au32ClkRate, HAL_HDMITX_CLK_NUM) != TRUE)
    {
        if (DrvHdmitxIfSetClk((void *)pstHdmitxCtxCfg, abClkEn, au32ClkRate, HAL_HDMITX_CLK_NUM)
            != E_MHAL_HDMITX_RET_SUCCESS)
        {
            bRet = 0;
            HDMITX_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
        }
    }

    return bRet;
}

static u8 _DrvHdmitxIfExecuteQuery(void *pCtx, HalHdmitxQueryConfig_t *pstQueryCfg)
{
    u8                    bRet            = 1;
    DrvHdmitxCtxConfig_t *pstHdmitxCtxCfg = pCtx;

    if (!pCtx)
    {
        return 0;
    }
    if (HalHdmitxIfQuery(pCtx, pstQueryCfg))
    {
        DRV_HDMITX_CTX_LOCK_SEM(&pstHdmitxCtxCfg->stSem);
        if (pstQueryCfg->stOutCfg.enQueryRet == E_HAL_HDMITX_QUERY_RET_OK
            || pstQueryCfg->stOutCfg.enQueryRet == E_HAL_HDMITX_QUERY_RET_NONEED)
        {
            if (pstQueryCfg->stOutCfg.pSetFunc)
            {
                pstQueryCfg->stOutCfg.pSetFunc(pCtx, pstQueryCfg->stInCfg.pInCfg);
            }
        }
        else
        {
            bRet = 0;
            HDMITX_ERR("%s %d, Query:%s, Ret:%s\n", __FUNCTION__, __LINE__,
                       PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType),
                       PARSING_HAL_QUERY_RET(pstQueryCfg->stOutCfg.enQueryRet));
        }
        DRV_HDMITX_CTX_UNLOCK_SEM(&pstHdmitxCtxCfg->stSem);
    }
    else
    {
        bRet = 0;
        HDMITX_ERR("%s %d, Query Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
MhalHdmitxRet_e DrvHdmitxIfSetClk(void *pCtx, u8 *pbEn, u32 *pu32ClkRate, u32 u32ClkNum)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;
    HalHdmitxClkConfig_t   stHalClkCfg;

    if (sizeof(stHalClkCfg.u32Rate) != sizeof(u32) * u32ClkNum || sizeof(stHalClkCfg.bEn) != sizeof(u8) * u32ClkNum)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Clk Num is not correct", __FUNCTION__, __LINE__);
    }
    else
    {
        HalHdmitxIfInit();

        memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_CLK_SET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxClkConfig_t);
        stQueryCfg.stInCfg.pInCfg      = &stHalClkCfg;

        stHalClkCfg.u32Num = u32ClkNum;
        memcpy(stHalClkCfg.u32Rate, pu32ClkRate, sizeof(u32) * u32ClkNum);
        memcpy(stHalClkCfg.bEn, pbEn, sizeof(u8) * u32ClkNum);

        if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
        {
            enRet = E_MHAL_HDMITX_RET_ERR;
            HDMITX_ERR("%s %d, Set Clk Fail\n", __FUNCTION__, __LINE__);
        }
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfGetClk(void *pCtx, u8 *pbEn, u32 *pu32ClkRate, u32 u32ClkNum)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;
    HalHdmitxClkConfig_t   stHalClkCfg;

    HalHdmitxIfInit();

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    memset(&stHalClkCfg, 0, sizeof(HalHdmitxClkConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_CLK_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxClkConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalClkCfg;

    stHalClkCfg.u32Num = u32ClkNum;

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg))
    {
        if (stHalClkCfg.u32Num == u32ClkNum)
        {
            memcpy(pu32ClkRate, stHalClkCfg.u32Rate, sizeof(stHalClkCfg.u32Rate));
            memcpy(pbEn, stHalClkCfg.bEn, sizeof(stHalClkCfg.bEn));
        }
        else
        {
            enRet = E_MHAL_HDMITX_RET_CFGERR;
        }
    }
    else
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfCreateInstance(void **pCtx, u32 u32HdmitxId)
{
    MhalHdmitxRet_e           enRet           = E_MHAL_HDMITX_RET_SUCCESS;
    DrvHdmitxCtxConfig_t *    pstHdmitxCtxCfg = NULL;
    DrvHdmitxCtxAllocConfig_t stHdmitxCtxAllcCfg;
    HalHdmitxQueryConfig_t    stQueryCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    DrvHdmitxCtxInit();
    HalHdmitxIfInit();

    stHdmitxCtxAllcCfg.s32HdmitxId = u32HdmitxId;

    if (DrvHdmitxCtxAllocate(&stHdmitxCtxAllcCfg, &pstHdmitxCtxCfg) == 0)
    {
        *pCtx = NULL;
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d: Allocate Ctx Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pCtx = (void *)pstHdmitxCtxCfg;
        if (_DrvHdmitxIfSetClkOn((void *)pstHdmitxCtxCfg))
        {
            memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
            stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_INIT;
            stQueryCfg.stInCfg.u32CfgSize  = 0;
            stQueryCfg.stInCfg.pInCfg      = NULL;
            if (_DrvHdmitxIfExecuteQuery((void *)pstHdmitxCtxCfg, &stQueryCfg) == 0)
            {
                enRet = E_MHAL_HDMITX_RET_ERR;
                HDMITX_ERR("%s %d:: Init Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            enRet = E_MHAL_HDMITX_RET_ERR;
            HDMITX_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
        }
    }

    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfGetInstance(void **pCtx, u32 u32Id)
{
    MhalHdmitxRet_e           enRet           = E_MHAL_HDMITX_RET_SUCCESS;
    DrvHdmitxCtxConfig_t *    pstHdmitxCtxCfg = NULL;
    DrvHdmitxCtxAllocConfig_t stHdmitxCtxAllcCfg;

    stHdmitxCtxAllcCfg.s32HdmitxId = u32Id;
    if (DrvHdmitxCtxGet(&stHdmitxCtxAllcCfg, &pstHdmitxCtxCfg))
    {
        *pCtx = (void *)pstHdmitxCtxCfg;
    }
    else
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        *pCtx = NULL;
        HDMITX_ERR("%s %d: Get Ctx Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfDestoryInstance(void *pCtx)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);
    if (pCtx)
    {
        memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_DEINIT;
        stQueryCfg.stInCfg.u32CfgSize  = 0;
        stQueryCfg.stInCfg.pInCfg      = NULL;
        if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg))
        {
            if (_DrvHdmitxIfSetClkOff(pCtx) == FALSE)
            {
                HDMITX_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
                enRet = E_MHAL_HDMITX_RET_ERR;
            }

            if (DrvHdmitxCtxFree((DrvHdmitxCtxConfig_t *)pCtx) == FALSE)
            {
                HDMITX_ERR("%s %d, Ctx Free Fail\n", __FUNCTION__, __LINE__);
                enRet = E_MHAL_HDMITX_RET_ERR;
            }
        }
        else
        {
            HDMITX_ERR("%s %d, destory instance Fail\n", __FUNCTION__, __LINE__);
            enRet = E_MHAL_HDMITX_RET_ERR;
        }
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetAttrBegin(void *pCtx)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_ATTR_BEGIN;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;
    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set Attr Begin Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetAttr(void *pCtx, MhalHdmitxAttrConfig_t *pstAttrCfg)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;
    HalHdmitxAttrConfig_t  stHalAttrCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_ATTR;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxAttrConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalAttrCfg;

    stHalAttrCfg.bVideoEn     = pstAttrCfg->bVideoEn;
    stHalAttrCfg.enInColor    = _DrvHdmitxIfTransColorTypeToHal(pstAttrCfg->enInColor);
    stHalAttrCfg.enOutColor   = _DrvHdmitxIfTransColorTypeToHal(pstAttrCfg->enOutColor);
    stHalAttrCfg.enColorDepth = _DrvHdmitxIfTransColorDepthToHal(pstAttrCfg->enColorDepth);
    stHalAttrCfg.enOutputMode = _DrvHdmitxIfTransOutpuModeToHal(pstAttrCfg->enOutputMode);
    stHalAttrCfg.enTiming     = _DrvHdmitxIfTransTimingResypeToHal(pstAttrCfg->enTiming);

    stHalAttrCfg.bAudioEn    = pstAttrCfg->bAudioEn;
    stHalAttrCfg.enAudioFreq = _DrvHdmitxIfTransAudioFreqToHal(pstAttrCfg->enAudioFreq);
    stHalAttrCfg.enAudioCh   = _DrvHdmitxIfTransAudioChTohal(pstAttrCfg->enAudioCh);
    stHalAttrCfg.enAudioCode = _DrvHdmitxIfTransAudioCodingToHal(pstAttrCfg->enAudioCode);
    stHalAttrCfg.enAudioFmt  = _DrvHdmitxIfTransAudioSrcFmtToHal(pstAttrCfg->enAudioFmt);

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set Attr Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetAttrEnd(void *pCtx)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_ATTR_END;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set Attr End Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetMute(void *pCtx, MhalHdmitxMuteConfig_t *pstMuteCfg)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t stQueryCfg;
    HalHdmitxMuteConfig_t  stHalMuteCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_MUTE;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxMuteConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalMuteCfg;

    stHalMuteCfg.bMute  = pstMuteCfg->bMute;
    stHalMuteCfg.enType = _DrvHdmitxIfTransMuteTypeToHal(pstMuteCfg->enType);

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set Mute Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetSignal(void *pCtx, MhalHdmitxSignalConfig_t *pstSignalCfg)
{
    MhalHdmitxRet_e         enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t  stQueryCfg;
    HalHdmitxSignalConfig_t stHalSignalCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_SIGNAL;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxSignalConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalSignalCfg;

    stHalSignalCfg.bEn = pstSignalCfg->bEn;

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set Signal Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfGetSinkInfo(void *pCtx, MhalHdmitxSinkInfoConfig_t *pstSinkInfoCfg)
{
    MhalHdmitxRet_e           enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t    stQueryCfg;
    HalHdmitxSinkInfoConfig_t stHalSinkInfoCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d, enType:%d \n", __FUNCTION__, __LINE__, pstSinkInfoCfg->enType);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_SINK_INFO;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxSinkInfoConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalSinkInfoCfg;

    memset(&stHalSinkInfoCfg, 0, sizeof(HalHdmitxSinkInfoConfig_t));

    stHalSinkInfoCfg.enType = _DrvHdmitxIfTransSinkInfoTypeToHal(pstSinkInfoCfg->enType);

    if (stHalSinkInfoCfg.enType == E_HAL_HDMITX_SINK_INFO_COLOR_FORMAT)
    {
        stHalSinkInfoCfg.stInfoUnit.stColoFmt.enTiming =
            _DrvHdmitxIfTransTimingResypeToHal(pstSinkInfoCfg->stInfoUnit.stColoFmt.enTiming);
    }
    else if (stHalSinkInfoCfg.enType == E_HAL_HDMITX_SINK_INFO_EDID_DATA)
    {
        stHalSinkInfoCfg.stInfoUnit.stEdidData.u8BlockId = pstSinkInfoCfg->stInfoUnit.stEdidData.u8BlockId;
    }

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Get Sink Info Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        switch (pstSinkInfoCfg->enType)
        {
            case E_MHAL_HDMITX_SINK_INFO_EDID_DATA:
                memcpy(pstSinkInfoCfg->stInfoUnit.stEdidData.au8EdidData,
                       stHalSinkInfoCfg.stInfoUnit.stEdidData.au8EdidData, sizeof(u8) * 128);
                break;

            case E_MHAL_HDMITX_SINK_INFO_HDMI_SUPPORT:
                pstSinkInfoCfg->stInfoUnit.stSupportedHdmi.bSupported =
                    stHalSinkInfoCfg.stInfoUnit.stSupportedHdmi.bSupported;
                break;

            case E_MHAL_HDMITX_SINK_INFO_COLOR_FORMAT:

                pstSinkInfoCfg->stInfoUnit.stColoFmt.enColor = 0;
                if (stHalSinkInfoCfg.stInfoUnit.stColoFmt.u32Color & (0x1 << E_HAL_HDMITX_COLOR_RGB444))
                {
                    pstSinkInfoCfg->stInfoUnit.stColoFmt.enColor |= E_MHAL_HDMITX_COLOR_RGB444;
                }

                if (stHalSinkInfoCfg.stInfoUnit.stColoFmt.u32Color & (0x1 << E_HAL_HDMITX_COLOR_YUV444))
                {
                    pstSinkInfoCfg->stInfoUnit.stColoFmt.enColor |= E_MHAL_HDMITX_COLOR_YUV444;
                }

                if (stHalSinkInfoCfg.stInfoUnit.stColoFmt.u32Color & (0x1 << E_HAL_HDMITX_COLOR_YUV422))
                {
                    pstSinkInfoCfg->stInfoUnit.stColoFmt.enColor |= E_MHAL_HDMITX_COLOR_YUV422;
                }

                if (stHalSinkInfoCfg.stInfoUnit.stColoFmt.u32Color & (0x1 << E_HAL_HDMITX_COLOR_YUV420))
                {
                    pstSinkInfoCfg->stInfoUnit.stColoFmt.enColor |= E_MHAL_HDMITX_COLOR_YUV420;
                }
                break;

            case E_MHAL_HDMITX_SINK_INFO_HPD_STATUS:
                pstSinkInfoCfg->stInfoUnit.stHpdStatus.bHpd = stHalSinkInfoCfg.stInfoUnit.stHpdStatus.bHpd;
                break;

            default:
                enRet = E_MHAL_HDMITX_RET_CFGERR;
                HDMITX_ERR("%s %d, Unknown Type: %d\n", __FUNCTION__, __LINE__, pstSinkInfoCfg->enType);
                break;
        }
    }
    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetInfoFrame(void *pCtx, MhalHdmitxInfoFrameConfig_t *pstInfoFrameCfg)
{
    MhalHdmitxRet_e            enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxQueryConfig_t     stQueryCfg;
    HalHdmitxInfoFrameConfig_t stInfoFrameCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d, enType:%d En:%d\n", __FUNCTION__, __LINE__, pstInfoFrameCfg->enType,
               pstInfoFrameCfg->bEn);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_INFO_FRAME;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxInfoFrameConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stInfoFrameCfg;

    memset(&stInfoFrameCfg, 0, sizeof(HalHdmitxInfoFrameConfig_t));
    if (_DrvHdmitxIfTransInfoFrameToHal(pstInfoFrameCfg, &stInfoFrameCfg))
    {
        if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
        {
            enRet = E_MHAL_HDMITX_RET_ERR;
            HDMITX_ERR("%s %d, Get Sink Info Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param Err \n", __FUNCTION__, __LINE__);
    }

    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetAnalogDrvCur(void *pCtx, MhalHdmitxAnaloDrvCurConfig_t *pstDrvCurCfg)
{
    MhalHdmitxRet_e              enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxAnaloDrvCurConfig_t stAnalogDrvCurCfg;
    HalHdmitxQueryConfig_t       stQueryCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_ANALOG_DRV_CUR;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxAnaloDrvCurConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stAnalogDrvCurCfg;

    stAnalogDrvCurCfg.u8DrvCurTap1Ch0 = pstDrvCurCfg->u8DrvCurTap1Ch0;
    stAnalogDrvCurCfg.u8DrvCurTap1Ch1 = pstDrvCurCfg->u8DrvCurTap1Ch1;
    stAnalogDrvCurCfg.u8DrvCurTap1Ch2 = pstDrvCurCfg->u8DrvCurTap1Ch2;
    stAnalogDrvCurCfg.u8DrvCurTap1Ch3 = pstDrvCurCfg->u8DrvCurTap1Ch3;

    stAnalogDrvCurCfg.u8DrvCurTap2Ch0 = pstDrvCurCfg->u8DrvCurTap2Ch0;
    stAnalogDrvCurCfg.u8DrvCurTap2Ch1 = pstDrvCurCfg->u8DrvCurTap2Ch1;
    stAnalogDrvCurCfg.u8DrvCurTap2Ch2 = pstDrvCurCfg->u8DrvCurTap2Ch2;
    stAnalogDrvCurCfg.u8DrvCurTap2Ch3 = pstDrvCurCfg->u8DrvCurTap2Ch3;

    stAnalogDrvCurCfg.u8DrvCurPdRtermCh     = pstDrvCurCfg->u8DrvCurPdRtermCh;
    stAnalogDrvCurCfg.u8DrvCurPdLdoPreDrvCh = pstDrvCurCfg->u8DrvCurPdLdoPreDrvCh;
    stAnalogDrvCurCfg.u8DrvCurPdLdoMuxCh    = pstDrvCurCfg->u8DrvCurPdLdoMuxCh;

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set AnalogDrvCur Fail\n", __FUNCTION__, __LINE__);
    }

    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetDebugLevel(void *pCtx, u32 u32DbgLevel)
{
    MhalHdmitxRet_e enRet = E_MHAL_HDMITX_RET_SUCCESS;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    HalHdmitxIfInit();
    g_u32HdmitxDbgLevel = u32DbgLevel;

    return enRet;
}

MhalHdmitxRet_e DrvHdmitxIfSetHpdConfig(void *pCtx, MhalHdmitxHpdConfig_t *pstHpdCfg)
{
    MhalHdmitxRet_e        enRet = E_MHAL_HDMITX_RET_SUCCESS;
    HalHdmitxHpdConfig_t   stHpdCfg;
    HalHdmitxQueryConfig_t stQueryCfg;

    HDMITX_DBG(HDMITX_DBG_LEVEL_DRV_IF, "%s %d\n", __FUNCTION__, __LINE__);

    memset(&stQueryCfg, 0, sizeof(HalHdmitxQueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_HDMITX_QUERY_HPD;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HalHdmitxHpdConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHpdCfg;

    stHpdCfg.u8GpioNum = pstHpdCfg->u8GpioNum;

    if (_DrvHdmitxIfExecuteQuery(pCtx, &stQueryCfg) == 0)
    {
        enRet = E_MHAL_HDMITX_RET_ERR;
        HDMITX_ERR("%s %d, Set HPD Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}
