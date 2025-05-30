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
/*
Sensor Porting on Master V4
Porting owner :Jilly
Date          :23/09/19
Build on      :Master_V4 i6c
Verified on   :not yet
Remark        :NA
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>

#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(F53);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MIPI_LANE_NUM_HDR (2)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    ��//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////
#define SENSOR_HDR_MODE                       CUS_HDR_MODE_VC
//#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
//#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT    CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR  CUS_BAYER_BG//CUS_BAYER_GR
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC      //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY

#define Preview_MCLK_SPEED  CUS_CMU_CLK_27MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR  CUS_CMU_CLK_27MHZ   //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 29629                   //2400*1125*30=81
#define Preview_line_period_HDR 29629//30580        //(36M/37.125M)*30fps=29.091fps(34.375msec), hts=34.375/1125=30556,
#define vts_30fps           1125                    //for 29.091fps @ MCLK=36MHz
#define vts_30fps_HDR       2250                    //for 29.091fps @ MCLK=36MHz
#define Preview_WIDTH       1928                    //resolution Width when preview
#define Preview_HEIGHT      1080                    //resolution Height when preview
#define Preview_MAX_FPS     30                      //fastest preview FPS
#define Preview_MAX_FPS_HDR 15                      //fastest preview FPS
#define Preview_MIN_FPS     5                       //slowest preview FPS
#define Preview_CROP_START_X     0                  //CROP_START_X
#define Preview_CROP_START_Y     0                  //CROP_START_Y

#define SENSOR_I2C_ADDR    0x80                   //I2C slave address
#define SENSOR_I2C_SPEED   240000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

//#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
//#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

//#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

//AE info
#define SENSOR_MAX_GAIN                       15872                 // max sensor again, a-gain
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)
#define SENSOR_SHUTTER_CTRL_NUM_HDR           (2)


//static int blk_flag = 1;
//static int  drv_Fnumber = 22;
#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
//static int cus_camsensor_release_handle(ss_cus_sensor *handle);
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
static int pCus_SetOrien_HDR(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
CUS_MCLK_FREQ UseParaMclk(void);

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
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
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

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    u32 skip_cnt;
    bool dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[1];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tExpo_reg_HDR_SEF[2];
    I2C_ARRAY tMirror_reg[2];
    I2C_ARRAY tMirror_reg_HDR[3];
    bool ori_dirty;
    CUS_CAMSENSOR_ORIT  orit;
} F53_params;
// set sensor ID address and data,

/* typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;
 */
const I2C_ARRAY Sensor_id_table[] =
{
    {0x0a, 0x08},
    {0x0b, 0x42},
};

