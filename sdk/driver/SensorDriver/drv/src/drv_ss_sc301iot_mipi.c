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
/* Sensor Porting on Master V4
   Porting owner : eddie.liang
   Date : 2023/9/25
   Build on : Master V4  I6C
   Verified on : not yet。
   Remark : NA
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>
#if defined(CAM_OS_RTK)
#include <stdlib.h>
#endif
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC301iot);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE  CAM_OS_ARRAY_SIZE
#endif

//============================================
//
//    SENSOR STATUS
//
//============================================
//MIPI config begin.
#undef SENSOR_NAME
#define SENSOR_NAME                           SC301iot

#define SENSOR_IFBUS_TYPE                     CUS_SENIF_BUS_MIPI
//Linear Mode
#define SENSOR_MIPI_LANE_NUM                  (2)
#define Preview_MCLK_SPEED                    CUS_CMU_CLK_27MHZ


//I2C bus
#define SENSOR_I2C_ADDR                       0x60
#define SENSOR_I2C_SPEED                      20000
#define SENSOR_I2C_LEGACY                     I2C_NORMAL_MODE
#define SENSOR_I2C_FMT                        I2C_FMT_A16D8

#define SENSOR_RGBIRID                        CUS_RGBIR_NONE

#define SENSOR_DATAPREC                       CUS_DATAPRECISION_10
#define SENSOR_BAYERID                        CUS_BAYER_BG

//AE info
#define SENSOR_MAX_GAIN                       (50208*31500/1000)
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define SENSOR_ISP_TYPE          ISP_EXT                 //ISP_EXT, ISP_SOC
#define SENSOR_IFBUS_TYPE        CUS_SENIF_BUS_MIPI     //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_HSYNC_MODE   PACKET_HEADER_EDGE1
#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC          CUS_DATAPRECISION_10    //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR      CUS_DATAPRECISION_10
#define SENSOR_DATAMODE          CUS_SEN_10TO12_9000
#define SENSOR_BAYERID           CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR       CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID           CUS_RGBIR_NONE
#define SENSOR_ORIT              CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_MAXGAIN           (50208*31500/1000)      // max sensor gain, a-gain*conversion-gain*d-gain
#define Preview_MCLK_SPEED       CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR   CUS_CMU_CLK_27MHZ

#define SENSOR_PWDN_POL          CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL           CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL         CUS_CLK_POL_NEG            // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL         CUS_CLK_POL_POS         // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL          CUS_CLK_POL_POS         // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#define Preview_MIN_FPS          5                       //slowest preview FPS

#define SENSOR_MIPI_LANE_NUM     (2)
#define SENSOR_MIPI_LANE_NUM_HDR    (2)

u32 vts_30fps_HDR = 3200;

u32 Preview_line_period = 20833;
u32 vts_30fps = 1600;
u32 Preview_line_period_HDR = 10417;

//============================================
//
//    SENSOR Resolution List
//
//============================================

static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
}mipi_linear[] = {
    {LINEAR_RES_1, {2048, 1536, 3, 30}, {0, 0, 2048, 1536}, {"2048x1536@30fps"}},
    {LINEAR_RES_2, {2048, 1536, 3, 60}, {0, 0, 2048, 1536}, {"2048X1536@60fps"}},
};
static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_END}mode;
    // Sensor Output Image info
    struct _hsenout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _hsensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _hsenstr{
        const char* strResDesc;
    }senstr;
}sc301iot_mipi_hdr[] = {
   {HDR_RES_1, {2048, 1536, 3, 30}, {0, 0, 2048, 1536}, {"2048x1536@30fps_HDR"}}, // Modify it
};
/*
typedef struct {
    struct {
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
        u32 vts;
        u32 final_gain;
        u32 fps;
        u32 preview_fps;
        u32 line;
        u32 max_short_exp;
    } expo;

    u32 skip_cnt;
    CUS_CAMSENSOR_ORIT  orit;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    bool orien_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
    bool dirty;
    bool change;
    int pre_ae_shutter;
    int pre_ae_gain;
    int pre_ae_rgain;
    int pre_ae_bgain;
    CUS_CAMSENSOR_ORIT cur_orien;
} SC301iot_params;
*/

