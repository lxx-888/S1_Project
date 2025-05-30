/*
 * hdmitx_debug.h- Sigmastar
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

#ifndef _HDMITX_DEBUG_H_
#define _HDMITX_DEBUG_H_

//-----------------------------------------------------------------------------------------------------
// Variable Prototype
//-----------------------------------------------------------------------------------------------------
#ifndef _DRV_HDMITX_IF_C_
extern u32 g_u32HdmitxDbgLevel;
#endif

//-----------------------------------------------------------------------------------------------------
// Debug Level
//-----------------------------------------------------------------------------------------------------
#define HDMITX_DBG_LEVEL_RESERVED_0 0x00000001 // HDMITX_DBG
#define HDMITX_DBG_LEVEL_RESERVED_1 0x00000002 // Reserve
#define HDMITX_DBG_LEVEL_RESERVED_2 0x00000004 // HDMITX_DBG_UTILTX
#define HDMITX_DBG_LEVEL_RESERVED_3 0x00000008 // HDMITX_DBG_EDID
#define HDMITX_DBG_LEVEL_RESERVED_4 0x00000010 // HDMITX_DBG_HAL
#define HDMITX_DBG_LEVEL_HAL_UTI    0x00000020 // HDMITX_DBG_OS
#define HDMITX_DBG_LEVEL_HAL        0x00000040
#define HDMITX_DBG_LEVEL_HAL_HL     0x00000080
#define HDMITX_DBG_LEVEL_DRV_IF     0x00010000
#define HDMITX_DBG_LEVEL_HAL_IF     0x00020000
#define HDMITX_DBG_LEVEL_CTX        0x00040000
#define HDMITX_DBG_LEVEL_EDID       0x00080000
#define HDMITX_DBG_LEVEL_EDIDEXT    0x00100000
#define HDMITX_DBG_LEVEL_HFVSDB     0x00200000
#define HDMITX_DBG_LEVEL_HDMIVSDB   0x00400000
#define HDMITX_DBG_LEVEL_TASK       0x00800000
#ifdef HDMITX_DBG_DISABLE
#define HDMITX_DBG_EN 0
#else
#define HDMITX_DBG_EN 1
#endif

#ifdef HDMITX_OS_TYPE_UBOOT
#define PRINT_NONE
#define PRINT_RED
#define PRINT_YELLOW
#define PRINT_GREEN
#else
#define PRINT_NONE   "\33[m"
#define PRINT_RED    "\33[1;31m"
#define PRINT_YELLOW "\33[1;33m"
#define PRINT_GREEN  "\33[1;32m"
#endif
#ifndef KERN_WARNING
#define KERN_WARNING
#endif
#ifndef KERN_DEBUG
#define KERN_DEBUG
#endif

#if HDMITX_DBG_EN

#define HDMITX_DBG(dbglv, _fmt, _args...)          \
    do                                             \
    {                                              \
        if (g_u32HdmitxDbgLevel & dbglv)           \
        {                                          \
            CamOsPrintf(KERN_DEBUG _fmt, ##_args); \
        }                                          \
    } while (0)

#define HDMITX_ERR(_fmt, _args...)                       \
    do                                                   \
    {                                                    \
        CamOsPrintf(PRINT_RED _fmt PRINT_NONE, ##_args); \
    } while (0)
#define HDMITX_MSG(_fmt, _args...)                         \
    do                                                     \
    {                                                      \
        CamOsPrintf(PRINT_GREEN _fmt PRINT_NONE, ##_args); \
    } while (0)
#define HDMITX_DBG_VAL(val)
#else
#define HDMITX_DBG(dbglv, _fmt, _args...)

#define HDMITX_ERR(_fmt, _args...)                       \
    do                                                   \
    {                                                    \
        CamOsPrintf(PRINT_RED _fmt PRINT_NONE, ##_args); \
    } while (0)
#define HDMITX_MSG(_fmt, _args...)                         \
    do                                                     \
    {                                                      \
        CamOsPrintf(PRINT_GREEN _fmt PRINT_NONE, ##_args); \
    } while (0)
#define HDMITX_DBG_VAL(val) ((val) = (val))

#endif
//-----------------------------------------------------------------------------------------------------
// Parsing String
//-----------------------------------------------------------------------------------------------------
#define PARSING_HAL_QUERY_TYPE(x)                                \
    (x == E_HAL_HDMITX_QUERY_INIT             ? "INIT"           \
     : x == E_HAL_HDMITX_QUERY_DEINIT         ? "DEINIT"         \
     : x == E_HAL_HDMITX_QUERY_ATTR_BEGIN     ? "ATTR_BEGIN"     \
     : x == E_HAL_HDMITX_QUERY_ATTR           ? "ATTR"           \
     : x == E_HAL_HDMITX_QUERY_ATTR_END       ? "ATTR_END"       \
     : x == E_HAL_HDMITX_QUERY_SIGNAL         ? "SIGNAL"         \
     : x == E_HAL_HDMITX_QUERY_MUTE           ? "MUTE"           \
     : x == E_HAL_HDMITX_QUERY_ANALOG_DRV_CUR ? "ANALOG_DRV_CUR" \
     : x == E_HAL_HDMITX_QUERY_SINK_INFO      ? "SINK_INFO"      \
     : x == E_HAL_HDMITX_QUERY_INFO_FRAME     ? "INFO_FRAME"     \
     : x == E_HAL_HDMITX_QUERY_DEBUG_LEVEL    ? "DEBUG_LEVEL"    \
     : x == E_HAL_HDMITX_QUERY_HPD            ? "HPD"            \
     : x == E_HAL_HDMITX_QUERY_CLK_SET        ? "CLK_SET"        \
     : x == E_HAL_HDMITX_QUERY_CLK_GET        ? "CLK_GET"        \
                                              : "UNKNOWN")

#define PARSING_HAL_QUERY_RET(x)                            \
    (x == E_HAL_HDMITX_QUERY_RET_OK           ? "OK"        \
     : x == E_HAL_HDMITX_QUERY_RET_CFGERR     ? "CFGERR"    \
     : x == E_HAL_HDMITX_QUERY_RET_NOTSUPPORT ? "NOSUPPORT" \
     : x == E_HAL_HDMITX_QUERY_RET_NONEED     ? "NONEED"    \
     : x == E_HAL_HDMITX_QUERY_RET_ERR        ? "ERR"       \
                                              : "UNKNOWN")

#define PARSING_HAL_COLOR_FMT(x)                 \
    (x == E_HAL_HDMITX_COLOR_RGB444   ? "RGB444" \
     : x == E_HAL_HDMITX_COLOR_YUV444 ? "YUV444" \
     : x == E_HAL_HDMITX_COLOR_YUV422 ? "YUV422" \
     : x == E_HAL_HDMITX_COLOR_YUV420 ? "YUV420" \
     : x == E_HAL_HDMITX_COLOR_AUTO   ? "AUTO"   \
                                      : "UNKNOWN")

#define PARSING_HAL_OUTPUT_MODE(x)                           \
    (x == E_HAL_HDMITX_OUTPUT_MODE_DVI         ? "DVI"       \
     : x == E_HAL_HDMITX_OUTPUT_MODE_DVI_HDCP  ? "DVI_HDCP"  \
     : x == E_HAL_HDMITX_OUTPUT_MODE_HDMI      ? "HDMI"      \
     : x == E_HAL_HDMITX_OUTPUT_MODE_HDMI_HDCP ? "HDMI_HDCP" \
     : x == E_HAL_HDMITX_OUTPUT_MODE_AUTO      ? "AUTO"      \
                                               : "UNKNOWN")

#define PARSING_HAL_COLOR_DEPTH(x)              \
    (x == E_HAL_HDMITX_CD_NO_ID     ? "NO_ID"   \
     : x == E_HAL_HDMITX_CD_24_BITS ? "24 Bits" \
     : x == E_HAL_HDMITX_CD_30_BITS ? "30 Bits" \
     : x == E_HAL_HDMITX_CD_36_BITS ? "36 Bits" \
     : x == E_HAL_HDMITX_CD_48_BITS ? "48 Bits" \
                                    : "UNKNOWN")

#define PARSING_HAL_TIMING(x)                                       \
    (x == E_HAL_HDMITX_RES_1280X720P_24HZ       ? "1280x720p24"     \
     : x == E_HAL_HDMITX_RES_1280X720P_25HZ     ? "1280x720p25"     \
     : x == E_HAL_HDMITX_RES_1280X720P_29D97HZ  ? "1280x720p29.97"  \
     : x == E_HAL_HDMITX_RES_1280X720P_30HZ     ? "1280x720p30"     \
     : x == E_HAL_HDMITX_RES_1280X720P_50HZ     ? "1280x720p50"     \
     : x == E_HAL_HDMITX_RES_1280X720P_59D94HZ  ? "1280x720p59.94"  \
     : x == E_HAL_HDMITX_RES_1280X720P_60HZ     ? "1280x720p60"     \
     : x == E_HAL_HDMITX_RES_1920X1080P_23D98HZ ? "1920x1080p23.98" \
     : x == E_HAL_HDMITX_RES_1920X1080P_24HZ    ? "1920x1080p24"    \
     : x == E_HAL_HDMITX_RES_1920X1080P_25HZ    ? "1920x1080p25"    \
     : x == E_HAL_HDMITX_RES_1920X1080P_29D97HZ ? "1920x1080p29D97" \
     : x == E_HAL_HDMITX_RES_1920X1080P_30HZ    ? "1920x1080p30"    \
     : x == E_HAL_HDMITX_RES_1920X1080P_50HZ    ? "1920x1080p50"    \
     : x == E_HAL_HDMITX_RES_1920X1080P_59D94HZ ? "1920x1080p59D94" \
     : x == E_HAL_HDMITX_RES_1920X1080P_60HZ    ? "1920x1080p60"    \
     : x == E_HAL_HDMITX_RES_2560X1440P_30HZ    ? "2560x1440p30"    \
     : x == E_HAL_HDMITX_RES_2560X1440P_50HZ    ? "2560x1440p50"    \
     : x == E_HAL_HDMITX_RES_2560X1440P_60HZ    ? "2560x1440p60"    \
     : x == E_HAL_HDMITX_RES_3840X2160P_24HZ    ? "3840x2160p24"    \
     : x == E_HAL_HDMITX_RES_3840X2160P_25HZ    ? "3840x2160p25"    \
     : x == E_HAL_HDMITX_RES_3840X2160P_29D97HZ ? "3840x2160p29.97" \
     : x == E_HAL_HDMITX_RES_3840X2160P_30HZ    ? "3840x2160p30"    \
     : x == E_HAL_HDMITX_RES_3840X2160P_50HZ    ? "3840x2160p50"    \
     : x == E_HAL_HDMITX_RES_3840X2160P_60HZ    ? "3840x2160p60"    \
     : x == E_HAL_HDMITX_RES_640X480P_60HZ      ? "640x480p60"      \
     : x == E_HAL_HDMITX_RES_720X480P_60HZ      ? "720x480p60"      \
     : x == E_HAL_HDMITX_RES_720X576P_50HZ      ? "720x576p50"      \
     : x == E_HAL_HDMITX_RES_800X600P_60HZ      ? "800x600p60"      \
     : x == E_HAL_HDMITX_RES_848X480P_60HZ      ? "848x480p60"      \
     : x == E_HAL_HDMITX_RES_1024X768P_60HZ     ? "1024x768p60"     \
     : x == E_HAL_HDMITX_RES_1280X768P_60HZ     ? "1280x768p60"     \
     : x == E_HAL_HDMITX_RES_1280X800P_60HZ     ? "1280x800p60"     \
     : x == E_HAL_HDMITX_RES_1280X960P_60HZ     ? "1280x960p60"     \
     : x == E_HAL_HDMITX_RES_1280X1024P_60HZ    ? "1280x1024p60"    \
     : x == E_HAL_HDMITX_RES_1360X768P_60HZ     ? "1360x768p60"     \
     : x == E_HAL_HDMITX_RES_1366X768P_60HZ     ? "1366x768p60"     \
     : x == E_HAL_HDMITX_RES_1400X1050P_60HZ    ? "1400x1050p60"    \
     : x == E_HAL_HDMITX_RES_1440X900P_60HZ     ? "1440x900p60"     \
     : x == E_HAL_HDMITX_RES_1600X900P_60HZ     ? "1600x900p60"     \
     : x == E_HAL_HDMITX_RES_1600X1200P_60HZ    ? "1600x1200p60"    \
     : x == E_HAL_HDMITX_RES_1680X1050P_60HZ    ? "1680x1050p60"    \
     : x == E_HAL_HDMITX_RES_1920X1200P_60HZ    ? "1920x1200p60"    \
     : x == E_HAL_HDMITX_RES_1920X1440P_60HZ    ? "1920x1440p60"    \
     : x == E_HAL_HDMITX_RES_2560X1600P_60HZ    ? "2560x1600p60"    \
                                                : "UNKNOWN")

#define PARSING_HAL_AUDIO_FREQ(x)                   \
    (x == E_HAL_HDMITX_AUDIO_FREQ_NO_SIG ? "NO_SIG" \
     : x == E_HAL_HDMITX_AUDIO_FREQ_32K  ? "32K"    \
     : x == E_HAL_HDMITX_AUDIO_FREQ_44K  ? "44K"    \
     : x == E_HAL_HDMITX_AUDIO_FREQ_48K  ? "48K"    \
     : x == E_HAL_HDMITX_AUDIO_FREQ_88K  ? "88K"    \
     : x == E_HAL_HDMITX_AUDIO_FREQ_96K  ? "96K"    \
     : x == E_HAL_HDMITX_AUDIO_FREQ_176K ? "176K"   \
     : x == E_HAL_HDMITX_AUDIO_FREQ_192K ? "192K"   \
                                         : "UNKNOWN")

#define PARSING_HAL_AUDIO_CH(x) \
    (x == E_HAL_HDMITX_AUDIO_CH_2 ? "CH_2" : x == E_HAL_HDMITX_AUDIO_CH_8 ? "CH_8" : "UNKNOWN")

#define PARSING_HAL_AUDIO_CODING(x) \
    (x == E_HAL_HDMITX_AUDIO_CODING_PCM ? "PCM" : x == E_HAL_HDMITX_AUDIO_CODING_NONPCM ? "NONPCM" : "UNKNOWN")

#define PARSING_HAL_AUDIO_FMT(x)                  \
    (x == E_HAL_HDMITX_AUDIO_FORMAT_PCM   ? "PCM" \
     : x == E_HAL_HDMITX_AUDIO_FORMAT_DSD ? "DSD" \
     : x == E_HAL_HDMITX_AUDIO_FORMAT_HBR ? "HBR" \
     : x == E_HAL_HDMITX_AUDIO_FORMAT_NA  ? "NA"  \
                                          : "UNKNOWN")

#define PARSING_HAL_SINK_TYPE(x)                                 \
    (x == E_HAL_HDMITX_SINK_INFO_EDID_DATA      ? "EDID_DATA"    \
     : x == E_HAL_HDMITX_SINK_INFO_HPD_STATUS   ? "HPD_STATUS"   \
     : x == E_HAL_HDMITX_SINK_INFO_COLOR_FORMAT ? "COLOR_FORMAT" \
     : x == E_HAL_HDMITX_SINK_INFO_HDMI_SUPPORT ? "HDMI_SUPPORT" \
                                                : "UNKNOWN")

#define PARSING_HAL_INFOFRAME_TYPE(x)                  \
    (x == E_HAL_HDMITX_INFOFRAM_TYPE_AVI     ? "AVI"   \
     : x == E_HAL_HDMITX_INFOFRAM_TYPE_SPD   ? "AVI"   \
     : x == E_HAL_HDMITX_INFOFRAM_TYPE_AUDIO ? "AUDIO" \
                                             : "UNKNOWN")

#define PARSING_DRV_INFOFRAME_TYPE(x)                   \
    (x == E_MHAL_HDMITX_INFOFRAM_TYPE_AVI     ? "AVI"   \
     : x == E_MHAL_HDMITX_INFOFRAM_TYPE_SPD   ? "AVI"   \
     : x == E_MHAL_HDMITX_INFOFRAM_TYPE_AUDIO ? "AUDIO" \
                                              : "UNKNOWN")

#define PARSING_DRV_OUTPUT_MODE(x)                            \
    (x == E_MHAL_HDMITX_OUTPUT_MODE_DVI         ? "DVI"       \
     : x == E_MHAL_HDMITX_OUTPUT_MODE_DVI_HDCP  ? "DVI_HDCP"  \
     : x == E_MHAL_HDMITX_OUTPUT_MODE_HDMI      ? "HDMI"      \
     : x == E_MHAL_HDMITX_OUTPUT_MODE_HDMI_HDCP ? "HDMI_HDCP" \
     : x == E_MHAL_HDMITX_OUTPUT_MODE_AUTO      ? "AUTO"      \
                                                : "UNKNOWN")

#define PARSING_DRV_COLOR_FMT(x)                  \
    (x == E_MHAL_HDMITX_COLOR_RGB444   ? "RGB444" \
     : x == E_MHAL_HDMITX_COLOR_YUV444 ? "YUV444" \
     : x == E_MHAL_HDMITX_COLOR_YUV422 ? "YUV422" \
     : x == E_MHAL_HDMITX_COLOR_YUV420 ? "YUV420" \
     : x == E_MHAL_HDMITX_COLOR_AUTO   ? "AUTO"   \
                                       : "UNKNOWN")

#define PARSING_DRV_TIMING(x)                                  \
    (x == E_MHAL_HDMITX_RES_720X480P_60HZ     ? "720x480p60"   \
     : x == E_MHAL_HDMITX_RES_720X576P_50HZ   ? "720x576p50"   \
     : x == E_MHAL_HDMITX_RES_1280X720P_60HZ  ? "1280x720p60"  \
     : x == E_MHAL_HDMITX_RES_1280X720P_50HZ  ? "1280x720p50"  \
     : x == E_MHAL_HDMITX_RES_1920X1080P_24HZ ? "1920x1080p24" \
     : x == E_MHAL_HDMITX_RES_1920X1080P_25HZ ? "1920x1080p25" \
     : x == E_MHAL_HDMITX_RES_1920X1080P_30HZ ? "1920x1080p30" \
     : x == E_MHAL_HDMITX_RES_1920X1080P_50HZ ? "1920x1080p50" \
     : x == E_MHAL_HDMITX_RES_1920X1080P_60HZ ? "1920x1080p60" \
     : x == E_MHAL_HDMITX_RES_3840X2160P_24HZ ? "3840x2160p24" \
     : x == E_MHAL_HDMITX_RES_3840X2160P_25HZ ? "3840x2160p25" \
     : x == E_MHAL_HDMITX_RES_3840X2160P_30HZ ? "3840x2160p30" \
     : x == E_MHAL_HDMITX_RES_3840X2160P_50HZ ? "3840x2160p50" \
     : x == E_MHAL_HDMITX_RES_3840X2160P_60HZ ? "3840x2160p60" \
     : x == E_MHAL_HDMITX_RES_1024X768P_60HZ  ? "1024x768p60"  \
     : x == E_MHAL_HDMITX_RES_1366X768P_60HZ  ? "1366x768p60"  \
     : x == E_MHAL_HDMITX_RES_1440X900P_60HZ  ? "1440x900p60"  \
     : x == E_MHAL_HDMITX_RES_1280X800P_60HZ  ? "1280x800p60"  \
     : x == E_MHAL_HDMITX_RES_1280X1024P_60HZ ? "1280x1024p60" \
     : x == E_MHAL_HDMITX_RES_1680X1050P_60HZ ? "1680x1050p60" \
     : x == E_MHAL_HDMITX_RES_1600X1200P_60HZ ? "1600x1200p60" \
     : x == E_MHAL_HDMITX_RES_2560X1440P_30HZ ? "2560x1440p30" \
     : x == E_MHAL_HDMITX_RES_1920X1200P_60HZ ? "1920x1200p60" \
                                              : "UNKNOWN")

#define PARSING_DRV_AVI_INFO_COLORIMETRY(x)                   \
    (x == E_MHAL_HDMITX_COLORIMETRY_NO_DATA     ? "NO_DATA"   \
     : x == E_MHAL_HDMITX_COLORIMETRY_SMPTE170M ? "SMPTE170M" \
     : x == E_MHAL_HDMITX_COLORIMETRY_ITUR709   ? "ITUR709"   \
     : x == E_MHAL_HDMITX_COLORIMETRY_EXTEND    ? "EXTEND"    \
                                                : "UNKNOWN")

#define PARSING_DRV_AVI_INFO_EXT_COLORIMETRY(x)                       \
    (x == E_MHAL_HDMITX_EXT_COLORIMETRY_XVYCC601      ? "XVYCC601"    \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_XVYCC709    ? "XVYCC709"    \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_SYCC601     ? "SYCC601"     \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_ADOBEYCC601 ? "ADOBEYCC601" \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_ADOBERGB    ? "ADOBERGB"    \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_BT2020CYCC  ? "BT2020CYCC"  \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_BT2020YCC   ? "BT2020YCC"   \
     : x == E_MHAL_HDMITX_EXT_COLORIMETRY_BT2020RGB   ? "BT2020RGB"   \
                                                      : "UNKNOWN")

#define PARSING_DRV_AVI_INFO_ASPECTRATIO(x)              \
    (x == E_MHAL_HDMITX_ASPECT_RATIO_INVALID ? "INVALID" \
     : x == E_MHAL_HDMITX_ASPECT_RATIO_4TO3  ? "4To3"    \
     : x == E_MHAL_HDMITX_ASPECT_RATIO_16TO9 ? "16To9"   \
     : x == E_MHAL_HDMITX_ASPECT_RATIO_21TO9 ? "21To9"   \
                                             : "UNKNOWN")

#define PARSING_DRV_AVI_INFO_YCC_QUANT_RANE(x)                   \
    (x == E_MHAL_HDMITX_YCC_QUANTIZATION_LIMITED_RANGE ? "LIMIT" \
     : x == E_MHAL_HDMITX_YCC_QUANTIZATION_FULL_RANGE  ? "FULL"  \
                                                       : "UNKNOWN")

#define PARSING_DRV_AVI_INFO_AFD_RATIO(x)                             \
    (x == E_MHAL_HDMITX_VIDEO_AFD_SameAsPictureAR ? "SameAsPictureAR" \
     : x == E_MHAL_HDMITX_VIDEO_AFD_4_3_Center    ? "4_3_Center"      \
     : x == E_MHAL_HDMITX_VIDEO_AFD_16_9_Center   ? "16_9_Center"     \
     : x == E_MHAL_HDMITX_VIDEO_AFD_14_9_Center   ? "14_9_Center"     \
     : x == E_MHAL_HDMITX_VIDEO_AFD_Others        ? "Other"           \
                                                  : "UNKNOWN")

#define PARSING_DRV_AVI_INFO_SCAN_INFO(x)                      \
    (x == E_MHAL_HDMITX_SCAN_INFO_NO_DATA        ? "NO_DATA"   \
     : x == E_MHAL_HDMITX_SCAN_INFO_OVERSCANNED  ? "OVERSCAN"  \
     : x == E_MHAL_HDMITX_SCAN_INFO_UNDERSCANNED ? "UNDERSCAN" \
     : x == E_MHAL_HDMITX_SCAN_INFO_FUTURE       ? "FUTURE"    \
                                                 : "UNKNOWN")

#define PARSING_DRV_AUD_INFO_CODE(x) \
    (x == E_MHAL_HDMITX_AUDIO_CODE_PCM ? "PCM" : x == E_MHAL_HDMITX_AUDIO_CODE_NON_PCM ? "NON_PCM" : "UNKNOWN")

#define PARSING_DRV_AUD_INFO_SAMPLE(x)                              \
    (x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_UNKNOWN ? "UNKNOWN_SAMPLE" \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_32K   ? "32K"            \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_44K   ? "44K"            \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_48K   ? "48K"            \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_88K   ? "88K"            \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_96K   ? "96K"            \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_176K  ? "176K"           \
     : x == E_MHAL_HDMITX_AUDIO_SAMPLERATE_192K  ? "192K"           \
                                                 : "UNKNOWN")

#define PARSING_DRV_AUDIO_FREQ(x)                    \
    (x == E_MHAL_HDMITX_AUDIO_FREQ_NO_SIG ? "NO_SIG" \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_32K  ? "32K"    \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_44K  ? "44K"    \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_48K  ? "48K"    \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_88K  ? "88K"    \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_96K  ? "96K"    \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_176K ? "176K"   \
     : x == E_MHAL_HDMITX_AUDIO_FREQ_192K ? "192K"   \
                                          : "UNKNOWN")

#define PARSING_DRV_AUDIO_CH(x) \
    (x == E_MHAL_HDMITX_AUDIO_CH_2 ? "CH_2" : x == E_MHAL_HDMITX_AUDIO_CH_8 ? "CH_8" : "UNKNOWN")

#endif
