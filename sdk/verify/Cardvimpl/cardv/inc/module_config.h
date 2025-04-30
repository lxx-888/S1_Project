/*
 * module_config.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
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
#ifndef _MODULE_CONFIG_H_
#define _MODULE_CONFIG_H_

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_OSD_ENABLE        (1)
#define CARDV_UVC_ENABLE        (0)
#define CARDV_VIF_ENABLE        (1)
#define CARDV_VDEC_ENABLE       (0 && INTERFACE_VDEC)
#define CARDV_JPD_ENABLE        (0 && INTERFACE_JPD)
#define CARDV_LDC_ENABLE        (0 && INTERFACE_LDC)
#define CARDV_DISALGO_ENABLE    (0 && CARDV_LDC_ENABLE)
#define CARDV_VDISP_ENABLE      (0 && INTERFACE_VDISP)
#define CARDV_DISPLAY_ENABLE    (1 && INTERFACE_DISP)
#define CARDV_PANEL_ENABLE      (1 && INTERFACE_PANEL)
#define CARDV_HDMI_ENABLE       (0 && INTERFACE_HDMI)
#define CARDV_HDMI_AUDIO_ENABLE (0 && CARDV_HDMI_ENABLE)
#define CARDV_MIPITX_ENABLE     (0 && INTERFACE_MIPITX)
#define CARDV_ISP_ENABLE        (1)
#define CARDV_IQSERVER_ENABLE   (1)
#define CARDV_LIVE555_ENABLE    (1)
#define CARDV_EDOG_ENABLE       (0)
#define CARDV_GPS_ENABLE        (0)
#define CARDV_SPEECH_ENABLE     (0)
#define CARDV_FB_ENABLE         (0)
#define CARDV_GUI_ENABLE        (0)
#define CARDV_WATCHDOG_ENABLE   (0)
#define CARDV_VDF_ENABLE        (0 && INTERFACE_VDF)
#define CARDV_DMS_ENABLE        (0 && DMS)
#define CARDV_ADAS_ENABLE       (0 && ADAS)
#define CARDV_LD_ENABLE         (0 && LIGHTDETECT)
#define CARDV_LD_VDEC_SRC_DEBUG (0 && LIGHTDETECT)
#define CARDV_ALINK_ENABLE      (0 && ALINK)
#define CARDV_EXFAT_ENABLE      (0) // need exfat.ko
#define CARDV_RTSP_INPUT_ENABLE (0)
#define CARDV_WS_INPUT_ENABLE   (0)
#define CARDV_WS_OUTPUT_ENABLE  (0)

#if (CARDV_RTSP_INPUT_ENABLE && CARDV_WS_INPUT_ENABLE)
#error "CARDV_RTSP_INPUT_ENABLE & CARDV_WS_INPUT_ENABLE can't be enabled at the same time now."
#endif

#endif //#define _MODULE_CONFIG_H_