typedef struct {
    struct {
        u16 pre_div0;
        u16 div124;
        u16 div_cnt7b;
        u16 sdiv0;
        u16 mipi_div0;
        u16 r_divp;
        u16 sdiv1;
        u16 r_seld5;
        u16 r_sclk_dac;
        u16 sys_sel;
        u16 pdac_sel;
        u16 adac_sel;
        u16 pre_div_sp;
        u16 r_div_sp;
        u16 div_cnt5b;
        u16 sdiv_sp;
        u16 div12_sp;
        u16 mipi_lane_sel;
        u16 div_dac;
    } clk_tree;
    struct {
        u32 sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 max_short_exp;
        u32 line;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tGain_reg_HDR_SEF[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_reg_HDR_SEF[2];
    I2C_ARRAY tMirror_reg[1];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orien_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
    bool change;
    CUS_CAMSENSOR_ORIT cur_orien;
    int pre_ae_shutter;
    int pre_ae_gain;
    int pre_ae_rgain;
    int pre_ae_bgain;
} SC301iot_params;

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)     (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)     (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
static u32 vts_60fps = 0;
#if 0
// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xce},
    {0x3108, 0x39},
};
#endif

const static I2C_ARRAY Sensor_init_table_3M30fps[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x301c,0x78},
    {0x301f,0x01},
    {0x30b8,0x44},
    {0x3208,0x08},
    {0x3209,0x00},
    {0x320a,0x06},
    {0x320b,0x00},
    {0x320c,0x04},
    {0x320d,0x65},
    {0x320e,0x06},
    {0x320f,0x40},
    {0x3214,0x11},
    {0x3215,0x11},
    {0x3223,0xc0}, //wait BLC done //c0 nodelay //d0 delay1frame
    {0x3028,0x04},//wait BLC done min~max  //1~0x37
    {0x3231,0x01},
    {0x3253,0x0c},
    {0x3274,0x09},
    {0x3301,0x08},
    {0x3306,0x58},
    {0x3308,0x08},
    {0x330a,0x00},
    {0x330b,0xe0},
    {0x330e,0x10},
    {0x3314,0x14},
    {0x331e,0x55},
    {0x331f,0x7d},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x5e},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x3390,0x01},
    {0x3391,0x03},
    {0x3392,0x07},
    {0x3393,0x08},
    {0x3394,0x08},
    {0x3395,0x08},
    {0x3396,0x08},
    {0x3397,0x09},
    {0x3398,0x1f},
    {0x3399,0x08},
    {0x339a,0x0a},
    {0x339b,0x40},
    {0x339c,0x88},
    {0x33a2,0x04},
    {0x33ad,0x0c},
    {0x33b1,0x80},
    {0x33b3,0x30},
    {0x33f9,0x68},
    {0x33fb,0x80},
    {0x33fc,0x48},
    {0x33fd,0x5f},
    {0x349f,0x03},
    {0x34a6,0x48},
    {0x34a7,0x5f},
    {0x34a8,0x30},
    {0x34a9,0x30},
    {0x34aa,0x00},
    {0x34ab,0xf0},
    {0x34ac,0x01},
    {0x34ad,0x08},
    {0x34f8,0x5f},
    {0x34f9,0x10},
    {0x3630,0xf0},
    {0x3631,0x85},
    {0x3632,0x74},
    {0x3633,0x22},
    {0x3637,0x4d},
    {0x3638,0xcb},
    {0x363a,0x8b},
    {0x363b,0x02},
    {0x363c,0x08},
    {0x3640,0x00},
    {0x3641,0x38},
    {0x3670,0x4e},
    {0x3674,0xc0},
    {0x3675,0xb0},
    {0x3676,0xa0},
    {0x3677,0x85},
    {0x3678,0x87},
    {0x3679,0x8a},
    {0x367c,0x49},
    {0x367d,0x4f},
    {0x367e,0x48},
    {0x367f,0x4b},
    {0x3690,0x33},
    {0x3691,0x33},
    {0x3692,0x44},
    {0x3699,0x8a},
    {0x369a,0xa1},
    {0x369b,0xc2},
    {0x369c,0x48},
    {0x369d,0x4f},
    {0x36a2,0x4b},
    {0x36a3,0x4f},
    {0x370f,0x01},
    {0x3714,0x80},
    {0x3722,0x09},
    {0x3724,0x41},
    {0x3725,0xc1},
    {0x3728,0x00},
    {0x3771,0x09},
    {0x3772,0x05},
    {0x3773,0x05},
    {0x377a,0x48},
    {0x377b,0x49},
    {0x3905,0x8d},
    //{0x3907,0x00}, //blc
    //{0x3908,0x41}, //blc
    {0x391d,0x08},
    {0x3922,0x1a},
    {0x3926,0x21},
    {0x3933,0x80},
    {0x3934,0x0d},
    {0x3937,0x6a},
    {0x3939,0x00},
    {0x393a,0x0e},
    {0x39dc,0x02},
    //{0x3e00,0x00},
    //{0x3e01,0x63},
    //{0x3e02,0xc0},
    {0x3e03,0x0b},
    {0x3e1b,0x2a},
    {0x4407,0x34},
    {0x440e,0x02},
    {0x5001,0x40},
    {0x5007,0x80},
    {0x36e9,0x24},
    {0x37f9,0x24},
    //{0x0100,0x01},
    //{0xffff,0x0a}, //mdelay(10),
};

const static I2C_ARRAY Sensor_init_table_3M60fps[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x301c,0x78},
    {0x301f,0x02},
    {0x30b8,0x44},
    {0x3208,0x08},
    {0x3209,0x00},
    {0x320a,0x06},
    {0x320b,0x00},
    {0x320c,0x04},
    {0x320d,0x65},
    {0x320e,0x06},
    {0x320f,0x40},
    {0x3214,0x11},
    {0x3215,0x11},
    {0x3223,0xc0}, //wait BLC done //c0 nodelay //d0 delay1frame
    {0x3028,0x04},//wait BLC done min~max  //1~0x37
    {0x3231,0x01},
    {0x3253,0x0c},
    {0x3274,0x09},
    {0x3301,0x08},
    {0x3304,0x80},
    {0x3306,0x58},
    {0x3308,0x08},
    {0x3309,0xa0},
    {0x330a,0x00},
    {0x330b,0xe0},
    {0x330e,0x10},
    {0x3314,0x14},
    {0x331e,0x71},
    {0x331f,0x91},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x5e},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x3390,0x01},
    {0x3391,0x03},
    {0x3392,0x07},
    {0x3393,0x08},
    {0x3394,0x08},
    {0x3395,0x08},
    {0x3396,0x08},
    {0x3397,0x09},
    {0x3398,0x1f},
    {0x3399,0x08},
    {0x339a,0x14},
    {0x339b,0x28},
    {0x339c,0x78},
    {0x33a2,0x04},
    {0x33ad,0x0c},
    {0x33b1,0x80},
    {0x33b3,0x38},
    {0x33f9,0x58},
    {0x33fb,0x80},
    {0x33fc,0x48},
    {0x33fd,0x4f},
    {0x349f,0x03},
    {0x34a6,0x48},
    {0x34a7,0x4f},
    {0x34a8,0x38},
    {0x34a9,0x28},
    {0x34aa,0x00},
    {0x34ab,0xe0},
    {0x34ac,0x01},
    {0x34ad,0x08},
    {0x34f8,0x5f},
    {0x34f9,0x18},
    {0x3630,0xf0},
    {0x3631,0x85},
    {0x3632,0x74},
    {0x3633,0x22},
    {0x3637,0x4d},
    {0x3638,0xcb},
    {0x363a,0x8b},
    {0x363c,0x08},
    {0x3641,0x38},
    {0x3670,0x4e},
    {0x3674,0xc0},
    {0x3675,0xa0},
    {0x3676,0x90},
    {0x3677,0x83},
    {0x3678,0x86},
    {0x3679,0x89},
    {0x367c,0x48},
    {0x367d,0x4f},
    {0x367e,0x48},
    {0x367f,0x4b},
    {0x3690,0x33},
    {0x3691,0x44},
    {0x3692,0x55},
    {0x3699,0x8a},
    {0x369a,0xa1},
    {0x369b,0xc2},
    {0x369c,0x48},
    {0x369d,0x4f},
    {0x36a2,0x4b},
    {0x36a3,0x4f},
    {0x36ea,0x0a},
    {0x36eb,0x0c},
    {0x36ec,0x0c},
    {0x36ed,0x15},
    {0x370f,0x01},
    {0x3714,0x80},
    {0x3722,0x01},
    {0x3724,0x41},
    {0x3725,0xc1},
    {0x3728,0x00},
    {0x3771,0x09},
    {0x3772,0x09},
    {0x3773,0x05},
    {0x377a,0x48},
    {0x377b,0x4f},
    {0x37fa,0x08},
    {0x37fb,0x31},
    {0x37fc,0x10},
    {0x37fd,0x18},
    {0x3905,0x8d},
    {0x391d,0x08},
    {0x3922,0x1a},
    {0x3926,0x21},
    {0x3933,0x80},
    {0x3934,0x0d},
    {0x3937,0x6a},
    {0x3939,0x00},
    {0x393a,0x0e},
    {0x39dc,0x02},
    {0x3e00,0x00},
    {0x3e01,0x63},
    {0x3e02,0x80},
    {0x3e03,0x0b},
    {0x3e1b,0x2a},
    {0x4407,0x34},
    {0x440e,0x02},
    {0x4509,0x10},
    {0x5001,0x40},
    {0x5007,0x80},
    {0x36e9,0x24},
    {0x37f9,0x24},
    {0x0100,0x01},
};

