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
#include "earlyinit_drv_iic.h"
#include "cam_os_wrapper.h"
#include "sys_sys_boot_timestamp.h"
#include "SC301IOT_init_table.h"         /* Sensor initial table */
#if defined(CAM_OS_RTK)
#include <stdlib.h>
#endif
extern void CamOsPrintf(const char *szFmt, ...);

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC301IOT)
typedef unsigned long long _u64;
typedef unsigned long _u32;
typedef unsigned short _u16;
typedef unsigned char _u8;

static EarlyInitHwRes_t* gHwRes[_MAX_SENSOR_PADID];

static EarlyInitSensorInfo_t _gSC301IOTInfo_4M30 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 2048,
    .u32Height      = 1536,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8Mirror       = 0,
    .u8Flip         = 0,
    .u32TimeoutMs   = 200,
    .u8NumLanes     = 2,
    .u8StreamOnoff  = 1
};
#ifdef _SC301IOT_RES_4M30_HDR
static EarlyInitSensorInfo_t _gSC301IOTInfo_3M30_HDR =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 2048,
    .u32Height      = 1536,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 1,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8Mirror       = 0,
    .u8Flip         = 0,
    .u32TimeoutMs   = 200,
    .u8NumLanes     = 2,
    .u8StreamOnoff  = 1
};
#endif
EarlyInitSensorInfo_t* pResList[] =
{
#ifdef _SC301IOT_RES_4M30
    &_gSC301IOTInfo_4M30,
    &_gSC301IOTInfo_4M30,
    &_gSC301IOTInfo_4M30,
#endif
#ifdef _SC301IOT_RES_4M30_HDR
    &_gSC301IOTInfo_3M30_HDR,
#endif
};

#define  Preview_line_period 20833                           // 33.333/1600=20833
#define  Preview_line_period_HDR 10417
#define  vts_30fps  1600                                //1090 //for 30fps @ MCLK=27MHz
#define  vts_30fps_HDR 3200
#define SENSOR_MAXGAIN      1024*1024   /////sensor again 15.875 dgain=31.5
/*****************  Interface VER2 ****************/
static void _memcpy(void* dest, void* src, unsigned int len)
{
    unsigned int n;
    for(n=0; n<len; ++n)
        ((char*)dest)[n] = ((char*)src)[n];
}

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int SC301IOTEarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;

    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x3e00, 0x00},//expo [20:17]
        {0x3e01, 0x8c}, // expo[16:8]
        {0x3e02, 0x60}, // expo[7:0], [3:0] fraction of line
        /*vts*/
        {0x320e, 0x04},
        {0x320f, 0x65},
    };
    /*VTS*/
    vts =  (vts_30fps*30000)/nFpsX1000;

    if(nFpsX1000<1000)    //for old method
        vts = (vts_30fps*30)/nFpsX1000;

    /*Exposure time*/
    lines = (1000*nShutterUsLef)/Preview_line_period;
    if(lines<=2)
        lines=2;
    if (lines > vts-8){
         vts = (lines+8);
    }
    lines = lines<<4;

    expo_reg[0].u16Data = (lines>>16) & 0x0f;
    expo_reg[1].u16Data = (lines>>8) & 0xff;
    expo_reg[2].u16Data = (lines>>0) & 0xf0;
    //vts
    expo_reg[3].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[4].u16Data = (vts >> 0) & 0x00ff;


    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    pHandle->tInfo.u32ShutterUs = nShutterUsLef;
    pHandle->tInfo.u32FpsX1000  = nFpsX1000;
    return nNumRegs; //Return number of sensor registers to write
}

