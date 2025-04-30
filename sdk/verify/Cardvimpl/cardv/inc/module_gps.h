/*
 * module_gps.h- Sigmastar
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
#ifndef _MODULE_GPS_H_
#define _MODULE_GPS_H_

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define GPS_NOSIGNAL_MARK_DOUBLE 0xFFFFFFFF
#define GPS_NOSIGNAL_MARK_LONG   0xFFFFFFFF
#define GPS_NOSIGNAL_MARK_SHORT  0xFFFF
#define GPS_NOSIGNAL_MARK_BYTE   0xFF

#define GPS_CONFIG_INFOSTRUC_VER 0x01

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct GPSINFOCHUCKTIME_s
{
    unsigned char byYear; /**< Years since 1900 */
    unsigned char byMon;  /**< Months since January - [0,11] */
    unsigned char byDay;  /**< Day of the month - [1,31] */
    unsigned char byHour; /**< Hours since midnight - [0,23] */
    unsigned char byMin;  /**< Minutes after the hour - [0,59] */
    unsigned char bySec;  /**< Seconds after the minute - [0,59] */
} GPSINFOCHUCKTIME;

typedef struct GPSINFOCHUCK_s
{
    double           dwLat;       /**< Latitude in NDEG - +/-[degree][min].[sec/60] */
    double           dwLon;       /**< Longitude in NDEG - +/-[degree][min].[sec/60] */
    long             lAlt;        /**< Altitude in meter +-:under/below sea level*/
    unsigned short   usSpeed;     /**< Speed unit: km/h */
    GPSINFOCHUCKTIME sUTC;        /**< UTC of position */
    unsigned char    ubDirection; /**< Clockwise degree from the North.*/
    unsigned char    ubFlag;      /**< Check if the GPS data is valid;*/
    unsigned char    ubVersion;   /**< Stuture Version*/
    unsigned char    ubReserved;
} GPSINFOCHUCK;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void           GpsStartThread(void);
void           GpsStopThread(void);
unsigned char *GpsGetChunk(int *pSize);
void           GpsStartRec(MI_BOOL bStart);

void EDOGCtrl_Operation(int *param, int paramLen);

#endif //#define _MODULE_GPS_H_