const I2C_ARRAY Sensor_init_table[] =
{
    {0x12, 0x40},
    {0x48, 0x8A},
    {0x48, 0x0A},
    {0x0E, 0x19},
    {0x0F, 0x04},
    {0x10, 0x20},
    {0x11, 0x80},
    {0x46, 0x09},
    {0x47, 0x66},
    {0x0D, 0xF2},
    {0x57, 0x6A},
    {0x58, 0x22},
    {0x5F, 0x41},
    {0x60, 0x24},
    {0xA5, 0xC0},
    {0x20, 0x00},
    {0x21, 0x05},
    {0x22, 0x65},
    {0x23, 0x04},
    {0x24, 0xC4},
    {0x25, 0x40},
    {0x26, 0x43},
    {0x27, 0xC6},
    {0x28, 0x1b},
    {0x29, 0x04},
    {0x2A, 0xBB},
    {0x2B, 0x14},
    {0x2C, 0x00},
    {0x2D, 0x00},
    {0x2E, 0x16},
    {0x2F, 0x04},
    {0x41, 0xC9},
    {0x42, 0x33},
    {0x47, 0x46},
    {0x76, 0x6A},
    {0x77, 0x09},
    {0x80, 0x01},
    {0xAF, 0x22},
    {0xAB, 0x00},
    {0x1D, 0x00},
    {0x1E, 0x04},
    {0x6C, 0x40},
    {0x9E, 0xF8},
    {0x6E, 0x2C},
    {0x70, 0x6C},
    {0x71, 0x6D},
    {0x72, 0x6A},
    {0x73, 0x56},
    {0x74, 0x02},
    {0x78, 0x9D},
    {0x89, 0x01},
    {0x6B, 0x20},
    {0x86, 0x40},
    {0x31, 0x10},
    {0x32, 0x18},
    {0x33, 0xE8},
    {0x34, 0x5E},
    {0x35, 0x5E},
    {0x3A, 0xAF},
    {0x3B, 0x00},
    {0x3C, 0xFF},
    {0x3D, 0xFF},
    {0x3E, 0xFF},
    {0x3F, 0xBB},
    {0x40, 0xFF},
    {0x56, 0x92},
    {0x59, 0xAF},
    {0x5A, 0x47},
    {0x61, 0x18},
    {0x6F, 0x04},
    {0x85, 0x5F},
    {0x8A, 0x44},
    {0x91, 0x13},
    {0x94, 0xA0},
    {0x9B, 0x83},
    {0x9C, 0xE1},
    {0xA4, 0x80},
    {0xA6, 0x22},
    {0xA9, 0x1C},
    {0x5B, 0xE7},
    {0x5C, 0x28},
    {0x5D, 0x67},
    {0x5E, 0x11},
    {0x62, 0x21},
    {0x63, 0x0F},
    {0x64, 0xD0},
    {0x65, 0x02},
    {0x67, 0x49},
    {0x66, 0x00},
    {0x68, 0x00},
    {0x69, 0x72},
    {0x6A, 0x12},
    {0x7A, 0x00},
    {0x82, 0x20},
    {0x8D, 0x47},
    {0x8F, 0x90},
    {0x45, 0x01},
    {0x97, 0x20},
    {0x13, 0x81},
    {0x96, 0x84},
    {0x4A, 0x01},
    {0xB1, 0x00},
    {0xA1, 0x0F},
    {0xBE, 0x00},
    {0x7E, 0x48},
    {0xB5, 0xC0},
    {0x50, 0x02},
    {0x49, 0x10},
    {0x7F, 0x57},
    {0x90, 0x00},
    {0x7B, 0x4A},
    {0x7C, 0x07},
    {0x8C, 0xFF},
    {0x8E, 0x00},
    {0x8B, 0x01},
    {0x0C, 0x00},
    {0xBC, 0x11},
    {0x19, 0x20},
    {0x1B, 0x4F},
    {0x12, 0x30},
    {0x00, 0x10},

};