static unsigned int SC301IOTEarlyInitShutterAndFpsHdr(EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    #define MAX_SHORT_EXP 191
    unsigned char n = 0;
    int nNumRegs = 0;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    I2cCfg_t hdr_expo_reg[] = {
        {0x3e04, 0x21}, // expo[7:0]
        {0x3e05, 0x00}, // expo[7:4]
        /*vts*/
        {0x320e, 0x04},
        {0x320f, 0x65},
    };
    /*VTS*/
    vts =  (vts_30fps_HDR*30000)/nFpsX1000;
    if(nFpsX1000<1000)    //for old method
        vts = (vts_30fps_HDR*30)/nFpsX1000;

    dou_lines = (1000*nShutterUsLef)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 2*dou_lines;
    if(half_lines<2) half_lines=2;
    if (half_lines > MAX_SHORT_EXP-9) {//10 update for danny 20220427
        half_lines =  MAX_SHORT_EXP-9;
    }
    half_lines = half_lines<<4;

    hdr_expo_reg[0].u16Data =  (half_lines>>8) & 0xff;
    hdr_expo_reg[1].u16Data = (half_lines>>0) & 0xf0;
    //vts
    hdr_expo_reg[2].u16Data = (vts >> 8) & 0x00ff;
    hdr_expo_reg[3].u16Data = (vts >> 0) & 0x00ff;

    for(n=0;n<sizeof(hdr_expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)hdr_expo_reg)[n];
    }
    nNumRegs = sizeof(hdr_expo_reg)/sizeof(hdr_expo_reg[0]);

    pHandle->tInfo.u32ShutterUs = nShutterUsLef;
    pHandle->tInfo.u32FpsX1000  = nFpsX1000;
    return nNumRegs; //Return number of sensor registers to write
}

/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int SC301IOTEarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    _u16 i=0 ;
    _u32 reg_gain = 0, dcg = 1024, DIG_gain = 1, DIG_Fine_gain_reg = 0;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e09, 0x20},
    };
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;

    if (u32GainLefX1024<1024){
        u32GainLefX1024=1024;
    }else if (u32GainLefX1024>=SENSOR_MAXGAIN){
        u32GainLefX1024=SENSOR_MAXGAIN;
    }
    if((u32GainLefX1024 >= 1024)&&(u32GainLefX1024 < 1607)) // x1.569
    {
        dcg = 1024;
        reg_gain = 0x00;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 1607)&&(u32GainLefX1024 < 3214)) // x1.569~ x3.138
    {
        dcg = 1607;
        reg_gain = 0x40;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 3214)&&(u32GainLefX1024 < 6428)) // x3.138 ~ x6.276
    {
        dcg = 3214;
        reg_gain = 0x48;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 6428)&&(u32GainLefX1024 < 12856)) // x6.276 ~ x12.552
    {
        dcg = 6428;
        reg_gain = 0x49;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 12856)&&(u32GainLefX1024 < 25712)) // x12.552 ~ x25.104
    {
        dcg = 12856;
        reg_gain = 0x4b;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 25712)&&(u32GainLefX1024 < 51424)) // x25.104 ~ x50.208
    {
        dcg = 25712;
        reg_gain = 0x4f;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 51424)&&(u32GainLefX1024 < 102848)) // x50.208 ~ x50.208*2
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 102848)&&(u32GainLefX1024 < 205696)) //  x50.208*2 ~ x50.208*4
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 2;
    }
    else if((u32GainLefX1024 >= 205696)&&(u32GainLefX1024 < 411392)) // x50.208*4 ~ x50.208*8
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 4;
    }
    else if((u32GainLefX1024 >= 411392)&&(u32GainLefX1024 < (411392*2))) // x50.208*8 ~ x50.208*16
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 8;
    }
    else if((u32GainLefX1024 >= (411392*2))&&(u32GainLefX1024 < 1619509)) // x50.208*16 ~ x50.208*31.5
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = ( 8 * 2 );
    }
    DIG_Fine_gain_reg = (_u8)((128*u32GainLefX1024/dcg/DIG_gain/4)*4);
    gain_reg[2].u16Data = reg_gain;  // 0x3e09
    gain_reg[1].u16Data = DIG_Fine_gain_reg;  // 0x3e07
    gain_reg[0].u16Data = DIG_gain-1;  // 0x3e06

    for (i = 0; i < sizeof(gain_reg); i++){
        ((unsigned char*)pRegs)[i] = ((unsigned char*)gain_reg)[i];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int SC301IOTEarlyInitGainHdr(EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    _u16 i=0 ;
    _u32 reg_gain = 0, dcg = 1024, DIG_gain = 1, DIG_Fine_gain_reg = 0;
    int nNumRegs = 0;
    I2cCfg_t hdr_gain_reg[] = {
        {0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e13, 0x00},
    };
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
    if (u32GainLefX1024 <= 1024) {
        u32GainLefX1024 = 1024;
    } else if (u32GainLefX1024 > SENSOR_MAXGAIN*1024) {
        u32GainLefX1024 = SENSOR_MAXGAIN*1024;
    }

    if((u32GainLefX1024 >= 1024)&&(u32GainLefX1024 < 1607)) // x1.569
    {
        dcg = 1024;
        reg_gain = 0x00;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 1607)&&(u32GainLefX1024 < 3214)) // x1.569~ x3.138
    {
        dcg = 1607;
        reg_gain = 0x40;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 3214)&&(u32GainLefX1024 < 6428)) // x3.138 ~ x6.276
    {
        dcg = 3214;
        reg_gain = 0x48;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 6428)&&(u32GainLefX1024 < 12856)) // x6.276 ~ x12.552
    {
        dcg = 6428;
        reg_gain = 0x49;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 12856)&&(u32GainLefX1024 < 25712)) // x12.552 ~ x25.104
    {
        dcg = 12856;
        reg_gain = 0x4b;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 25712)&&(u32GainLefX1024 < 51424)) // x25.104 ~ x50.208
    {
        dcg = 25712;
        reg_gain = 0x4f;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 51424)&&(u32GainLefX1024 < 102848)) // x50.208 ~ x50.208*2
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 1;
    }
    else if((u32GainLefX1024 >= 102848)&&(u32GainLefX1024 < 205696)) //  x50.208*2 ~ x50.208*4
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 2;
    }
    else if((u32GainLefX1024 >= 205696)&&(u32GainLefX1024 < 411392)) // x50.208*4 ~ x50.208*8
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 4;
    }
	    else if((u32GainLefX1024 >= 411392)&&(u32GainLefX1024 < (411392*2))) // x50.208*8 ~ x50.208*16
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 8;
    }
	    else if((u32GainLefX1024 >= (411392*2))&&(u32GainLefX1024 < 1619509)) // x50.208*16 ~ x50.208*31.5
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = ( 8 * 2 );
    }
    DIG_Fine_gain_reg = abs((128*u32GainLefX1024/dcg/DIG_gain/4)*4);

    hdr_gain_reg[2].u16Data = reg_gain;  // 0x3e09
    hdr_gain_reg[1].u16Data = DIG_Fine_gain_reg;  // 0x3e07
    hdr_gain_reg[0].u16Data = DIG_gain-1;  // 0x3e06
    for (i = 0; i < sizeof(hdr_gain_reg); i++){
        ((unsigned char*)pRegs)[i] = ((unsigned char*)hdr_gain_reg)[i];
    }
    nNumRegs = sizeof(hdr_gain_reg)/sizeof(hdr_gain_reg[0]);
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
    pHandle->tInfo.u32GainShortX1024 = u32GainSefX1024;
    return nNumRegs;
}