const static I2C_ARRAY Sensor_init_table_HDR_DOL_2lane[] =
{
    //cleaned_0x07_SC301IOT_MIPI_27Minput_2lane_10bit_1080Mbps_2048x1536_30fpsSHDR
	{0x0103,0x01},
	{0x0100,0x00},
	{0x36e9,0x80},
	{0x37f9,0x80},
	{0x301c,0x78},
	{0x301f,0x07},
	{0x30b8,0x44},
	{0x3208,0x08},
	{0x3209,0x00},
	{0x320a,0x06},
	{0x320b,0x00},
	{0x320c,0x04},
	{0x320d,0x65},
	{0x320e,0x0c},
	{0x320f,0x80},
	{0x3214,0x11},
	{0x3215,0x11},
	{0x3223,0xd0},
	{0x3231,0x01},
	{0x3250,0xff},
	{0x3253,0x0c},
	{0x3274,0x09},
	{0x3281,0x01},
	{0x3301,0x08},
	{0x3304,0x80},
	{0x3306,0x58},
	{0x3308,0x08},
	{0x3309,0xa0},
	{0x330a,0x00},
	{0x330b,0xe0},
	{0x330e,0x10},
	{0x3314,0x14},
	{0x331e,0x71},
	{0x331f,0x91},
	{0x3333,0x10},
	{0x3334,0x40},
	{0x335e,0x06},
	{0x335f,0x08},
	{0x3364,0x5e},
	{0x337c,0x02},
	{0x337d,0x0a},
	{0x3390,0x01},
	{0x3391,0x03},
	{0x3392,0x07},
	{0x3393,0x08},
	{0x3394,0x08},
	{0x3395,0x08},
	{0x3396,0x08},
	{0x3397,0x09},
	{0x3398,0x1f},
	{0x3399,0x08},
	{0x339a,0x14},
	{0x339b,0x28},
	{0x339c,0x78},
	{0x33a2,0x04},
	{0x33ad,0x0c},
	{0x33b1,0x80},
	{0x33b3,0x38},
	{0x33f9,0x58},
	{0x33fb,0x80},
	{0x33fc,0x48},
	{0x33fd,0x4f},
	{0x349f,0x03},
	{0x34a6,0x48},
	{0x34a7,0x4f},
	{0x34a8,0x38},
	{0x34a9,0x28},
	{0x34aa,0x00},
	{0x34ab,0xe0},
	{0x34ac,0x01},
	{0x34ad,0x08},
	{0x34f8,0x5f},
	{0x34f9,0x18},
	{0x3630,0xf0},
	{0x3631,0x85},
	{0x3632,0x74},
	{0x3633,0x22},
	{0x3637,0x4d},
	{0x3638,0xcb},
	{0x363a,0x8b},
	{0x363b,0x02},//add 20231222 for danny
	{0x363c,0x08},
	{0x3641,0x38},
	{0x3670,0x4e},
	{0x3674,0xc0},
	{0x3675,0xa0},
	{0x3676,0x90},
	{0x3677,0x85},
	{0x3678,0x86},
	{0x3679,0x89},
	{0x367c,0x48},
	{0x367d,0x4f},
	{0x367e,0x48},
	{0x367f,0x4b},
	{0x3690,0x33},
	{0x3691,0x44},
	{0x3692,0x55},
	{0x3699,0x8a},
	{0x369a,0xa1},
	{0x369b,0xc2},
	{0x369c,0x48},
	{0x369d,0x4f},
	{0x36a2,0x4b},
	{0x36a3,0x4f},
	{0x36ea,0x0a},
	{0x36eb,0x0c},
	{0x36ec,0x0c},
	{0x36ed,0x15},
	{0x370f,0x01},
	{0x3714,0x80},
	{0x3722,0x01},
	{0x3724,0x41},
	{0x3725,0xc1},
	{0x3728,0x00},
	{0x3771,0x09},
	{0x3772,0x09},
	{0x3773,0x05},
	{0x377a,0x48},
	{0x377b,0x4f},
	{0x37fa,0x08},
	{0x37fb,0x31},
	{0x37fc,0x10},
	{0x37fd,0x18},
	{0x3905,0x8d},
	{0x391d,0x08},
	{0x3922,0x1a},
	{0x3926,0x21},
	{0x3933,0x80},
	{0x3934,0x0d},
	{0x3937,0x6a},
	{0x3939,0x00},
	{0x393a,0x0e},
	{0x39dc,0x02},
	{0x3e00,0x00},
	{0x3e01,0xb9},
	{0x3e02,0xc0},
	{0x3e03,0x0b},
	{0x3e04,0x0b},
	{0x3e05,0xa0},
	{0x3e1b,0x2a},
	{0x3e23,0x00},
	{0x3e24,0xbf},
	{0x4407,0x34},
	{0x440e,0x02},
	{0x4509,0x10},
	{0x4816,0x71},
	{0x4825,0x05},//Tlpx
	{0x4827,0x07},//Tclk-prepare
	{0x5001,0x40},
	{0x5007,0x80},
	{0x36e9,0x24},
	{0x37f9,0x24},
	{0x0100,0x01},
    {0xffff,0x0a},
};

const static I2C_ARRAY mirror_reg[] =
{
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};

const static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0xf0},
    {0x3e09, 0x40}, //low bit, 0x40 - 0x7f, step 1/64
};

const static I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x63}, // expo[16:8]
    {0x3e02, 0x40}, // expo[7:0], [3:0] fraction of line
};

const static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x04},
    {0x320f, 0x65},
};

const static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3e10, 0x00},
    {0x3e11, 0x00},
    {0x3e13, 0x40}, //low bit, 0x40 - 0x7f, step 1/64
};
const static I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3e04, 0x00}, // expo[16:8]
    {0x3e05, 0x80}, // expo[7:0], [3:0] fraction of line
};


static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us);
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain);
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);


