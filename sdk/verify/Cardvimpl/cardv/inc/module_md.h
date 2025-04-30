/*
 * module_md.h- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
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

#ifndef __MODULE_MD_H__
#define __MODULE_MD_H__

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern MI_S32 g_MdtLevel;
extern MI_S32 g_MdSens;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int MD_Param_Change(MI_S8 *param, MI_S32 paramLen);
int MD_Initial(int param);
int MD_Uninitial(void);

#endif /* __MODULE_MD_H__ */