static unsigned int SC301IOTEarlyInitGainEx(EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    switch(pHandle->uPresetId)
    {
    case SC301IOT_PRESET_4M30:
    case SC301IOT_PRESET_POWERON:
    case SC301IOT_PRESET_4M30_FASTAE:
        return SC301IOTEarlyInitGainLinear(pHandle, u32GainLefX1024, u32GainSefX1024,u32GainMefX1024, pRegs, nMaxNumRegs);
        break;
    case SC301IOT_PRESET_3M30_HDR:
        return SC301IOTEarlyInitGainHdr(pHandle, u32GainLefX1024, u32GainSefX1024,u32GainMefX1024, pRegs, nMaxNumRegs);
        break;
    }
    return 0;
}

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int SC301IOTEarlyInitShutterAndFpsEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    switch(pHandle->uPresetId)
    {
    case SC301IOT_PRESET_4M30:
    case SC301IOT_PRESET_POWERON:
    case SC301IOT_PRESET_4M30_FASTAE:
        return SC301IOTEarlyInitShutterAndFpsLinear(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef, nShutterUsMef, pRegs, nMaxNumRegs);
        break;
    case SC301IOT_PRESET_3M30_HDR:
        return SC301IOTEarlyInitShutterAndFpsHdr(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef,nShutterUsMef, pRegs, nMaxNumRegs);
        break;
    }
    return 0;
}

