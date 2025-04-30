/*
 * module_menusetting.h- Sigmastar
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
#ifndef _MODULE_MENUSETTING_H_
#define _MODULE_MENUSETTING_H_

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define GET_CONFIG_SETTING "nvconf get 0"
#define SET_CONFIG_SETTING "nvconf set 0"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

BOOL cardv_menu_getResult(const char *cmd, char *outBuf, int inLen);
void cardv_menu_updateSettingFromConfig(void);
void cardv_menu_updateVideoRecordSize(MI_U32 u32Width, MI_U32 u32Height);

#endif //#define _MODULE_MENUSETTING_H_