const I2C_ARRAY Sensor_init_table_HDR[] =
{
    {0x12, 0x48},
    {0x48, 0x8A},
    {0x48, 0x0A},
    {0x0E, 0x19},
    {0x0F, 0x04},
    {0x10, 0x20},
    {0x11, 0x80},
    {0x46, 0x0D},
    {0x47, 0x66},
    {0x0D, 0xF2},
    {0x57, 0x6A},
    {0x58, 0x22},
    {0x5F, 0x41},
    {0x60, 0x24},
    {0xA5, 0xC0},
    {0x20, 0x00},
    {0x21, 0x05},
    {0x22, 0xCA},
    {0x23, 0x08},
    {0x24, 0xC4},
    {0x25, 0x40},
    {0x26, 0x43},
    {0x27, 0xC6},
    {0x28, 0x25},
    {0x29, 0x04},
    {0x2A, 0xBB},
    {0x2B, 0x14},
    {0x2C, 0x00},
    {0x2D, 0x00},
    {0x2E, 0x15},
    {0x2F, 0x04},
    {0x41, 0xC9},
    {0x42, 0x33},
    {0x47, 0x46},
    {0x76, 0x6A},
    {0x77, 0x09},
    {0x80, 0x01},
    {0xAF, 0x22},
    {0xAB, 0x00},
    {0x1D, 0x00},
    {0x1E, 0x04},
    {0x6C, 0x40},
    {0x9E, 0xF8},
    {0x6E, 0x2C},
    {0x70, 0x6C},
    {0x71, 0x6D},
    {0x72, 0x6A},
    {0x73, 0x56},
    {0x74, 0x02},
    {0x78, 0x9D},
    {0x89, 0x81},
    {0x6B, 0x20},
    {0x86, 0x40},
    {0x31, 0x10},
    {0x32, 0x18},
    {0x33, 0xE8},
    {0x34, 0x5E},
    {0x35, 0x5E},
    {0x3A, 0xAF},
    {0x3B, 0x00},
    {0x3C, 0xFF},
    {0x3D, 0xFF},
    {0x3E, 0xFF},
    {0x3F, 0xBB},
    {0x40, 0xFF},
    {0x56, 0x92},
    {0x59, 0xAF},
    {0x5A, 0x47},
    {0x61, 0x18},
    {0x6F, 0x04},
    {0x85, 0x5F},
    {0x8A, 0x44},
    {0x91, 0x13},
    {0x94, 0xA0},
    {0x9B, 0x83},
    {0x9C, 0xE1},
    {0xA4, 0x80},
    {0xA6, 0x22},
    {0xA9, 0x1C},
    {0x5B, 0xE7},
    {0x5C, 0x28},
    {0x5D, 0x67},
    {0x5E, 0x11},
    {0x62, 0x21},
    {0x63, 0x0F},
    {0x64, 0xD0},
    {0x65, 0x02},
    {0x67, 0x49},
    {0x66, 0x00},
    {0x68, 0x00},
    {0x69, 0x72},
    {0x6A, 0x12},
    {0x7A, 0x00},
    {0x82, 0x20},
    {0x8D, 0x47},
    {0x8F, 0x90},
    {0x45, 0x01},
    {0x97, 0x20},
    {0x13, 0x81},
    {0x96, 0x84},
    {0x4A, 0x01},
    {0xB1, 0x00},
    {0xA1, 0x0F},
    {0xBE, 0x00},
    {0x7E, 0x48},
    {0xB5, 0xC0},
    {0x50, 0x02},
    {0x49, 0x10},
    {0x7F, 0x57},
    {0x90, 0x00},
    {0x7B, 0x4A},
    {0x7C, 0x07},
    {0x8C, 0xFF},
    {0x8E, 0x00},
    {0x8B, 0x01},
    {0x0C, 0x00},
    {0xBC, 0x11},
    {0x19, 0x20},
    {0x1B, 0x4F},
    {0x07, 0x43},
    {0x06, 0x80},//23
    {0x03, 0xFF},
    {0x04, 0xFF},
    {0x12, 0x38},
    {0x00, 0x10},
};

const static I2C_ARRAY mirror_reg[] =
{
    {0x12, 0x30}, // bit[5]mirror,bit[4]flip
    {0x28, 0x1b}, // vertical start
};

const static I2C_ARRAY mirror_reg_HDR[] =
{
    {0x12, 0x38}, // mirror bit[5] HDR on/off[3]
    {0x28, 0x25}, // crop for bayer mirror
    {0x2e, 0x15},
};

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] = {
    {0x00, 0x00},   //again:2^PGA[6:4]*(1+PGA[3:0]/16)
};

const I2C_ARRAY expo_reg[] = {
    {0x02, 0x00}, //long expo[15:8] texp=expo[15:0]*Tline
    {0x01, 0xff}, //long expo[7:0]
};

const static I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x08, 0x00}, //short expo[8]
    {0x05, 0xff}, //short expo[7:0]
};

const static I2C_ARRAY vts_reg[] = {
    {0x23,0x04},
    {0x22,0x65},    //frame H 1125
};

#if 0
static CUS_INT_TASK_ORDER def_order = {
        .RunLength = 9,
        .Orders = {
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
        },
};
#endif

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME F53


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
#if 0
static int ISP_config_io(ss_cus_sensor *handle) {
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s]", __FUNCTION__);

    sensor_if->HsyncPol(handle, handle->HSYNC_POLARITY);
    sensor_if->VsyncPol(handle, handle->VSYNC_POLARITY);
    sensor_if->ClkPol(handle, handle->PCLK_POLARITY);
    sensor_if->BayerFmt(handle, handle->bayer_id);
    sensor_if->DataBus(handle, handle->sif_bus);

    sensor_if->DataPrecision(handle, handle->data_prec);
    sensor_if->FmtConv(handle,  handle->data_mode);
    return SUCCESS;
}
#endif