/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period + 999;
    info->u32AEShutter_step                  = Preview_line_period ;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo_HDR_SEF(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = 2;
    info->u32AEShutter_step                  = 2;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo_HDR_LEF(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = 2;
    info->u32AEShutter_step                  = 2;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
/////////////////// sensor hardware dependent //////////////
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    s32           res       = 0;

    SENSOR_DMSG("[%s] ", __FUNCTION__);
    //ISP_config_io(handle);
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    res = sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    if (res >= 0)
    {
        SENSOR_USLEEP(1000);
    }

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    res = sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    if (res >= 0)
    {
        SENSOR_USLEEP(1000);
    }

    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    res = sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    if (res >= 0)
    {
        SENSOR_MSLEEP(1);
    }

    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    res = sensor_if->Reset(idx, CUS_CLK_POL_POS);
    if (res >= 0)
    {
        SENSOR_MSLEEP(5);
    }

    return SUCCESS;
}


static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;

    // power/reset low
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_MSLEEP(1);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);

    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data;
    SENSOR_DMSG("[%s] mirror:%x\r\n", __FUNCTION__, sen_data & 0x66);
    switch(sen_data & 0x66)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
            break;
        case 0x06:
            *orit = CUS_ORIT_M1F0;
            break;
        case 0x60:
            *orit = CUS_ORIT_M0F1;
            break;
        case 0x66:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_StreamOn(struct __ss_cus_sensor *handle, u32 idx)
{
    I2C_ARRAY stream_onoff_reg = {0x0100, 0x00};

    stream_onoff_reg.data= 0x01;//disable sleep mode
    SensorReg_Write(stream_onoff_reg.reg, stream_onoff_reg.data);

    SENSOR_EMSG("[%s] sensor stream on!\n", __FUNCTION__);
    return SUCCESS;
}


static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;

    switch(orit) {
    case CUS_ORIT_M0F0:
        params->tMirror_reg[0].data = 0;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F0:
        params->tMirror_reg[0].data = 6;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M0F1:
        params->tMirror_reg[0].data = 0x60;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F1:
        params->tMirror_reg[0].data = 0x66;
        params->orien_dirty = true;
        break;
    default :
        break;
    }

    params->cur_orien = orit;
    return SUCCESS;
}


/////////////////// image function /////////////////////////

static int SC301iot_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

  return SUCCESS;
}


static int Pre_ae(ss_cus_sensor* handle)
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
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u64 Big_Exp = 0;
    u16 ae_stable = 0, L_target = 0, sValue = 0;
    u64 sExp = 0, sGain =  0, bExp = 0, bGain = 0;
    u64 sRowTime = 20833;
    u64 bRowTime = Preview_line_period;
    u8 i=0 ;
    u16 awb_rGain = 0, awb_bGain = 0;
    u16 awb_rGain_isp = 0, awb_bGain_isp = 0;
    //u32 reg_gain = 0, dcg = 1024, DIG_gain = 1, DIG_Fine_gain_reg = 0;
    u64 maxExp = (u64)(vts_30fps - 4);
    u16 r_value = 0,  g_value = 0,  b_value = 0;

    I2C_ARRAY exp_reg[] = {
        {0x3e00, 0x00},
        {0x3e01, 0x00},
        {0x3e02, 0x00},
    };

    I2C_ARRAY gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e09, 0x00},
    };

    params->pre_ae_shutter = 0;
    params->pre_ae_gain    = 0;
    params->pre_ae_rgain   = 0;
    params->pre_ae_bgain   = 0;

    //small window AE Calculate
    while(i < 50) //50ms
    {
        SensorReg_Read(0x5e32, &ae_stable);
        if(ae_stable == 1)
        {
            break;
        }
        else
        {
            CamOsPrintf("SENSOR_MSLEEP 1ms\n");
            SENSOR_MSLEEP(1);
            i++;
        }
    }

    CamOsPrintf("ae_stable status %d!!!!!!!!!!!!!!\n", ae_stable);
    SensorReg_Read(0x5e20, &sValue);
    SensorReg_Read(0x5e03, &L_target);
    SensorReg_Read(0x3e00, &exp_reg[0].data);
    SensorReg_Read(0x3e01, &exp_reg[1].data);
    SensorReg_Read(0x3e02, &exp_reg[2].data);
    SensorReg_Read(0x3e08, &gain_reg[0].data);
    SensorReg_Read(0x3e09, &gain_reg[1].data);
    SensorReg_Read(0x5e24, &awb_rGain);
    SensorReg_Read(0x5e25, &awb_bGain);

    SensorReg_Read(0x5e21, &b_value);
    SensorReg_Read(0x5e22, &g_value);
    SensorReg_Read(0x5e23, &r_value);

    CamOsPrintf(">>>r_value %d, g_value %d, b_value %d\n", r_value, g_value, b_value);

    CamOsPrintf("sValue=%d;L_target=%d;exp_reg[0].data=%d;exp_reg[1].data=%d;exp_reg[2].data=%d;gain_reg[0].data=%d;gain_reg[1].data=%d;\n",
    sValue,
    L_target,
    exp_reg[0].data,
    exp_reg[1].data,
    exp_reg[2].data,
    gain_reg[0].data,
    gain_reg[1].data);

    if(sValue == 0)
    {
        CamOsPrintf("sValue is 0,error>>>>>>\n");
        return -1;
    }


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

    sExp = ((exp_reg[0].data << 16) + (exp_reg[1].data << 8) + exp_reg[2].data) >> 4;
    sGain = (gain_reg[0].data << 8) + gain_reg[1].data;

    CamOsPrintf("L_target=%d;sExp=%lld;sGain=%lld\n",L_target,sExp, sGain);
    Big_Exp = L_target * sExp * sRowTime * sGain / sValue / 0x10 / bRowTime * 6 / 5;
    CamOsPrintf("Big_Exp=%lld>>>>>>\n",Big_Exp);

    bExp = (Big_Exp > maxExp) ? maxExp : Big_Exp;
    CamOsPrintf("bExp=%lld;maxExp=%lld\n",bExp,maxExp);
    bGain = Big_Exp * 1024 / bExp;
    CamOsPrintf("bGain=%lld\n",bGain);

    params->pre_ae_shutter = sRowTime*bExp/1000;
    params->pre_ae_gain    = bGain;
    params->pre_ae_rgain   = awb_rGain_isp;
    params->pre_ae_bgain   = awb_bGain_isp;

    return SUCCESS;
}


