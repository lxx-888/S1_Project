/*
 * module_audio.h- Sigmastar
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
#ifndef _MODULE_AUDIO_H_
#define _MODULE_AUDIO_H_

#include "mid_audio_type.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define MAX_AUDIO_INPUT  (1)
#define MAX_AUDIO_OUTPUT (2)

#define AI_DEV_ID_ADC_0_1     (0)
#define AI_DEV_ID_DMIC        (1)
#define AI_DEV_ID_I2S_RX      (2)
#define AI_DEV_ID_LINE_IN     (3)
#define AI_DEV_ID_ADC_2_3     (4)
#define AI_DEV_ID_ADC_0_1_2_3 (5)

void audioVqeVolumeTrans(MI_U32 u32DevId, MI_S8 *s32InputDB);

#endif //#define _MODULE_AUDIO_H_