static unsigned int SC301IOTEarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case SC301IOT_PRESET_4M30:
        pHandle->pTable = (void*) Sensor_init_table;
        pHandle->uTableSize = sizeof(Sensor_init_table);
        _memcpy(&pHandle->tInfo, &_gSC301IOTInfo_4M30, sizeof(_gSC301IOTInfo_4M30));
        break;
    case SC301IOT_PRESET_POWERON:
        pHandle->pTable = (void*) SC301IOT_init_table_PowerOn;
        pHandle->uTableSize = sizeof(SC301IOT_init_table_PowerOn);
        _memcpy(&pHandle->tInfo, &_gSC301IOTInfo_4M30, sizeof(_gSC301IOTInfo_4M30));
        break;
    case SC301IOT_PRESET_4M30_FASTAE:
        pHandle->pTable = (void*) SC301IOT_init_table_4M30_FASTAE;
        pHandle->uTableSize = sizeof(SC301IOT_init_table_4M30_FASTAE);
        _memcpy(&pHandle->tInfo, &_gSC301IOTInfo_4M30, sizeof(_gSC301IOTInfo_4M30));
        break;
    case SC301IOT_PRESET_3M30_HDR:
        pHandle->pTable = (void*) Sensor_3M30_HDR_init_table;
        pHandle->uTableSize = sizeof(Sensor_3M30_HDR_init_table);
        _memcpy(&pHandle->tInfo, &_gSC301IOTInfo_3M30_HDR, sizeof(_gSC301IOTInfo_3M30_HDR));
        break;
    default:
        return -1;
        break;
    }
    return 0;
}

unsigned int SC301IOTEarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    if((uResId >= sizeof(pResList)/sizeof(pResList[0])) || (uHdrMode != pResList[uResId]->u8HdrMode))
    {
        return -1;
    }
    SC301IOTEarlyInitSelPresetId(pHandle, uResId);
    return 0;

}

static unsigned int SC301IOTEarlyInitSelOrientEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uMirror, unsigned char uFlip, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t orient_reg[] =
    {
        {0x3221, 0x00},//low bit
    };

    if(uFlip)
    {
        orient_reg[0].u16Data  |= 0x60;
    }
    if(uMirror)
    {
        orient_reg[0].u16Data  |= 0x6;
    }

    /*copy result*/
    for(n=0;n<sizeof(orient_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)orient_reg)[n];
    }
    pHandle->tInfo.u8Mirror = uMirror;
    pHandle->tInfo.u8Flip   = uFlip;
    return sizeof(orient_reg)/sizeof(orient_reg[0]);
}

static unsigned int SC301IOTEarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t stream_onoff_reg[] =
    {
        {0x0100,0x01},//0x01 stream on,0x0 stream off
    };
    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    if(uStreamOnoff == 1)
    {
        BootTimestampRecord(__LINE__, "early301 oepn stream ");
        stream_onoff_reg[0].u16Data = uStreamOnoff;
    }
    else
    {
        BootTimestampRecord(__LINE__, "early301 close stream ");
        stream_onoff_reg[0].u16Data = uStreamOnoff;
    }
    /*copy result*/
    for(n=0;n<sizeof(stream_onoff_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)stream_onoff_reg)[n];
    }
    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    return sizeof(stream_onoff_reg)/sizeof(stream_onoff_reg[0]);
}