static int pCus_init_linear_3M30fps(ss_cus_sensor *handle)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    Pre_ae(handle);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_3M30fps);i++)
    {
        if(Sensor_init_table_3M30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_3M30fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_3M30fps[i].reg, Sensor_init_table_3M30fps[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;

    CamOsPrintf("pre_ae_shutter=%d, pre_ae_gain=%d\n", params->pre_ae_shutter, params->pre_ae_gain);
    if(params->pre_ae_shutter)
    {
        pCus_SetAEUSecs(handle, params->pre_ae_shutter);
    }
    else
    {
        pCus_SetAEUSecs(handle, 10000);
    }

    if(params->pre_ae_gain)
    {
        pCus_SetAEGain(handle, params->pre_ae_gain);
    }
    else
    {
        pCus_SetAEGain(handle, 5035);
    }

    //pCus_AEStatusNotify(handle, 0, CUS_FRAME_ACTIVE);
    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
    params->reg_dirty = false;

    CamOsPrintf("[%s]>>>>>>>>>>notify write exp [0x%x,0x%x,0x%x], gain [0x%x,0x%x,0x%x]\n ",__FUNCTION__,
            params->tExpo_reg[0].data, params->tExpo_reg[1].data, params->tExpo_reg[2].data,
            params->tGain_reg[0].data, params->tGain_reg[1].data, params->tGain_reg[2].data);

    SensorReg_Write(0x0100, 0x01);

#if 1 //for debug
    {
        I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
                {0x3e00, 0x00}, //expo [20:17]
                {0x3e01, 0x00}, // expo[16:8]
                {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
            };

        I2C_ARRAY gain_reg_temp[] = {
            {0x3e06, 0x00},
            {0x3e07, 0x80},
            {0x3e09, 0x00},
        };
        u32  us = 0, lines = 0;

        SensorReg_Read(expo_reg_temp[0].reg, &expo_reg_temp[0].data);
        SensorReg_Read(expo_reg_temp[1].reg, &expo_reg_temp[1].data);
        SensorReg_Read(expo_reg_temp[2].reg, &expo_reg_temp[2].data);
        SensorReg_Read(gain_reg_temp[0].reg, &gain_reg_temp[0].data);
        SensorReg_Read(gain_reg_temp[1].reg, &gain_reg_temp[1].data);
        SensorReg_Read(gain_reg_temp[2].reg, &gain_reg_temp[2].data);

        lines |= (u32)(expo_reg_temp[0].data&0x0f)<<16;
        lines |= (u32)(expo_reg_temp[1].data&0xff)<<8;
        lines |= (u32)(expo_reg_temp[2].data&0xf0)<<0;
        lines >>= 4;
        us = (lines*Preview_line_period)/1000; //return us

        CamOsPrintf(">>>>>>>>>>read exp [0x%x,0x%x,0x%x]\n ",
            expo_reg_temp[0].data, expo_reg_temp[1].data, expo_reg_temp[2].data);

        CamOsPrintf(">>>>>>>>>>read lines %d, us %d, gain [0x%x,0x%x,0x%x]\n", lines,
                    us, gain_reg_temp[0].data, gain_reg_temp[1].data, gain_reg_temp[2].data);

    }
#endif

    return SUCCESS;
}

static int pCus_init_linear_3M60fps(ss_cus_sensor *handle)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_3M60fps);i++)
    {
        if(Sensor_init_table_3M60fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_3M60fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_3M60fps[i].reg, Sensor_init_table_3M60fps[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

static int pCus_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int pCus_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"2048x1536@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_3M30fps;
            vts_30fps = 1600;//1500
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period  = 20833;
            handle->mclk = CUS_CMU_CLK_27MHZ;
            break;
        case 1: //"2048x1536@60fps"
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_linear_3M60fps;
            vts_30fps = 1600;
            params->expo.vts = vts_30fps;
            params->expo.fps = 60;
            Preview_line_period = 10417;
            handle->mclk = CUS_CMU_CLK_27MHZ;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts=0;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u16 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.line >  (params->expo.vts) -8){
        vts = (params->expo.line +8);
    }else{
        vts = params->expo.vts;
    }
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        break;
        case CUS_FRAME_ACTIVE:
        if(params->orien_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->orien_dirty = false;
        }
        if(params->reg_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            params->reg_dirty = false;

           // CamOsPrintf("[%s]>>>>>>>>>>notify write exp [0x%x,0x%x,0x%x], gain [0x%x,0x%x,0x%x]\n ",__FUNCTION__,
           //         params->tExpo_reg[0].data, params->tExpo_reg[1].data, params->tExpo_reg[2].data,
           //         params->tGain_reg[0].data, params->tGain_reg[1].data, params->tGain_reg[2].data);

#if 0 //for debug
            {
                I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
                        {0x3e00, 0x00}, //expo [20:17]
                        {0x3e01, 0x00}, // expo[16:8]
                        {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
                    };

                I2C_ARRAY gain_reg_temp[] = {
                    {0x3e06, 0x00},
                    {0x3e07, 0x80},
                    {0x3e09, 0x00},
                };
                u32  us = 0, lines = 0;

                SensorReg_Read(expo_reg_temp[0].reg, &expo_reg_temp[0].data);
                SensorReg_Read(expo_reg_temp[1].reg, &expo_reg_temp[1].data);
                SensorReg_Read(expo_reg_temp[2].reg, &expo_reg_temp[2].data);
                SensorReg_Read(gain_reg_temp[0].reg, &gain_reg_temp[0].data);
                SensorReg_Read(gain_reg_temp[1].reg, &gain_reg_temp[1].data);
                SensorReg_Read(gain_reg_temp[2].reg, &gain_reg_temp[2].data);

                lines |= (u32)(expo_reg_temp[0].data&0x0f)<<16;
                lines |= (u32)(expo_reg_temp[1].data&0xff)<<8;
                lines |= (u32)(expo_reg_temp[2].data&0xf0)<<0;
                lines >>= 4;
                us = (lines*Preview_line_period)/1000; //return us


                CamOsPrintf(">>>>>>>>>>notify write exp [0x%x,0x%x,0x%x], gain [0x%x,0x%x,0x%x]\n ",
                            params->tExpo_reg[0].data, params->tExpo_reg[1].data, params->tExpo_reg[2].data,
                            params->tGain_reg[0].data, params->tGain_reg[1].data, params->tGain_reg[2].data);
                CamOsPrintf(">>>>>>>>>>notify read lines %d, us %d, gain [0x%x,0x%x,0x%x]\n", lines,
                            us, gain_reg_temp[0].data, gain_reg_temp[1].data, gain_reg_temp[2].data);

            }
#endif


        }
        break;
        default :
        break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc=0;
    u32 lines = 0;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period)/1000; //return us

  SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
  return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    int i;
    u32  lines = 0, vts = 0;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
        {0x3e00, 0x00}, //expo [20:17]
        {0x3e01, 0x00}, // expo[16:8]
        {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };

    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg_temp));

    lines = (1000*us)/Preview_line_period; // Preview_line_period in ns

    if(lines<=2) lines=2;
    if (lines > (params->expo.vts)-8) {
        vts = (lines+8);
    }
    else
        vts=params->expo.vts;
    params->expo.line = lines;

    SENSOR_DMSG("[%s] us %d, lines %d, vts %d\n", __FUNCTION__, us, lines, params->expo.vts);

    lines = lines<<4;
    params->tExpo_reg[0].data = (lines>>16) & 0x0f;
    params->tExpo_reg[1].data = (lines>>8) & 0xff;
    params->tExpo_reg[2].data = (lines>>0) & 0xf0;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    for (i = 0; i < ARRAY_SIZE(expo_reg); i++)
    {
      if (params->tExpo_reg[i].data != expo_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
     }
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int rc = 0;

    return rc;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u8 i=0 ;
    u32 reg_gain = 0, dcg = 1024, DIG_gain = 1, DIG_Fine_gain_reg = 0;
    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e09, 0x00},
    };

    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if((gain >= 1024)&&(gain < 1607)) // x1.569
    {
        dcg = 1024;
        reg_gain = 0x00;
        DIG_gain = 1;
    }
    else if((gain >= 1607)&&(gain < 3214)) // x1.569~ x3.138
    {
        dcg = 1607;
        reg_gain = 0x40;
        DIG_gain = 1;
    }
    else if((gain >= 3214)&&(gain < 6428)) // x3.138 ~ x6.276
    {
        dcg = 3214;
        reg_gain = 0x48;
        DIG_gain = 1;
    }
    else if((gain >= 6428)&&(gain < 12856)) // x6.276 ~ x12.552
    {
        dcg = 6428;
        reg_gain = 0x49;
        DIG_gain = 1;
    }
    else if((gain >= 12856)&&(gain < 25712)) // x12.552 ~ x25.104
    {
        dcg = 12856;
        reg_gain = 0x4b;
        DIG_gain = 1;
    }
    else if((gain >= 25712)&&(gain < 51424)) // x25.104 ~ x50.208
    {
        dcg = 25712;
        reg_gain = 0x4f;
        DIG_gain = 1;
    }
    else if((gain >= 51424)&&(gain < 102848)) // x50.208 ~ x50.208*2
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 1;
    }
    else if((gain >= 102848)&&(gain < 205696)) //  x50.208*2 ~ x50.208*4
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 2;
    }
    else if((gain >= 205696)&&(gain < 411392)) // x50.208*4 ~ x50.208*8
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 4;
    }
    else if((gain >= 411392)&&(gain < (411392*2))) // x50.208*8 ~ x50.208*16
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 8;
    }
    else if((gain >= (411392*2))&&(gain < 1619509)) // x50.208*16 ~ x50.208*31.5
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = ( 8 * 2 );
    }
    DIG_Fine_gain_reg = (u8)((128*gain/dcg/DIG_gain/4)*4);
    params->tGain_reg[2].data = reg_gain;
    params->tGain_reg[1].data = DIG_Fine_gain_reg;
    params->tGain_reg[0].data = DIG_gain-1;
    for (i = 0; i < ARRAY_SIZE(gain_reg); i++)
    {
      if (params->tGain_reg[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    SENSOR_DMSG("[%s] reg_gain 0x%x, dcg %d, DIG_gain %d\n", __FUNCTION__, reg_gain, dcg, DIG_gain);


    return SUCCESS;
}

#define CMDID_I2C_READ   (0x01)
#define CMDID_I2C_WRITE  (0x02)
#define CMDID_GET_FAST_AE_PARAM (0x03)

typedef struct
{
    u32 state;   // 0 : AE working, 1 : AE done,
    u32 shutter; // us
    u32 again;   // 1X : 1024 (include hcg)
    u32 dgain;   // 1X : 1024
    u32 ymean;   // 0~255 : AE ROI average Y value
    u32 rGain;
    u32 bGain;
} SensorCusAeState_t;


static int pCus_sensor_CustDefineFunction(ss_cus_sensor* handle,u32 cmd_id, void *param) {

    if(param == NULL || handle == NULL)
    {
        SENSOR_EMSG("param/handle data NULL \n");
        return FAIL;
    }

    switch(cmd_id)
    {
        case CMDID_I2C_READ:
        {
            I2C_ARRAY *reg = (I2C_ARRAY *)param;
            SensorReg_Read(reg->reg, (void*)&reg->data);
            SENSOR_EMSG("reg %x, read data %x \n", reg->reg, reg->data);
            break;
        }
        case CMDID_I2C_WRITE:
        {
            I2C_ARRAY *reg = (I2C_ARRAY *)param;
            SENSOR_EMSG("reg %x, write data %x \n", reg->reg, reg->data);
            SensorReg_Write(reg->reg, reg->data);
            break;
        }
        case CMDID_GET_FAST_AE_PARAM:
        {
            SC301iot_params *params = (SC301iot_params *)handle->private_data;
            SensorCusAeState_t *pstParam = (SensorCusAeState_t *)param;
            pstParam->shutter = params->pre_ae_shutter;
            pstParam->again   = params->pre_ae_gain;
            pstParam->rGain   = params->pre_ae_rgain;
            pstParam->bGain   = params->pre_ae_bgain;
            CamOsPrintf("get fastAE param[shutter:%d, gain:%d, rGain:%d, bGain:%d]\n",
                pstParam->shutter, pstParam->again, pstParam->rGain, pstParam->bGain);
            break;
        }
        default:
            SENSOR_EMSG("cmd id %d err \n", cmd_id);
            break;
    }

    return SUCCESS;
}

static int pCus_init_mipi2lane_HDR_VC(ss_cus_sensor *handle)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    int i,cnt=0;
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_DOL_2lane);i++)
    {
        if(Sensor_init_table_HDR_DOL_2lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR_DOL_2lane[i].data);
        }else
        {
            cnt = 0;
            SENSOR_EMSG("reg =  %x, data = %x\n", Sensor_init_table_HDR_DOL_2lane[i].reg, Sensor_init_table_HDR_DOL_2lane[i].data);
            while(SensorReg_Write(Sensor_init_table_HDR_DOL_2lane[i].reg,Sensor_init_table_HDR_DOL_2lane[i].data) != SUCCESS)
            {
                cnt++;
                 SENSOR_EMSG("Sensor_init_table_HDR_DOL_2lane -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
            }
        }
    }

    pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    SENSOR_EMSG("[%s] res_idx[%d] num_res[%d] mipi_hdr_mode[%d]!\n", __FUNCTION__,res_idx,num_res,handle->interface_attr.attr_mipi.mipi_hdr_mode);
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_HDR_VC;
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_2T1;
            handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
            sprintf(handle->video_res_supported.res[res_idx].strResDesc, sc301iot_mipi_hdr[res_idx].senstr.strResDesc);
            vts_30fps_HDR = 3200;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            Preview_line_period  = 10417;//1sec/(vts*fps)=1sec/(3300*30)
            params->expo.max_short_exp=191;//3e23 3e24
            break;
        default:
            break;
    }
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_LEF(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_EMSG("[%s] res)idx[%d]!\n", __FUNCTION__,res_idx);
    handle->video_res_supported.ulcur_res = 0;
    sprintf(handle->strSensorStreamName,"SC301iot_MIPI_HDR_LEF");
    handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_2T1;
    return SUCCESS;
}
static int pCus_GetFPS_HDR_SEF(ss_cus_sensor *handle)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);
    SENSOR_EMSG("pCus_GetFPS_HDR_SEF [%s]\n", __FUNCTION__);
    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;
     sprintf(handle->strSensorStreamName,"SC301iot_MIPI_HDR_LEF");
    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_SEF(ss_cus_sensor *handle, u32 fps)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    SENSOR_EMSG("pCus_SetFPS_HDR_SEF [%s]\n", __FUNCTION__);
    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        return SUCCESS;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        return SUCCESS;
    }else{
      return FAIL;
    }
}