static void pCus_PowerOn_InitChipRX(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
}
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    F53_params *params = (F53_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period_HDR*1;
        info->u32AEShutter_step                  = Preview_line_period_HDR*2;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = (Preview_line_period_HDR * params->expo.max_short_exp);
        }
        else
        {
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    F53_params *params = (F53_params *)handle->private_data;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);//pwd high
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);//rst low
    CamOsMsSleep(2);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);//rst high
    CamOsMsSleep(5);

    //Sensor power on sequence
    //sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    //sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    //sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    //sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
	//sensor_if->MCLK(idx, 1, handle->mclk);
    //if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_DCG) {
    //    sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    //}
    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);
    CamOsMsSleep(5);

    sensor_if->Reset(idx, CUS_CLK_POL_NEG);//rst low
    CamOsMsSleep(15);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);//rst high
    CamOsMsSleep(3);
    SENSOR_DMSG("[%s] pwd high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);//pwd low
    CamOsMsSleep(5);

    params->orit = SENSOR_ORIT;
    //CamOsPrintf("pCus_poweron = %d us \n",timeGetTimeU()-TStart);
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
   ISensorIfAPI *sensor_if = handle->sensor_if_api;
//    F53_params *params = (F53_params *)handle->private_data;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    CamOsMsSleep(5);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    CamOsMsSleep(1);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    CamOsMsSleep(1);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, handle->mclk);

//    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

/////////////////// image function /////////////////////////
//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
#if 0
static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    int i,n;
    int table_length= ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    //SensorReg_Write(0xfe,0x00);

    for(n=0;n<table_length;++n)
    {
      id_from_sensor[n].reg = Sensor_id_table[n].reg;
      id_from_sensor[n].data = 0;
    }

    *id =0;
    if(table_length>8) table_length=8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n=0;n<4;++n) //retry , until I2C success
    {
      if(n>2) return FAIL;

      if( SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
          break;
      else
          continue;
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
      if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
      //*id = id_from_sensor[i].data;
      *id = ((*id)+ id_from_sensor[i].data)<<8;
    }

    *id >>= 8;
    SENSOR_DMSG("[%s]F53 Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    //SENSOR_DMSG("[%s]Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif

static int F53_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init(ss_cus_sensor *handle)
{
    int i,cnt=0;
    F53_params *params = (F53_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table[i].reg,Sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->orit);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

static int pCus_init_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;
    F53_params *params = (F53_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR);i++)
    {
        if(Sensor_init_table_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_HDR[i].reg,Sensor_init_table_HDR[i].data) != SUCCESS)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien_HDR(handle, params->orit );
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

/*
int pCus_release(ss_cus_sensor *handle)
{
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    sensor_if->PCLK(NULL,CUS_PCLK_OFF);
    return SUCCESS;
}
*/

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
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    F53_params *params = (F53_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 15;
            params->expo.max_short_exp=256;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    F53_params *params = (F53_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data & 0x30;
    SENSOR_DMSG("\n\n[%s]:mirror:%x\r\n\n\n\n",__FUNCTION__, sen_data);
    switch(sen_data) {
        case 0x00:
        *orit = CUS_ORIT_M1F1;
        break;
        case 0x20:
        *orit = CUS_ORIT_M0F1;
        break;
        case 0x10:
        *orit = CUS_ORIT_M1F0;
        break;
        case 0x30:
        *orit = CUS_ORIT_M0F0;
        break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    F53_params *params = (F53_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[0].data = 0x30;
            params->tMirror_reg[1].data = 0x1b;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[0].data = 0x10;
            params->tMirror_reg[1].data = 0x1b;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[0].data = 0x20;
            params->tMirror_reg[1].data = 0x11;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[0].data = 0x00;
            params->tMirror_reg[1].data = 0x11;
            params->ori_dirty = true;
        break;
    }
    return SUCCESS;
}

static int pCus_SetOrien_HDR(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    F53_params *params = (F53_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tMirror_reg_HDR[0].data = 0x38;
            params->tMirror_reg_HDR[1].data = 0x25;
            params->tMirror_reg_HDR[2].data = 0x15;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg_HDR[0].data = 0x18;
            params->tMirror_reg_HDR[1].data = 0x25;
            params->tMirror_reg_HDR[2].data = 0x15;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg_HDR[0].data = 0x28;
            params->tMirror_reg_HDR[1].data = 0x21;
            params->tMirror_reg_HDR[2].data = 0x16;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg_HDR[0].data = 0x08;
            params->tMirror_reg_HDR[1].data = 0x21;
            params->tMirror_reg_HDR[2].data = 0x16;
            params->ori_dirty = true;
        break;
    }
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    F53_params *params = (F53_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
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
    F53_params *params = (F53_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
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

    if ((params->expo.line) > (params->expo.vts)-4) {
        vts = params->expo.line + 4;
    }else
        vts = params->expo.vts;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetFPS_HDR_SEF(ss_cus_sensor *handle)
{
    F53_params *params = (F53_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_SEF(ss_cus_sensor *handle, u32 fps)
{
    F53_params *params = (F53_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true;
        return SUCCESS;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true;
        //pr_info("[%s]  vts_reg_sef : %x , %x\n\n", __FUNCTION__,vts_reg[0].data,vts_reg[1].data);
        return SUCCESS;
    }else{
      return FAIL;
    }
}

static int pCus_SetFPS_HDR_LEF(ss_cus_sensor *handle, u32 fps)
{
    F53_params *params = (F53_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true;
        return SUCCESS;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true;
        //pr_info("[%s]  vts_reg_sef : %x , %x\n\n", __FUNCTION__,vts_reg[0].data,vts_reg[1].data);
        return SUCCESS;
    }else{
      return FAIL;
    }

}

#if 0
static int pCus_GetSensorCap(ss_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap) {
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else     return FAIL;
    return SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    F53_params *params = (F53_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->ori_dirty){
                //handle->sensor_if_api->SetSkipFrame(handle->snr_pad_group, params->expo.fps, 3);
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                //sensor_if->BayerFmt(handle, handle->bayer_id);
                params->ori_dirty = false;
            }
        break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                params->dirty = false;
            }
        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    F53_params *params = (F53_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        break;
        case CUS_FRAME_ACTIVE:

            if(params->dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                params->dirty = false;
            }
        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    F53_params *params = (F53_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->ori_dirty){
                //handle->sensor_if_api->SetSkipFrame(handle->snr_pad_group, params->expo.fps, 3);
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg_HDR, ARRAY_SIZE(mirror_reg_HDR));
                params->ori_dirty = false;
            }
        break;
        case CUS_FRAME_ACTIVE:

        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    F53_params *params = (F53_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period_HDR)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);
    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us) {
    u32 vts = 0, lines = 0;
    u16 smpl_start;
    F53_params *params = (F53_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;

    SensorReg_Read(0x06,&smpl_start);
    params->expo.max_short_exp = smpl_start*2;
    if(lines<1) lines=1;
    if (lines > (params->expo.vts - params->expo.max_short_exp - 6)) {
        lines = params->expo.vts - params->expo.max_short_exp - 6;
    }else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg[0].data = (lines>>8) & 0xff;
    params->tExpo_reg[1].data = (lines>>0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    F53_params *params = (F53_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff)<<0;

    *us = (lines*Preview_line_period_HDR)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us) {
    u32 vts=0, lines = 0;
    F53_params *params = (F53_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;
    if(lines<1) lines=1;
    if (lines > params->expo.max_short_exp - 4) {
        lines = params->expo.max_short_exp - 4;
    }else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg_HDR_SEF[0].data = (lines>>8) & 0xff;
    params->tExpo_reg_HDR_SEF[1].data = (lines>>0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    F53_params *params = (F53_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);
    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    F53_params *params = (F53_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period;

    if(lines<1) lines=1;
    if (lines > params->expo.vts-4) {
        vts = lines + 4;
    }else
        vts = params->expo.vts;

    params->expo.line = lines;
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[1].data = (lines>>0) & 0x00ff;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
  //  F53_params *params = (F53_params *)handle->private_data;
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    F53_params *params = (F53_params *)handle->private_data;
    u32 gain2_4,gain4_8,gain8_16;

    if (gain<1024) {
        gain=1024;
    } else if (gain>=15872) {
        gain=15872; //again max 31,suggest 15.5x,Without using digital gain,ISP gain 2x
    }

  //gain_double=(double)gain;
    if(gain<2048) {
      params->tGain_reg[0].data = (((gain-1024)>>6)) & 0x000f;//<X2
    }
    else if((gain>=2048 )&&(gain<4096))//X2~X4
    {
      gain2_4=gain-2048;
      params->tGain_reg[0].data = ((gain2_4>>7) & 0x000f)|0x10;
    }
    else if((gain>=4096 )&&(gain<8192))//X4~X8
    {
      gain4_8=gain-4096;
      params->tGain_reg[0].data =( (gain4_8>>8) & 0x000f)|0x20;
    }
    else if((gain>=8192 )&&(gain<=15872))//X8~X15.5
    {
      gain8_16=gain-8192;
      params->tGain_reg[0].data =( (gain8_16>>9) & 0x000f)|0x30;
    }

    SENSOR_DMSG("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}
#if 0
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max) {
    *min = 30;
    *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max) {
    *min =handle->sat_mingain;
    *max = SENSOR_MAX_GAIN*1024;
    return SUCCESS;
}

static int F53_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS; ///12;
    info->min =  Preview_line_period*1;//2
    info->step = Preview_line_period*1;//2
    return SUCCESS;
}

static int F53_GetShutterInfo_HDR_LEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS; ///12;
    info->min =  Preview_line_period_HDR*1;//2
    info->step = Preview_line_period_HDR*1;//2
    return SUCCESS;
}

static int F53_GetShutterInfo_HDR_SEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    F53_params *params = (F53_params *)handle->private_data;
    info->max = (Preview_line_period_HDR * params->expo.max_short_exp);
    info->min =  Preview_line_period_HDR*1;//2
    info->step = Preview_line_period_HDR*1;//2
    return SUCCESS;
}
#endif
static int pCus_poweron_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}
#if 0
static int pCus_GetSensorID_HDR_LEF(ss_cus_sensor *handle, u32 *id)
{
    *id = 0;
     return SUCCESS;
}
#endif
static int pCus_init_HDR_LEF(ss_cus_sensor *handle)
{
    return SUCCESS;
}
#if 0
static int pCus_GetVideoRes_HDR_LEF( ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res )
{
    *res = &handle->video_res_supported.res[res_idx];
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_LEF( ss_cus_sensor *handle, u32 res )
{
    handle->video_res_supported.ulcur_res = 0; //TBD
    return SUCCESS;
}
#endif

static int pCus_GetFPS_HDR_LEF(ss_cus_sensor *handle)
{
    F53_params *params = (F53_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}
#if 0
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
#endif

#define CMDID_I2C_READ   (0x01)
#define CMDID_I2C_WRITE  (0x02)

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
            SensorReg_Read(reg->reg, &reg->data);
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
        default:
            SENSOR_EMSG("cmd id %d err \n", cmd_id);
            break;
    }

    return SUCCESS;
}

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    F53_params *params;
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
    params = (F53_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tMirror_reg_HDR, mirror_reg_HDR, sizeof(mirror_reg_HDR));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"F53_MIPI_Linear");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    params->orit        = SENSOR_ORIT;      //CUS_ORIT_M1F1;
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

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].u16width                   = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height                  = Preview_HEIGHT;
    handle->video_res_supported.res[0].u16max_fps                 = Preview_MAX_FPS;
    handle->video_res_supported.res[0].u16min_fps                 = Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x            = 2;
    handle->video_res_supported.res[0].u16crop_start_y            = 2;
    handle->video_res_supported.res[0].u16OutputWidth             = 1928;
    handle->video_res_supported.res[0].u16OutputHeight            = 1088;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1920x1080@30fps");

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period*1;//2
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS; ///12;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period*1;//2

    // i2c

    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

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
    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 1;

    ///calibration
    //handle->sat_mingain=g_sensor_ae_min_gain;


    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init    ;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

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
    handle->pCus_sensor_SetPatternMode = F53_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;

    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

     //sensor calibration
//    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = F53_GetShutterInfo;
    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    params->expo.line = 100;
    params->dirty = false;
    params->ori_dirty = false;
    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_SEF(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    F53_params *params = NULL;

    cus_camsensor_init_handle(drv_handle);
    params = (F53_params *)handle->private_data;

    sprintf(handle->strSensorStreamName,"F53_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC_HDR;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = SENSOR_HDR_MODE;

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].u16width                   = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height                  = Preview_HEIGHT;
    handle->video_res_supported.res[0].u16max_fps                 = Preview_MAX_FPS_HDR;
    handle->video_res_supported.res[0].u16min_fps                 = Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x            = 0;
    handle->video_res_supported.res[0].u16crop_start_y            = 0;
    handle->video_res_supported.res[0].u16OutputWidth             = 1928;
    handle->video_res_supported.res[0].u16OutputHeight            = 1088;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1920x1080@15fps_HDR");

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*1;//2
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = (Preview_line_period_HDR * params->expo.max_short_exp);
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR*1;//2

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR;
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    handle->pCus_sensor_init        = pCus_init_HDR;

    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_SEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_SEF;
    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien_HDR;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_SEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_SEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_SEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetShutterInfo = F53_GetShutterInfo_HDR_SEF;

    params->expo.vts = vts_30fps_HDR;
    params->expo.line = 1000;
    params->expo.fps = 15;
    params->expo.max_short_exp=160;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 2;

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    F53_params *params;
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
    params = (F53_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tMirror_reg_HDR, mirror_reg_HDR, sizeof(mirror_reg_HDR));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"F53_MIPI_HDR_LEF");

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
    params->orit        = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = SENSOR_HDR_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].u16width                   = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height                  = Preview_HEIGHT;
    handle->video_res_supported.res[0].u16max_fps                 = Preview_MAX_FPS_HDR;
    handle->video_res_supported.res[0].u16min_fps                 = Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x            = 0;
    handle->video_res_supported.res[0].u16crop_start_y            = 0;
    handle->video_res_supported.res[0].u16OutputWidth             = 1928;
    handle->video_res_supported.res[0].u16OutputHeight            = 1088;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1920x1080@15fps_HDR");

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*1;//2
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;//12;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR*1;//2

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
    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 2;

    ///calibration
    //handle->sat_mingain=g_sensor_ae_min_gain;


    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_HDR_LEF;

    handle->pCus_sensor_poweron     = pCus_poweron_HDR_LEF ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_HDR_LEF;

    // Normal
    //handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_HDR_LEF;

    handle->pCus_sensor_GetVideoResNum    = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes    = NULL;
    handle->pCus_sensor_SetVideoRes       = NULL;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien_HDR;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS_HDR_LEF;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS_HDR_LEF;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = F53_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_LEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;

    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

     //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_HDR_LEF;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity_HDR_LEF;
    //handle->pCus_sensor_GetShutterInfo = F53_GetShutterInfo_HDR_LEF;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->expo.vts=vts_30fps_HDR;
    params->expo.fps = 15;
    params->expo.max_short_exp=160;
    params->dirty = false;
    params->ori_dirty = false;
    return SUCCESS;
}
#if 0
static int cus_camsensor_release_handle(ss_cus_sensor *handle) {

    return SUCCESS;
}
#endif

SENSOR_DRV_ENTRY_IMPL_END_EX(  F53,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            F53_params
                         );