static unsigned int SC301IOTEarlyInitFastAe(EarlyInitSensorDrvCfg_t *pHandle, unsigned long *pShutter, unsigned long *pGain)
{
    /* //Big_Exp = L_Target * sExp * sRowTime * sGain / sValue / 0x10 / bRowTime * 13 / 9
    //sRowTime = 20833 ns
    //bRowTime = Preview_line_period
    //bExp = (Big_Exp > maxExp) ? maxExp : Big_exp
    //bGain = Big_Exp / bExp
    0x5e32,0x01,//stable?
    0x5e20,0x2a,//sValue
    0x5e03,0x32,//L_target
    0x3e00,0x00,//sExp
    0x3e01,0x0c,//
    0x3e02,0xc0,//
    0x3e08,0x00,//sGain
    0x3e09,0xf9,//
    0x5e24,0x7d,//rgain
    0x5e25,0x77.//bgain */
    u64 Big_Exp = 0;
    u16 ae_stable = 0, L_target = 0, sValue = 0;
    u64 sExp = 0, sGain =  0, bExp = 0, bGain = 0;
    u64 sRowTime = 20833;
    u64 bRowTime = Preview_line_period;
    u8 i=0 ;
    u16 awb_rGain = 0, awb_bGain = 0;
    u16 awb_rGain_isp = 0, awb_bGain_isp = 0;
    //u32 reg_gain = 0, dcg = 1024, DIG_gain = 1, DIG_Fine_gain_reg = 0;
    //u16 data1 = 0, data2 = 0;
    u64 maxExp = (u64)(vts_30fps - 4);

    I2cCfg_t exp_reg[] = {
        {0x3e00, 0x00},
        {0x3e01, 0x00},
        {0x3e02, 0x00},
    };

    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e09, 0x00},
    };

    _u16 I2cId = gHwRes[pHandle->uSensorPadId]->eI2cPort ;//HwRes->eI2cPort;
    _u16 slave_addr = 0x60;
    EARLYINIT_I2C_FMT fmt = I2C_FMT_A16D8;
    //small window AE Calculate
    while(i < 50) //50ms
    {
        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x5e32, &ae_stable, fmt);
        if(ae_stable == 1)
        {
            break;
        }
        else
        {
            CamOsPrintf("wait sensor 1ms small window AE Calculate\n");
            CamOsUsDelay(1);
            i++;
        }
    }

    CamOsPrintf("ae_stable status %d!!!!!!!!!!!!!!\n", ae_stable);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x5e20, &sValue, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x5e03, &L_target, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x3e00, &exp_reg[0].u16Data, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x3e01, &exp_reg[1].u16Data, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x3e02, &exp_reg[2].u16Data, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x3e08, &gain_reg[0].u16Data, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x3e09, &gain_reg[1].u16Data, fmt);

    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x5e24, &awb_rGain, fmt);
    earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x5e25, &awb_bGain, fmt);

    CamOsPrintf("sValue=%d;L_target=%d;exp_reg[0].data=%d;exp_reg[1].data=%d;exp_reg[2].data=%d;gain_reg[0].data=%d;gain_reg[1].data=%d;\n",
        sValue,
        L_target,
        exp_reg[0].u16Data,
        exp_reg[1].u16Data,
        exp_reg[2].u16Data,
        gain_reg[0].u16Data,
        gain_reg[1].u16Data);

    CamOsPrintf("reg awb_rGain=%d, awb_bGain=%d\n", awb_rGain, awb_bGain);

    awb_rGain_isp = awb_rGain *1024 / 128;
    awb_bGain_isp = awb_bGain *1024 / 128;

    CamOsPrintf("awb_rGain_isp=%d, awb_bGain_isp=%d\n", awb_rGain_isp, awb_bGain_isp);

    //isp open ob, avgY= raw_avgY * ob_gain
    //sensor ob offset = 16
    //raw_avgY=44
    //ob_gain=255/(255-ob)
    //isp avgY = 44 * 255/(255-16) = 47
    L_target = 44;

    sExp = ((exp_reg[0].u16Data << 16) + (exp_reg[1].u16Data << 8) + exp_reg[2].u16Data) >> 4;
    sGain = (gain_reg[0].u16Data << 8) + gain_reg[1].u16Data;

    CamOsPrintf("L_target=%d;sExp=%lld;sGain=%lld\n",L_target, sExp, sGain);
    Big_Exp = L_target * sExp * sRowTime * sGain / sValue / 0x10 / bRowTime * 6 / 5;
    CamOsPrintf("Big_Exp=%lld\n",Big_Exp);
    bExp = (Big_Exp > maxExp) ? maxExp : Big_Exp;
    CamOsPrintf("bExp=%lld;maxExp=%lld\n",bExp,maxExp);
    bGain = Big_Exp * 1024 / bExp;
    CamOsPrintf("bGain=%lld\n",bGain);

    *pShutter = sRowTime*bExp/1000;
    *pGain    = bGain;
    CamOsPrintf("pShutter=%lld, pGain=%lld\n",*pShutter, *pGain);

    return 0;
}



unsigned int SC301IOTInitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = SC301IOTEarlyInitGainEx;
    pHandle->fpShutterFpsParserEx = SC301IOTEarlyInitShutterAndFpsEx;
    pHandle->fpSelResId           = SC301IOTEarlyInitSelResId;
    pHandle->fpOrientParerEx      = SC301IOTEarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= SC301IOTEarlyInitSelStreamOnoff;
    pHandle->fpFastAeParseEx      = SC301IOTEarlyInitFastAe;
    gHwRes[eSnrId]                = &pHandle->tHwRes;

    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gSC301IOTInfo_4M30, sizeof(_gSC301IOTInfo_4M30));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;
    return 0;
}

SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( SC301iot, SC301IOTInitHandle);