static int pCus_SetFPS_HDR_LEF(ss_cus_sensor *handle, u32 fps)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    SENSOR_EMSG("pCus_SetFPS_HDR_LEF [%s]\n", __FUNCTION__);
    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        return SUCCESS;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        return SUCCESS;
    }else{
      return FAIL;
    }

}
static int pCus_AEStatusNotify_HDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->orien_dirty){
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->orien_dirty = false;
        }
        if(params->reg_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            params->reg_dirty = false;
        }
        break;
        default :
        break;
    }
   // SENSOR_EMSG("pCus_AEStatusNotify_HDR_LEF [%s]\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:

        break;
        default :
        break;
    }
   // SENSOR_EMSG("pCus_AEStatusNotify_HDR_SEF [%s]\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x80}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));
    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 2*dou_lines;
    if(half_lines<2) half_lines=2;//half line
    if (half_lines >  (params->expo.vts - params->expo.max_short_exp) - 11) {
        half_lines =  (params->expo.vts - params->expo.max_short_exp) - 11;
    }
    else
        vts=params->expo.vts;
    vts = vts;
    half_lines = half_lines<<4;

    params->tExpo_reg[0].data = (half_lines>>16) & 0x0f;
    params->tExpo_reg[1].data =  (half_lines>>8) & 0xff;
    params->tExpo_reg[2].data = (half_lines>>0) & 0xf0;

    for (i = 0; i < ARRAY_SIZE(expo_reg); i++)
    {
      if (params->tExpo_reg[i].data != expo_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }
    SENSOR_DMSG("[%s] sensor expo half_lines/us %ld,%ld us\n", __FUNCTION__, half_lines, us);
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period_HDR)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {
        // {0x3e22, 0x00}, // fix me
        {0x3e04, 0x21}, // expo[7:0]
        {0x3e05, 0x00}, // expo[7:4]
    };
    memcpy(expo_reg_temp, params->tExpo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));

    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 2*dou_lines;
    if(half_lines<2) half_lines=2;
    if (half_lines >   (params->expo.max_short_exp)-9) {//10 update for danny 20220427
        half_lines =  (params->expo.max_short_exp)-9;
    }
    else
     vts=params->expo.vts;
    vts = vts;
    half_lines = half_lines<<4;

    params->tExpo_reg_HDR_SEF[0].data =  (half_lines>>8) & 0xff;
    params->tExpo_reg_HDR_SEF[1].data = (half_lines>>0) & 0xf0;
    for (i = 0; i < ARRAY_SIZE(expo_reg_HDR_SEF); i++)
    {
      if (params->tExpo_reg_HDR_SEF[i].data != expo_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
     }
    return SUCCESS;
}
static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u8 i=0 ;
    u32 reg_gain = 0, dcg = 1024, DIG_gain = 1, DIG_Fine_gain_reg = 0;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e13, 0x00},
    };

    memcpy(gain_reg_temp, params->tGain_reg_HDR_SEF, sizeof(gain_reg_temp));
    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAXGAIN*1024) {
        gain = SENSOR_MAXGAIN*1024;
    }

    if((gain >= 1024)&&(gain < 1607)) // x1.569
    {
        dcg = 1024;
        reg_gain = 0x00;
        DIG_gain = 1;
    }
    else if((gain >= 1607)&&(gain < 3214)) // x1.569~ x3.138
    {
        dcg = 1607;
        reg_gain = 0x40;
        DIG_gain = 1;
    }
    else if((gain >= 3214)&&(gain < 6428)) // x3.138 ~ x6.276
    {
        dcg = 3214;
        reg_gain = 0x48;
        DIG_gain = 1;
    }
    else if((gain >= 6428)&&(gain < 12856)) // x6.276 ~ x12.552
    {
        dcg = 6428;
        reg_gain = 0x49;
        DIG_gain = 1;
    }
    else if((gain >= 12856)&&(gain < 25712)) // x12.552 ~ x25.104
    {
        dcg = 12856;
        reg_gain = 0x4b;
        DIG_gain = 1;
    }
    else if((gain >= 25712)&&(gain < 51424)) // x25.104 ~ x50.208
    {
        dcg = 25712;
        reg_gain = 0x4f;
        DIG_gain = 1;
    }
    else if((gain >= 51424)&&(gain < 102848)) // x50.208 ~ x50.208*2
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 1;
    }
    else if((gain >= 102848)&&(gain < 205696)) //  x50.208*2 ~ x50.208*4
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 2;
    }
    else if((gain >= 205696)&&(gain < 411392)) // x50.208*4 ~ x50.208*8
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 4;
    }
	    else if((gain >= 411392)&&(gain < (411392*2))) // x50.208*8 ~ x50.208*16
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = 8;
    }
	    else if((gain >= (411392*2))&&(gain < 1619509)) // x50.208*16 ~ x50.208*31.5
    {
        dcg = 51424;
        reg_gain = 0x5f;
        DIG_gain = ( 8 * 2 );
    }
    DIG_Fine_gain_reg = abs((128*gain/dcg/DIG_gain/4)*4);

    params->tGain_reg_HDR_SEF[2].data = reg_gain;
    params->tGain_reg_HDR_SEF[1].data = DIG_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[0].data = DIG_gain-1;

    for (i = 0; i < ARRAY_SIZE(params->tGain_reg_HDR_SEF); i++)
    {
      if (params->tGain_reg_HDR_SEF[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    return SUCCESS;
}
/*
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max) {
  *min = 30;
  *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max) {
  *min = 1024;
  *max = SENSOR_MAXGAIN*1024;
  return SUCCESS;
}

static int sc301iot_GetShutterInfo(ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS;
    info->min = Preview_line_period + 999;
    info->step = Preview_line_period ;
    return SUCCESS;
}

static int sc301iot_GetShutterInfo_HDR_LEF(ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS; ///12;
    info->min =  Preview_line_period_HDR*2 + 999;//2
    info->step = Preview_line_period_HDR*2;//2
    return SUCCESS;
}

static int sc301iot_GetShutterInfo_HDR_SEF(ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    info->max = (Preview_line_period_HDR * (params->expo.max_short_exp-9));
    info->min =  Preview_line_period_HDR*2 + 999;//2
    info->step = Preview_line_period_HDR*2;//2
    return SUCCESS;
}
*/
static int pCus_poweron_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
   // pCus_poweron(handle,idx);//add
    return SUCCESS;
}

static int pCus_poweroff_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
   // pCus_poweroff(handle,idx);//add
    return SUCCESS;
}
/*static int pCus_GetSensorID_HDR_LEF(ss_cus_sensor *handle, u32 *id)
{
    *id = 0;
     return SUCCESS;
}*/
static int pCus_init_HDR_LEF(ss_cus_sensor *handle)
{
    return SUCCESS;
}

