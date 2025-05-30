/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include <drv_sensor_earlyinit_common.h>
#include <drv_sensor_earlyinit_method_1.h>
#include "SC2336P_init_table.h"         /* Sensor initial table */
/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC2336P)

static EarlyInitSensorInfo_t _gSC2336PInfo =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 1920,
    .u32Height      = 1080,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
    .u32CropX       = 0,
    .u32CropY       = 0
};

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int SC2336PEarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 29630
unsigned int vts_30fps = 1125;

    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x3e00, 0x00},
        {0x3e01, 0x5d},
        {0x3e02, 0x00},
        /*vts*/
        {0x320e, 0x05},
        {0x320f, 0xdc},

    };

    vts_30fps = (vts_30fps * 30000) / nFpsX1000;

    lines = (1000 * nShutterUs ) / Preview_line_period; // Preview_line_period in ns
    if(lines <= 1)
    {
        lines = 1;
    }
    if(lines > (vts_30fps - 6))
    {
        vts = (lines + 6) ;
    }
    else
    {
        vts = vts_30fps;// 15fps ,meanwhile 0xa6a
    }
    lines = lines<<4;

    expo_reg[0].u16Data = (lines>>16) & 0x000f;
    expo_reg[1].u16Data = (lines>>8) & 0x00ff;
    expo_reg[2].u16Data = (lines>>0) & 0x00f0;

    expo_reg[3].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[4].u16Data = (vts >> 0) & 0x00ff;


    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    _gSC2336PInfo.u32ShutterUs = nShutterUs;
    _gSC2336PInfo.u32FpsX1000 = nFpsX1000;
    return nNumRegs; //Return number of sensor registers to write
}

/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
typedef unsigned long long _u64;
typedef unsigned long _u32;
typedef unsigned short _u16;
static unsigned int SC2336PEarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAXGAIN     (32000*4 / 1000) // (32768*15 / 1000)//32XAGAIN 15XDGAIN
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
	_u64 gain_factor;
    _u64 gain_reg_0x3e09,gain_reg_0x3e06,gain_reg_0x3e07;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e09, 0x00},
    };

    _gSC2336PInfo.u32GainX1024 = u32GainX1024;

     if (u32GainX1024 <= 1024) {
        u32GainX1024 = 1024;
    } else if (u32GainX1024 > SENSOR_MAXGAIN*1024) {
        u32GainX1024 = SENSOR_MAXGAIN*1024;
    }
	gain_factor = u32GainX1024 * 1000 / 1024;
	 
	if(gain_factor < 1000) gain_factor = 1000;
	if(gain_factor > 128*1000) gain_factor =128*1000;
	
	if (gain_factor < 2000)
	    {
			gain_reg_0x3e09 = 0x00;
			gain_reg_0x3e06 = 0x00;
			gain_reg_0x3e07 = (gain_factor * 128 / 1000 / 4) * 4 ;
	    }
	else if( gain_factor < 4000)
		{
			gain_reg_0x3e09 = 0x08;
			gain_reg_0x3e06 = 0x00;
			gain_reg_0x3e07 = (gain_factor * 128 / 2000 / 4) * 4 ;
		}
	else if( gain_factor < 8000)
		{
			gain_reg_0x3e09 = 0x09;
			gain_reg_0x3e06 = 0x00;
			gain_reg_0x3e07 = (gain_factor * 128 / 4000 / 4) * 4 ;
		}
	else if( gain_factor < 16000)
		{
			gain_reg_0x3e09 = 0x0b;
			gain_reg_0x3e06 = 0x00;
			gain_reg_0x3e07 = (gain_factor * 128 / 8000 / 4) * 4 ;
		}
	else if( gain_factor < 32000)
		{
			gain_reg_0x3e09 = 0x0f;
			gain_reg_0x3e06 = 0x00;
			gain_reg_0x3e07 = (gain_factor * 128 / 16000 / 4) * 4 ;
		}
	else if( gain_factor < 32000*2)//open dgain begin  max digital gain 4X
		{
			gain_reg_0x3e09 = 0x1f;
			gain_reg_0x3e06 = 0x00;
			gain_reg_0x3e07 = (gain_factor * 128 / 32000 / 4) * 4 ;
		}
	else if( gain_factor < 32000*4){
			gain_reg_0x3e09 = 0x1f;
			gain_reg_0x3e06 = 0x01;
			gain_reg_0x3e07 = (gain_factor * 128 / 32000 / 2 / 4) * 4 ;
	}else{
			gain_reg_0x3e09 = 0x1f;
			gain_reg_0x3e06 = 0x03;
			gain_reg_0x3e07 = 0x80;
	}
    gain_reg[2].u16Data = gain_reg_0x3e09; // 0x3e09
    gain_reg[1].u16Data = gain_reg_0x3e07; // 0x3e07
    gain_reg[0].u16Data = gain_reg_0x3e06; // 0x3e06

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int SC2336PEarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = _gSC2336PInfo.eBayerID;
        pSnrInfo->ePixelDepth   = _gSC2336PInfo.ePixelDepth;
        pSnrInfo->eIfBusType    = _gSC2336PInfo.eIfBusType;
        pSnrInfo->u32FpsX1000   = _gSC2336PInfo.u32FpsX1000;
        pSnrInfo->u32Width      = _gSC2336PInfo.u32Width;
        pSnrInfo->u32Height     = _gSC2336PInfo.u32Height;
        pSnrInfo->u32GainX1024  = _gSC2336PInfo.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _gSC2336PInfo.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _gSC2336PInfo.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _gSC2336PInfo.u32GainShortX1024;
        pSnrInfo->u8ResId       = _gSC2336PInfo.u8ResId;
        pSnrInfo->u8HdrMode     = _gSC2336PInfo.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 200;
        pSnrInfo->u32CropX       = _gSC2336PInfo.u32CropX;
        pSnrInfo->u32CropY       = _gSC2336PInfo.u32CropY;
    }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( SC2336P,
                                 Sensor_init_table,
                                 SC2336PEarlyInitShutterAndFps,
                                 SC2336PEarlyInitGain,
                                 SC2336PEarlyInitGetSensorInfo
                                );
