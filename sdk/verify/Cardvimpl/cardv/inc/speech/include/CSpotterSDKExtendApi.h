/*
* CSpotterSDKExtendApi.h- Sigmastar
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

#ifndef __CSPOTTERSDK_EXTEND_API_H
#define __CSPOTTERSDK_EXTEND_API_H

#if defined(__SYMBIAN32__)
	#ifdef CSpotterDll_EXPORTS
		#define CSPDLL_API EXPORT_C
	#endif
#elif defined(_WIN32)
	#ifdef CSpotterDll_EXPORTS
		#define CSPDLL_API __declspec(dllexport)
	#endif
#endif

#ifndef CSPDLL_API
#define CSPDLL_API
#endif

#include "base_types.h"
#include "CSpotterSDKApi_Const.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Get Command
CSPDLL_API INT CSpotter_GetCommandNumber( HANDLE hCSpotter ) ;
CSPDLL_API INT CSpotter_GetCommand( HANDLE hCSpotter , INT nID , UNICODE* lpwchBuffer ) ;

// Threshold Adjust API
CSPDLL_API INT CSpotter_GetThreshold( HANDLE hCSpotter , INT* lpnGMM1 , INT* lpnSG1 , INT* lpnGMM2 , INT* lpnSG2 ) ;
CSPDLL_API INT CSpotter_SetThreshold( HANDLE hCSpotter , INT nGMM1 , INT nSG1 , INT nGMM2 , INT nSG2 ) ;
CSPDLL_API INT CSpotter_SetWordAddScore( HANDLE hCSpotter , INT nWordIdx , INT nAddScore ) ;

#ifdef __cplusplus
}
#endif

#endif // __CSPOTTERSDK_EXTEND_API_H