#if 0
static int pCus_GetVideoRes_HDR_LEF( ms_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res )
{
    *res = &handle->video_res_supported.res[res_idx];
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_LEF( ms_cus_sensor *handle, u32 res )
{
    handle->video_res_supported.ulcur_res = 0; //TBD
    return SUCCESS;
}
#endif

static int pCus_GetFPS_HDR_LEF(ss_cus_sensor *handle)
{
    SC301iot_params *params = (SC301iot_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}
/*
static int pCus_setCaliData_gain_linearity_HDR_LEF(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num)
{
    return SUCCESS;
}
static int pCus_SetAEGain_cal_HDR_LEF(ss_cus_sensor *handle, u32 gain)
{
    return SUCCESS;
}
static int pCus_setCaliData_gain_linearity(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num) {

    return SUCCESS;
}
*/
static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    SC301iot_params *params;
    int res;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (SC301iot_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SC301iot_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
   // handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, mipi_linear[res].senstr.strResDesc);
    }

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period + 999;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
   /* handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    handle->PCLK_POLARITY               = SENSOR_PCLK_POL; */ //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 1;

    ///calibration
    //handle->sat_mingain=g_sensor_ae_min_gain;

    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_linear_3M60fps;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;
    handle->pCus_sensor_streamon    = pCus_StreamOn;
    // Normal
    //handle->pCus_sensor_GetSensorID       = pCus_GetSensorID   ;
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS      ;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = SC301iot_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

  //  handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
  //  handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = sc301iot_GetShutterInfo;
    params->expo.vts=vts_60fps;
    params->expo.fps = 60;
    params->expo.line= 10417;
    params->reg_dirty = false;
    params->orien_dirty = false;
    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_SEF(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    SC301iot_params *params = NULL;
    int res;

    //cus_camsensor_init_handle(drv_handle);
    params = (SC301iot_params *)handle->private_data;

    sprintf(handle->strSensorStreamName,"SC301iot_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC_HDR;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;

    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;//CUS_HDR_MODE_DCG;
    handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_2T1;
    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sc301iot_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sc301iot_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sc301iot_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sc301iot_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sc301iot_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sc301iot_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sc301iot_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sc301iot_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc301iot_mipi_hdr[res].senstr.strResDesc);
    }

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR + 999;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;


    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes_HDR;   // Need to check
    handle->pCus_sensor_GetVideoRes    = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes = pCus_GetCurVideoRes;

    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    handle->pCus_sensor_init            = pCus_init_mipi2lane_HDR_VC;
    handle->pCus_sensor_poweron         = pCus_poweron;               // Need to check
    handle->pCus_sensor_poweroff        = pCus_poweroff;              // Need to check
    handle->pCus_sensor_streamon        = pCus_StreamOn;

    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_SEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_SEF;
    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify_HDR_SEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_SEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_SEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_HDR_SEF;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo_HDR_SEF;
   // handle->pCus_sensor_GetShutterInfo = sc301iot_GetShutterInfo_HDR_SEF;

    params->expo.vts = vts_30fps_HDR;
    params->expo.line = 1000;
    params->expo.fps = 30;
    params->expo.max_short_exp=191;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    /*handle->ae_gain_delay       = 2;
    handle->ae_shutter_delay    = 2;

    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 2;*/

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    SC301iot_params *params;
    int res;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (SC301iot_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SC301iot_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;

   // handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;//CUS_HDR_MODE_DCG;
    handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_2T1;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 2;//0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sc301iot_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sc301iot_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sc301iot_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sc301iot_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sc301iot_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sc301iot_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sc301iot_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sc301iot_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc301iot_mipi_hdr[res].senstr.strResDesc);
    }

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR + 999;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////


    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    /*handle->ae_gain_delay       = 2;
    handle->ae_shutter_delay    = 2;

    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 2;

    ///calibration
    handle->sat_mingain=g_sensor_ae_min_gain;*/


    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_HDR_LEF;

    handle->pCus_sensor_poweron     = pCus_poweron_HDR_LEF ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_HDR_LEF;

    // Normal
    //handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_HDR_LEF;

    handle->pCus_sensor_GetVideoResNum = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR_LEF;   // Need to check;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_LEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_LEF;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = SC301iot_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify_HDR_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo_HDR_LEF;
    /*handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_HDR_LEF;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity_HDR_LEF;
    handle->pCus_sensor_GetShutterInfo = sc301iot_GetShutterInfo_HDR_LEF;*/
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->expo.vts=vts_30fps_HDR;
    params->expo.fps = 30;
    params->expo.max_short_exp=191;
    params->reg_dirty = false;
    params->orien_dirty = false;
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(SC301iot,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            SC301iot_params
                         );
