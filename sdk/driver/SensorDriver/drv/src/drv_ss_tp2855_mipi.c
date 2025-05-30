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
   Porting owner : paul-pc.wang
   Date : 2023/9/25
   Build on : Master V4  I6C
   Verified on : not yet。
   Remark : NA
*/
#ifdef __cplusplus
extern "C"
{
#endif

#define _SUPPORT_MIPI_AHD_
#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>

#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(TP2855_MIPI);

///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @ @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//          @@@@                                             //
//                                                           //
//      Start Step 1 --  show preview on LCM                 //
//                                                           //
//  Fill these #define value and table with correct settings //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////


#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT      CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_DELAY   0x1212                  //CFG
#define SENSOR_DATAPREC     CUS_DATAPRECISION_16    //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_BAYERID      CUS_BAYER_RG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define lane_number 4
#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
#define long_packet_type_enable 0x00 //UD1~UD8 (user define)

#define Preview_MCLK_SPEED  CUS_CMU_CLK_27MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 30000                  ////HTS/PCLK=4455 pixels/148.5MHZ=30usec
#define Prv_Max_line_number 1080                    //maximum exposure line munber of sensor when preview
#define vts_30fps 1125//1346,1616                      //for 29.1fps
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

//#define Capture_MCLK_SPEED  CUS_CMU_CLK_16M     //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Cap_Max_line_number 720                   //maximum exposure line munber of sensor when capture

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16
#define SENSOR_I2C_ADDR     0x88                   //I2C slave address
#define SENSOR_I2C_SPEED    20000   //200KHz
#define SENSOR_I2C_CHANNEL  1                           //I2C Channel
#define SENSOR_I2C_PAD_MODE 2                           //Pad/Mode Number

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG//CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_POS        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
//static int  drv_Fnumber = 22;
#define SENSOR_MAX_GAIN     177*1024//(31.5)                  // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN     1024//                     // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
#if 0
static CUS_CAMSENSOR_CAP sensor_cap = {
    .length = sizeof(CUS_CAMSENSOR_CAP),
    .version = 0x0001,
};
#endif
typedef struct {
    // struct {
    //     u16 pre_div0;
    //     u16 div124;
    //     u16 div_cnt7b;
    //     u16 sdiv0;
    //     u16 mipi_div0;
    //     u16 r_divp;
    //     u16 sdiv1;
    //     u16 r_seld5;
    //     u16 r_sclk_dac;
    //     u16 sys_sel;
    //     u16 pdac_sel;
    //     u16 adac_sel;
    //     u16 pre_div_sp;
    //     u16 r_div_sp;
    //     u16 div_cnt5b;
    //     u16 sdiv_sp;
    //     u16 div12_sp;
    //     u16 mipi_lane_sel;
    //     u16 div_dac;
    // } clk_tree;
    struct {
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;

    // struct {
    //     float sclk;
    //     u32 hts;
    //     u32 vts;
    //     u32 ho;
    //     u32 xinc;
    //     u32 line_freq;
    //     u32 us_per_line;
    //     u32 final_us;
    //     u32 final_gain;
    //     u32 back_pv_us;
    //     u32 fps;
    // } expo;

    // int sen_init;
    // int still_min_fps;
    // int video_min_fps;
    // bool reg_dirty; //sensor setting need to update through I2C
} tp2855_params;

enum {
    CH_1=0,   //
    CH_2=1,   //
    CH_3=2,   //
    CH_4=3,   //
    CH_ALL=4, //
    MIPI_PAGE=8,
};
enum {
    STD_TVI, //TVI
    STD_HDA, //AHD
};
enum {
    PAL = 0,
    NTSC,
    HD25,  //720p25
    HD30,  //720p30
    FHD25, //1080p25
    FHD30, //1080p30
    FHD50, //1080p50
    FHD60, //1080p60
    QHD25, //2560x1440p25
    QHD30, //2560x1440p30
    UVGA25, //1280x960p25, must use with MIPI_4CH4LANE_445M
    UVGA30, //1280x960p30, must use with MIPI_4CH4LANE_445M
};
enum {
    MIPI_4CH4LANE_297M, //up to 4x720p25/30
    MIPI_4CH4LANE_594M, //up to 4x1080p25/30
    MIPI_4CH2LANE_594M, //up to 4x720pp25/30
    MIPI_2CH2LANE_594M, //up to 2x1080p25/30
    MIPI_4CH4LANE_445M, //only for 4x960p25/30
    MIPI_2CH4LANE_594M, //up to 2xQHDp25/30 or 2x1080p50/60
};

///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @  @@                                             //
//           @@                                              //
//          @@                                               //
//         @@@@@                                             //
//                                                           //
//      Start Step 2 --  set Sensor initial and              //
//                       adjust parameter                    //
//  Fill these register table with resolution settings       //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//       @@@@@@@                                               //
//           @@                                                //
//          @@                                                 //
//          @@@                                                //
//       @   @@                                                //
//        @@@@                                                 //
//                                                             //
//      Step 3 --  complete camera features                    //
//                                                             //
//  camera set EV, MWB, orientation, contrast, sharpness       //
//   , saturation, and Denoise can work correctly.             //
//                                                             //
/////////////////////////////////////////////////////////////////


typedef struct
{
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

#if 0
static I2C_ARRAY mirror_reg[] =
{

};

static I2C_ARRAY gain_reg[] =
{

};
#endif
// static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] =
// {

// };
#if 0
static I2C_ARRAY expo_reg[] =
{

};

static I2C_ARRAY vts_reg[] =
{

};
#endif
/*
static CUS_INT_TASK_ORDER def_order =
{
    .RunLength = 9,
    .Orders =
    {
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
};*/

static unsigned char fmt[4] = {FHD25, FHD25, FHD25, FHD25};
static unsigned char std[4] = {STD_TVI, STD_TVI, STD_TVI, STD_TVI};
static unsigned char output = MIPI_4CH4LANE_297M;

/////////// function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period *5;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    //Sensor power on sequence
    //sensor_if->SetIOPad(idx, handle->sif_bus, 0);
    //sensor_if->SetCSI_Clk(CUS_CSI_CLK_DISABLE);       //Set_csi_if(0, 0); //disable MIPI
    //
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0x4000, 0x0, 0);

    sensor_if->MCLK(idx, 1, handle->mclk);
    //
    #if 0
    SENSOR_EMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, handle->reset_POLARITY);
    SENSOR_MSLEEP(5);
    SENSOR_EMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);
    SENSOR_MSLEEP(5);

    // power -> high, reset -> high
    SENSOR_EMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    SENSOR_MSLEEP(5);
    SENSOR_EMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !handle->reset_POLARITY);
    SENSOR_MSLEEP(5);

    //sensor_if->Set3ATaskOrder( def_order);
    // pure power on
    sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    #endif
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_MSLEEP(5);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(5);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(5);

    return SUCCESS;
}
static int pCus_poweron_dummy(ss_cus_sensor *handle, u32 idx)
{

return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{

    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);

    SENSOR_MSLEEP(1);
    //sensor_if->SetCSI_Clk(CUS_CSI_CLK_DISABLE);       //Set_csi_if(0, 0);
    sensor_if->MCLK(idx, 0, handle->mclk);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);

    return SUCCESS;
}
static int pCus_poweroff_dummy(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

/////////////////// image function /////////////////////////
//Get and check sensor ID

static int tp2855_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
#if 0
    int i;
        SENSOR_DMSG("[%s]\n", __FUNCTION__);
    switch(mode)
    {
        case 1:
            PatternTbl[2].data = 0x08;
            PatternTbl[3].data = 0x30;
            PatternTbl[4].data = 0x00;
            PatternTbl[6].data |= 0x01; //enable
            break;
        case 0:
        default:
            PatternTbl[2].data = 0x08;
            PatternTbl[3].data = 0x34;
            PatternTbl[4].data = 0x3C;
            PatternTbl[6].data &= ~0x01; //disable
            break;
    }

    for(i=0;i< ARRAY_SIZE(PatternTbl);i++)
    {
        if (SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
        {
            SENSOR_EMSG("[%s:%d]SensorReg_Write fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }
#endif
    return SUCCESS;
}

#if 0//defined(__MV5_FPGA__)
static int pCus_I2CWrite(ss_cus_sensor *handle, unsigned short usreg, unsigned short usval)
{
    unsigned short sen_data;

    SensorReg_Write(usreg, usval);
    SensorReg_Read(usreg, &sen_data);
    UartSendTrace("tp2855 reg: 0x%x, data: 0x%x, read: 0x%x.\n", usreg, usval, sen_data);

    return SUCCESS;
}

static int pCus_I2CRead(ss_cus_sensor *handle, unsigned short usreg, unsigned short* pusval)
{
    unsigned short usread_data;

    SensorReg_Read(usreg, &usread_data);
    *pusval = usread_data;
    UartSendTrace("tp2855 reg: 0x%x, data: 0x%x\n", usreg, usread_data);

    return SUCCESS;
}
#endif

/////////////////////////////////
//ch: video channel
//fmt: PAL/NTSC/HD25/HD30
//std: STD_TVI/STD_HDA
////////////////////////////////
static void TP2855_decoder_init(ss_cus_sensor *handle, unsigned char ch, unsigned char fmt, unsigned char std)
{
    unsigned short tmp;
    const unsigned char SYS_MODE[5]={0x01,0x02,0x04,0x08,0x0f};
    SensorReg_Write(0x40, ch);
    if (PAL == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp |= SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x47);
        SensorReg_Write(0x0c, 0x13);
        SensorReg_Write(0x0d, 0x51);

        SensorReg_Write(0x15, 0x13);
        SensorReg_Write(0x16, 0x76);
        SensorReg_Write(0x17, 0x80);
        SensorReg_Write(0x18, 0x17);
        SensorReg_Write(0x19, 0x20);
        SensorReg_Write(0x1a, 0x17);
        SensorReg_Write(0x1c, 0x09);
        SensorReg_Write(0x1d, 0x48);

        SensorReg_Write(0x20, 0x48);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x37);
        SensorReg_Write(0x23, 0x3f);

        SensorReg_Write(0x2b, 0x70);
        SensorReg_Write(0x2c, 0x2a);
        SensorReg_Write(0x2d, 0x64);
        SensorReg_Write(0x2e, 0x56);

        SensorReg_Write(0x30, 0x7a);
        SensorReg_Write(0x31, 0x4a);
        SensorReg_Write(0x32, 0x4d);
        SensorReg_Write(0x33, 0xf0);

        SensorReg_Write(0x35, 0x25);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x04);
    }
    else if (NTSC == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp |= SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x47);
        SensorReg_Write(0x0c, 0x13);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x13);
        SensorReg_Write(0x16, 0x60);
        SensorReg_Write(0x17, 0x80);
        SensorReg_Write(0x18, 0x12);
        SensorReg_Write(0x19, 0xf0);
        SensorReg_Write(0x1a, 0x07);
        SensorReg_Write(0x1c, 0x09);
        SensorReg_Write(0x1d, 0x38);

        SensorReg_Write(0x20, 0x40);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x70);
        SensorReg_Write(0x2c, 0x2a);
        SensorReg_Write(0x2d, 0x68);
        SensorReg_Write(0x2e, 0x57);

        SensorReg_Write(0x30, 0x62);
        SensorReg_Write(0x31, 0xbb);
        SensorReg_Write(0x32, 0x96);
        SensorReg_Write(0x33, 0xc0);

        SensorReg_Write(0x35, 0x25);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x04);
    }
    else if (HD25 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp |= SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x42);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x13);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x13);
        SensorReg_Write(0x16, 0x15);
        SensorReg_Write(0x17, 0x00);
        SensorReg_Write(0x18, 0x19);
        SensorReg_Write(0x19, 0xd0);
        SensorReg_Write(0x1a, 0x25);
        SensorReg_Write(0x1c, 0x07);  //1280*720, 25fps
        SensorReg_Write(0x1d, 0xbc);  //1280*720, 25fps

        SensorReg_Write(0x20, 0x30);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x30);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x48);
        SensorReg_Write(0x31, 0xbb);
        SensorReg_Write(0x32, 0x2e);
        SensorReg_Write(0x33, 0x90);

        SensorReg_Write(0x35, 0x25);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x18);

        if (STD_HDA == std)
        {
            SensorReg_Write(0x02, 0x46);

            SensorReg_Write(0x0d, 0x71);

            SensorReg_Write(0x20, 0x40);
            SensorReg_Write(0x21, 0x46);

            SensorReg_Write(0x25, 0xfe);
            SensorReg_Write(0x26, 0x01);

            SensorReg_Write(0x2c, 0x3a);
            SensorReg_Write(0x2d, 0x5a);
            SensorReg_Write(0x2e, 0x40);

            SensorReg_Write(0x30, 0x9e);
            SensorReg_Write(0x31, 0x20);
            SensorReg_Write(0x32, 0x10);
            SensorReg_Write(0x33, 0x90);
        }
    }
    else if (HD30 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp |= SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x42);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x13);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x13);
        SensorReg_Write(0x16, 0x15);
        SensorReg_Write(0x17, 0x00);
        SensorReg_Write(0x18, 0x19);
        SensorReg_Write(0x19, 0xd0);
        SensorReg_Write(0x1a, 0x25);
        SensorReg_Write(0x1c, 0x06);  //1280*720, 30fps
        SensorReg_Write(0x1d, 0x72);  //1280*720, 30fps

        SensorReg_Write(0x20, 0x30);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x30);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x48);
        SensorReg_Write(0x31, 0xbb);
        SensorReg_Write(0x32, 0x2e);
        SensorReg_Write(0x33, 0x90);

        SensorReg_Write(0x35, 0x25);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x18);

        if (STD_HDA == std)
        {
            SensorReg_Write(0x02, 0x46);

            SensorReg_Write(0x0d, 0x70);

            SensorReg_Write(0x20, 0x40);
            SensorReg_Write(0x21, 0x46);

            SensorReg_Write(0x25, 0xfe);
            SensorReg_Write(0x26, 0x01);

            SensorReg_Write(0x2c, 0x3a);
            SensorReg_Write(0x2d, 0x5a);
            SensorReg_Write(0x2e, 0x40);

            SensorReg_Write(0x30, 0x9d);
            SensorReg_Write(0x31, 0xca);
            SensorReg_Write(0x32, 0x01);
            SensorReg_Write(0x33, 0xd0);
        }
    }
    else if (FHD30 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp &= ~SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x40);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x03);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x03);
        SensorReg_Write(0x16, 0xd2);
        SensorReg_Write(0x17, 0x80);
        SensorReg_Write(0x18, 0x29);
        SensorReg_Write(0x19, 0x38);
        SensorReg_Write(0x1a, 0x47);
        SensorReg_Write(0x1c, 0x08);  //1920*1080, 30fps
        SensorReg_Write(0x1d, 0x98);  //

        SensorReg_Write(0x20, 0x30);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x30);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x48);
        SensorReg_Write(0x31, 0xbb);
        SensorReg_Write(0x32, 0x2e);
        SensorReg_Write(0x33, 0x90);

        SensorReg_Write(0x35, 0x05);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x1C);

        if (STD_HDA == std)
        {
            SensorReg_Write(0x02, 0x44);

            SensorReg_Write(0x0d, 0x72);

            SensorReg_Write(0x15, 0x01);
            SensorReg_Write(0x16, 0xf0);

            SensorReg_Write(0x20, 0x38);
            SensorReg_Write(0x21, 0x46);

            SensorReg_Write(0x25, 0xfe);
            SensorReg_Write(0x26, 0x0d);

            SensorReg_Write(0x2c, 0x3a);
            SensorReg_Write(0x2d, 0x54);
            SensorReg_Write(0x2e, 0x40);

            SensorReg_Write(0x30, 0xa5);
            SensorReg_Write(0x31, 0x95);
            SensorReg_Write(0x32, 0xe0);
            SensorReg_Write(0x33, 0x60);
        }
    }
    else if (FHD25 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp &= ~SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x40);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x03);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x03);
        SensorReg_Write(0x16, 0xd2);
        SensorReg_Write(0x17, 0x80);
        SensorReg_Write(0x18, 0x29);
        SensorReg_Write(0x19, 0x38);
        SensorReg_Write(0x1a, 0x47);

        SensorReg_Write(0x1c, 0x0a);  //1920*1080, 25fps
        SensorReg_Write(0x1d, 0x50);  //

        SensorReg_Write(0x20, 0x30);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x30);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x48);
        SensorReg_Write(0x31, 0xbb);
        SensorReg_Write(0x32, 0x2e);
        SensorReg_Write(0x33, 0x90);

        SensorReg_Write(0x35, 0x05);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x1C);

        if (STD_HDA == std)
        {
            SensorReg_Write(0x02, 0x44);

            SensorReg_Write(0x0d, 0x73);

            SensorReg_Write(0x15, 0x01);
            SensorReg_Write(0x16, 0xf0);

            SensorReg_Write(0x20, 0x3c);
            SensorReg_Write(0x21, 0x46);

            SensorReg_Write(0x25, 0xfe);
            SensorReg_Write(0x26, 0x0d);

            SensorReg_Write(0x2c, 0x3a);
            SensorReg_Write(0x2d, 0x54);
            SensorReg_Write(0x2e, 0x40);

            SensorReg_Write(0x30, 0xa5);
            SensorReg_Write(0x31, 0x86);
            SensorReg_Write(0x32, 0xfb);
            SensorReg_Write(0x33, 0x60);
        }
    }
    else if (FHD60 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp &= ~SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x40);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x03);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x03);
        SensorReg_Write(0x16, 0xf0);
        SensorReg_Write(0x17, 0x80);
        SensorReg_Write(0x18, 0x12);
        SensorReg_Write(0x19, 0x38);
        SensorReg_Write(0x1a, 0x47);
        SensorReg_Write(0x1c, 0x08);  //
        SensorReg_Write(0x1d, 0x96);  //

        SensorReg_Write(0x20, 0x38);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x27, 0xad);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x40);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x74);
        SensorReg_Write(0x31, 0x9b);
        SensorReg_Write(0x32, 0xa5);
        SensorReg_Write(0x33, 0xe0);

        SensorReg_Write(0x35, 0x05);
        SensorReg_Write(0x38, 0x40);
        SensorReg_Write(0x39, 0x68);
    }
    else if (FHD50 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp &= ~SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x40);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x03);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x03);
        SensorReg_Write(0x16, 0xe2);
        SensorReg_Write(0x17, 0x80);
        SensorReg_Write(0x18, 0x27);
        SensorReg_Write(0x19, 0x38);
        SensorReg_Write(0x1a, 0x47);

        SensorReg_Write(0x1c, 0x0a);  //
        SensorReg_Write(0x1d, 0x4e);  //

        SensorReg_Write(0x20, 0x38);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x27, 0xad);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x40);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x74);
        SensorReg_Write(0x31, 0x9b);
        SensorReg_Write(0x32, 0xa5);
        SensorReg_Write(0x33, 0xe0);

        SensorReg_Write(0x35, 0x05);
        SensorReg_Write(0x38, 0x40);
        SensorReg_Write(0x39, 0x68);
    }
    else if (QHD30 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp &= ~SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x50);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x03);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x23);
        SensorReg_Write(0x16, 0x1b);
        SensorReg_Write(0x17, 0x00);
        SensorReg_Write(0x18, 0x38);
        SensorReg_Write(0x19, 0xa0);
        SensorReg_Write(0x1a, 0x5a);
        SensorReg_Write(0x1c, 0x0c);  //2560*1440, 30fps
        SensorReg_Write(0x1d, 0xe2);  //

        SensorReg_Write(0x20, 0x50);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x27, 0xad);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x58);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x74);
        SensorReg_Write(0x31, 0x58);
        SensorReg_Write(0x32, 0x9f);
        SensorReg_Write(0x33, 0x60);

        SensorReg_Write(0x35, 0x15);
        SensorReg_Write(0x36, 0xdc);
        SensorReg_Write(0x38, 0x40);
        SensorReg_Write(0x39, 0x48);

    }
    else if (QHD25 == fmt)
    {
        SensorReg_Read(0xf5, &tmp);
        tmp &= ~SYS_MODE[ch];
        SensorReg_Write(0xf5, tmp);

        SensorReg_Write(0x02, 0x50);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x03);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x23);
        SensorReg_Write(0x16, 0x1b);
        SensorReg_Write(0x17, 0x00);
        SensorReg_Write(0x18, 0x38);
        SensorReg_Write(0x19, 0xa0);
        SensorReg_Write(0x1a, 0x5a);
        SensorReg_Write(0x1c, 0x0f);  //2560*1440, 25fps
        SensorReg_Write(0x1d, 0x76);  //

        SensorReg_Write(0x20, 0x50);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x27, 0xad);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x58);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x74);
        SensorReg_Write(0x31, 0x58);
        SensorReg_Write(0x32, 0x9f);
        SensorReg_Write(0x33, 0x60);

        SensorReg_Write(0x35, 0x15);
        SensorReg_Write(0x36, 0xdc);
        SensorReg_Write(0x38, 0x40);
        SensorReg_Write(0x39, 0x48);

    }
    else if (UVGA25 == fmt) //960P25
    {
        SensorReg_Write(0xf5, 0xf0);

        SensorReg_Write(0x02, 0x42);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x13);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x13);
        SensorReg_Write(0x16, 0x16);
        SensorReg_Write(0x17, 0x00);
        SensorReg_Write(0x18, 0xa0);
        SensorReg_Write(0x19, 0xc0);
        SensorReg_Write(0x1a, 0x35);
        SensorReg_Write(0x1c, 0x07);  //
        SensorReg_Write(0x1d, 0xbc);  //

        SensorReg_Write(0x20, 0x30);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x30);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x48);
        SensorReg_Write(0x31, 0xba);
        SensorReg_Write(0x32, 0x2e);
        SensorReg_Write(0x33, 0x90);

        SensorReg_Write(0x35, 0x14);
        SensorReg_Write(0x36, 0x65);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x1c);
    }
    else if (UVGA30 == fmt) //960P30
    {
        SensorReg_Write(0xf5, 0xf0);

        SensorReg_Write(0x02, 0x42);
        SensorReg_Write(0x07, 0xc0);
        SensorReg_Write(0x0b, 0xc0);
        SensorReg_Write(0x0c, 0x13);
        SensorReg_Write(0x0d, 0x50);

        SensorReg_Write(0x15, 0x13);
        SensorReg_Write(0x16, 0x16);
        SensorReg_Write(0x17, 0x00);
        SensorReg_Write(0x18, 0xa0);
        SensorReg_Write(0x19, 0xc0);
        SensorReg_Write(0x1a, 0x35);
        SensorReg_Write(0x1c, 0x06);  //
        SensorReg_Write(0x1d, 0x72);  //

        SensorReg_Write(0x20, 0x30);
        SensorReg_Write(0x21, 0x84);
        SensorReg_Write(0x22, 0x36);
        SensorReg_Write(0x23, 0x3c);

        SensorReg_Write(0x2b, 0x60);
        SensorReg_Write(0x2c, 0x0a);
        SensorReg_Write(0x2d, 0x30);
        SensorReg_Write(0x2e, 0x70);

        SensorReg_Write(0x30, 0x43);
        SensorReg_Write(0x31, 0x3b);
        SensorReg_Write(0x32, 0x79);
        SensorReg_Write(0x33, 0x90);

        SensorReg_Write(0x35, 0x14);
        SensorReg_Write(0x36, 0x65);
        SensorReg_Write(0x38, 0x00);
        SensorReg_Write(0x39, 0x1c);
    }
}

static void TP2855_mipi_out(ss_cus_sensor *handle, unsigned char output)
{
    //mipi setting
    SensorReg_Write(0x40, MIPI_PAGE); //MIPI page
    SensorReg_Write(0x01, 0xf0);
    SensorReg_Write(0x02, 0x01);
    SensorReg_Write(0x08, 0x0f);

    if (MIPI_4CH4LANE_594M == output || MIPI_2CH4LANE_594M == output)
    {
        SensorReg_Write(0x20, 0x44);
        if (MIPI_2CH4LANE_594M == output)
            SensorReg_Write(0x20, 0x24);
        SensorReg_Write(0x34, 0xe4); //
        SensorReg_Write(0x15, 0x0C);
        SensorReg_Write(0x25, 0x08);
        SensorReg_Write(0x26, 0x06);
        SensorReg_Write(0x27, 0x11);
        SensorReg_Write(0x29, 0x0a);
        SensorReg_Write(0x33, 0x07);
        SensorReg_Write(0x33, 0x00);
        SensorReg_Write(0x14, 0x33);
        SensorReg_Write(0x14, 0xb3);
        SensorReg_Write(0x14, 0x33);
    }
    else if (MIPI_4CH4LANE_297M == output)
    {
        SensorReg_Write(0x20, 0x44);
        SensorReg_Write(0x34, 0xe4); //
        SensorReg_Write(0x14, 0x44);
        SensorReg_Write(0x15, 0x0d);
        SensorReg_Write(0x25, 0x04);
        SensorReg_Write(0x26, 0x03);
        SensorReg_Write(0x27, 0x09);
        SensorReg_Write(0x29, 0x02);
        SensorReg_Write(0x33, 0x07);
        SensorReg_Write(0x33, 0x00);
        SensorReg_Write(0x14, 0xc4);
        SensorReg_Write(0x14, 0x44);

    }
    else if (MIPI_4CH2LANE_594M == output)
    {
        SensorReg_Write(0x20, 0x42);
        SensorReg_Write(0x34, 0xe4); //output vin1&vin2
        SensorReg_Write(0x15, 0x0c);
        SensorReg_Write(0x25, 0x08);
        SensorReg_Write(0x26, 0x06);
        SensorReg_Write(0x27, 0x11);
        SensorReg_Write(0x29, 0x0a);
        SensorReg_Write(0x33, 0x07);
        SensorReg_Write(0x33, 0x00);
        SensorReg_Write(0x14, 0x43);
        SensorReg_Write(0x14, 0xc3);
        SensorReg_Write(0x14, 0x43);
    }
    else if (MIPI_2CH2LANE_594M == output)
    {
        SensorReg_Write(0x20, 0x22);
        SensorReg_Write(0x34, 0x10); //output vin1&vin2
        SensorReg_Write(0x15, 0x0c);
        SensorReg_Write(0x25, 0x08);
        SensorReg_Write(0x26, 0x06);
        SensorReg_Write(0x27, 0x11);
        SensorReg_Write(0x29, 0x0a);
        SensorReg_Write(0x33, 0x07);
        SensorReg_Write(0x33, 0x00);
        SensorReg_Write(0x14, 0x43);
        SensorReg_Write(0x14, 0xc3);
        SensorReg_Write(0x14, 0x43);
    }
    else if (MIPI_4CH4LANE_445M == output) //only for 4x960p25/30
    {
        SensorReg_Write(0x20, 0x44);
        SensorReg_Write(0x34, 0xe4); //
        SensorReg_Write(0x12, 0x5f);
        SensorReg_Write(0x13, 0x07);
        SensorReg_Write(0x15, 0x0C);
        SensorReg_Write(0x25, 0x08);
        SensorReg_Write(0x26, 0x06);
        SensorReg_Write(0x27, 0x11);
        SensorReg_Write(0x29, 0x0a);
        SensorReg_Write(0x33, 0x07);
        SensorReg_Write(0x33, 0x00);
        SensorReg_Write(0x14, 0x33);
        SensorReg_Write(0x14, 0xb3);
        SensorReg_Write(0x14, 0x33);
    }

    /* Enable MIPI CSI2 output */
    SensorReg_Write(0x23, 0x02);
    SensorReg_Write(0x23, 0x00);
}

static int pCus_init(ss_cus_sensor *handle)
{
    SENSOR_MSLEEP(10); // Sleep for i2c timeout

    if (lane_num == 2) {
        if (fmt[0] <= HD30 && fmt[1] <= HD30 && fmt[2] <= HD30 && fmt[3] <= HD30) {
            output = MIPI_4CH2LANE_594M;
            TP2855_decoder_init(handle, CH_1, fmt[0], std[0]);
            TP2855_decoder_init(handle, CH_2, fmt[1], std[1]);
            TP2855_decoder_init(handle, CH_3, fmt[2], std[2]);
            TP2855_decoder_init(handle, CH_4, fmt[3], std[3]);
            TP2855_mipi_out(handle, output);
        } else if (fmt[0] <= FHD30 && fmt[1] <= FHD30) {
            output = MIPI_2CH2LANE_594M;
            TP2855_decoder_init(handle, CH_1, fmt[0], std[0]);
            TP2855_decoder_init(handle, CH_2, fmt[1], std[1]);
            TP2855_mipi_out(handle, output);
        } else {
            SENSOR_EMSG("tp2855 not support\n");
            return SUCCESS;
        }
    } else if (lane_num == 4) {
        if (fmt[0] <= HD30 && fmt[1] <= HD30 && fmt[2] <= HD30 && fmt[3] <= HD30) {
            output = MIPI_4CH4LANE_297M;
            TP2855_decoder_init(handle, CH_1, fmt[0], std[0]);
            TP2855_decoder_init(handle, CH_2, fmt[1], std[1]);
            TP2855_decoder_init(handle, CH_3, fmt[2], std[2]);
            TP2855_decoder_init(handle, CH_4, fmt[3], std[3]);
            TP2855_mipi_out(handle, output);
        } else if (fmt[0] <= FHD30 && fmt[1] <= FHD30 && fmt[2] <= FHD30 && fmt[3] <= FHD30) {
            output = MIPI_4CH4LANE_594M;
            TP2855_decoder_init(handle, CH_1, fmt[0], std[0]);
            TP2855_decoder_init(handle, CH_2, fmt[1], std[1]);
            TP2855_decoder_init(handle, CH_3, fmt[2], std[2]);
            TP2855_decoder_init(handle, CH_4, fmt[3], std[3]);
            TP2855_mipi_out(handle, output);
        } else if (fmt[0] <= QHD30 && fmt[1] <= QHD30) {
            output = MIPI_2CH4LANE_594M;
            TP2855_decoder_init(handle, CH_1, fmt[0], std[0]);
            TP2855_decoder_init(handle, CH_2, fmt[1], std[1]);
            TP2855_mipi_out(handle, output);
        } else {
            SENSOR_EMSG("tp2855 not support\n");
            return SUCCESS;
        }
    }

    SENSOR_MSLEEP(200); // TBD : For stable
    return SUCCESS;
}

static int pCus_init_dummy(ss_cus_sensor *handle)
{
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
    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    if (res_idx >= handle->video_res_supported.num_res)
    {
        handle->video_res_supported.ulcur_res = 0; //TBD
    }
    else
    {
        handle->video_res_supported.ulcur_res = res_idx; //TBD
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
#if 0
    char sen_data;

    sen_data = mirror_reg[0].data;
    SENSOR_DMSG("[%s] mirror:%x\r\n", __FUNCTION__, sen_data & 0x03);
    switch(sen_data & 0x03)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
            break;
        case 0x02:
            *orit = CUS_ORIT_M1F0;
            break;
        case 0x01:
            *orit = CUS_ORIT_M0F1;
            break;
        case 0x03:
            *orit = CUS_ORIT_M1F1;
            break;
    }
#endif
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
#if 0
    char index=0;
    tp2855_params *params = (tp2855_params *)handle->private_data;

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            index = 0;
            break;
        case CUS_ORIT_M1F0:
            index = 1;
            break;
        case CUS_ORIT_M0F1:
            index = 2;
            break;
        case CUS_ORIT_M1F1:
            index = 3;
            break;
    }
    SENSOR_DMSG("[%d] CUS_CAMSENSOR_ORIT orit : %x\r\n",__FUNCTION__, orit);
    if (index != mirror_reg[0].data)
    {
        mirror_reg[0].data = index;
        SensorRegArrayW((I2C_ARRAY*)mirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
        params->reg_dirty = true;
    }
#endif
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    //tp2855_params *params = (tp2855_params *)handle->private_data;
    //SENSOR_DMSG("[%s] FPS %d\n", __FUNCTION__, params->expo.fps);

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
#if 0
    tp2855_params *params = (tp2855_params *)handle->private_data;

    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    return 0; //test only

    if (fps>=3 && fps <= 30)
    {
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*30.1f)/fps;
        //vts_reg[0].data = (params->expo.vts>> 16) & 0x0003;
        vts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        vts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
       // params->reg_dirty = true; //reg need to update = true;
       return SUCCESS;
    }
    else if (fps>=3000 && fps <= 30000)
    {
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*30100.0f)/fps;
        vts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        vts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        return SUCCESS;
    }
    else
    {
        //SENSOR_EMSG("[%s] FPS %d out of range.\n",fps,__FUNCTION__);
        return FAIL;
    }
#endif
    return SUCCESS;
}
#if 0
static int pCus_GetSensorCap(ss_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap)
{
    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else
        return FAIL;
    return SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    // tp2855_params *params = (tp2855_params *)handle->private_data;
    // SENSOR_DMSG("[%s]\n", __FUNCTION__);
    // switch(status)
    // {
    //     case CUS_FRAME_INACTIVE:
    //         //SensorReg_Write(0x3001,0);
    //         break;
    //     case CUS_FRAME_ACTIVE:
    //         if (params->reg_dirty)
    //         {
    //             //SensorReg_Write(0x3001,1);

    //             // SensorRegArrayW((I2C_ARRAY*)mirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
    //             // SensorRegArrayW((I2C_ARRAY*)expo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
    //             // SensorRegArrayW((I2C_ARRAY*)gain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
    //             //  SensorRegArrayW((I2C_ARRAY*)vts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));

    //             //SensorReg_Write(0x3001,0);
    //             params->reg_dirty = false;
    //         }
    //         break;
    //     default :
    //         break;
    // }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
#if 0
    //int rc;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)expo_reg, ARRAY_SIZE(expo_reg));

    //lines |= (u32)(expo_reg[0].data&0xff)<<16;
    lines |= (u32)(expo_reg[0].data&0xff)<<8;
    lines |= (u32)(expo_reg[1].data&0xff)<<0;
    // lines >>= 4;
    // *us = (lines+dummy) * params->expo.us_per_line;
    *us = lines;//(lines*Preview_line_period);
    SENSOR_DMSG("[%s] sensor expo lines/us 0x%x,0x%x us\n", __FUNCTION__, lines, *us);
#endif
    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
#if 0
    u32 lines = 0, vts = 0;
    tp2855_params *params = (tp2855_params *)handle->private_data;

    return 0; //test only

    lines=(1000*us)/Preview_line_period;
    if (lines<1) lines=1;
    if (lines >params->expo.vts-1)
    {
        vts = lines +1;
    }
    else
    vts=params->expo.vts;

    // lines=us/Preview_line_period;
    SENSOR_DMSG("[%s] us %u, lines %u, vts %u\n", __FUNCTION__,us,lines,params->expo.vts);
    lines=vts-lines-1;
    //expo_reg[0].data = (lines>>16) & 0x0003;
    expo_reg[0].data = (lines>>8) & 0x00ff;
    expo_reg[1].data = (lines>>0) & 0x00ff;

    // vts_reg[0].data = (vts >> 16) & 0x0003;
    vts_reg[0].data = (vts >> 8) & 0x00ff;
    vts_reg[1].data = (vts >> 0) & 0x00ff;

    SensorReg_Write(0x3001,1);
    SensorRegArrayW((I2C_ARRAY*)expo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
    SensorRegArrayW((I2C_ARRAY*)gain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
    SensorRegArrayW((I2C_ARRAY*)vts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
    //SensorReg_Write(0x3001,0);
    params->reg_dirty = true; //reg need to update
#endif
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
#if 0
    //int rc = SensorRegArrayR((I2C_ARRAY*)gain_reg, ARRAY_SIZE(gain_reg));
    u16 temp_gain;
    double temp_gain_double;

    temp_gain=gain_reg[0].data;
    temp_gain_double=((double)(temp_gain*3)/200.0f);
    *gain=(u32)(pow(10,temp_gain_double)*1024);

    SENSOR_DMSG("[%s] get gain/reg (1024=1X)= 0x%x/0x%x\n", __FUNCTION__, *gain,gain_reg[0].data);
#endif
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
#if 0
    //extern DBG_ITEM Dbg_Items[DBG_TAG_MAX];
    tp2855_params *params = (tp2855_params *)handle->private_data;
    u32 i;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    //double gain_double;
    // u32 times = log2((double)gain/1024.0f)/log(2);

    return 0; //test only

    if (gain<1024)
    gain=1024;
    else if (gain>=177*1024)
    gain=177*1024;

    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate)/sizeof(CUS_GAIN_GAP_ARRAY); i++)
    {
        //SENSOR_DMSG("GAP:%x %x\r\n",Sensor_Gain_Linearity[i].gain, Sensor_Gain_Linearity[i].offset);

        if (Sensor_Gain_Linearity[i].gain == 0)
            break;
        if ((gain>Sensor_Gain_Linearity[i].gain) && (gain < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset)))
        {
            gain=Sensor_Gain_Linearity[i].gain;
            break;
        }
    }

    gain = (gain * handle->sat_mingain+512)>>10;
    params->expo.final_gain = gain;
    //gain_double = 20*log10((double)gain/1024);
    //gain_reg[0].data=(u16)((gain_double*10)/3);
    // params->reg_dirty = true;    //reg need to update
    SENSOR_DMSG("[%s] set gain/reg=0x%x/0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
#endif
    return SUCCESS;
}

static int tp2855_cus_camsensor_init_res(ss_cus_sensor* handle, int w, int h, int fps)
{
    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].u16width = w;
    handle->video_res_supported.res[0].u16height = h; // Fixme : TOTAL_LINE_CNT not enough to 1080
    handle->video_res_supported.res[0].u16max_fps = fps;
    handle->video_res_supported.res[0].u16min_fps = fps;
    handle->video_res_supported.res[0].u16crop_start_x = Preview_CROP_START_X;
    handle->video_res_supported.res[0].u16crop_start_y = Preview_CROP_START_Y;
    handle->video_res_supported.res[0].u16OutputWidth = w;
    handle->video_res_supported.res[0].u16OutputHeight = h;
    sprintf(handle->video_res_supported.res[0].strResDesc, "%dx%d@%dfps", w, h ,fps);

    return SUCCESS;
}

static int pCus_Anadec_GetSrcAttr(ss_cus_sensor *handle, u32 plane_id, CUS_SNR_Anadec_SrcAttr_t *psrcAttr)
{
    unsigned short try_cnt = 0;
    unsigned short tmp1 = 0, tmp2 = 0;
    CUS_SNR_Anadec_SrcAttr_t last_src_attr;

    switch (plane_id) {
    case 0:
        SensorReg_Write(0x40, CH_1);
    break;
    case 1:
        SensorReg_Write(0x40, CH_2);
    break;
    case 2:
        SensorReg_Write(0x40, CH_3);
    break;
    case 3:
        SensorReg_Write(0x40, CH_4);
    break;
    default:
    psrcAttr->eStatus = CUS_SNR_ANADEC_STATUS_DISCNT;
    return SUCCESS;
    }

    memset(&last_src_attr, 0x0, sizeof(CUS_SNR_Anadec_SrcAttr_t));

// 7.1.2 Video Input Status Register
#define CDET       (BIT(0))
#define NINTL      (BIT(1))
#define EQDET      (BIT(2))
#define VDET       (BIT(3))
#define SLOCK      (BIT(4))
#define HLOCK      (BIT(5))
#define VLOCK      (BIT(6))
#define VD_LOSS    (BIT(7))

// 7.1.4 Detection Status Register
#define CVSTD      (BIT(0) | BIT(1) | BIT(2))
#define SYWD       (BIT(3))

L_RETRY:
    SensorReg_Read(0x01, &tmp1);
    SENSOR_DMSG("[TP2855] VC [%d] Video Input Status [%x]\n", plane_id, tmp1);
    if ((tmp1 & VDET) /* && (tmp1 & SLOCK) && (tmp1 & HLOCK) && (tmp1 & VLOCK) */) {
        // Video detected
        SensorReg_Read(0x03, &tmp2);
        SENSOR_DMSG("[TP2855] VC [%d] Detection Status [%x]\n", plane_id, tmp2);

        psrcAttr->eStatus = CUS_SNR_ANADEC_STATUS_CONNECT;
        switch (tmp2 & CVSTD) {
        case 0: // 720p/60
            {
                psrcAttr->width = 1280;
                psrcAttr->height = 720;
                psrcAttr->u32Fps = 60;
            }
            break;
        case 1: // 720p/50
            {
                psrcAttr->width = 1280;
                psrcAttr->height = 720;
                psrcAttr->u32Fps = 50;
            }
            break;
        case 2: // 1080p/30
            {
                psrcAttr->width = 1920;
                psrcAttr->height = 1080;
                psrcAttr->u32Fps = 30;
            }
            break;
        case 3: // 1080p/25
            {
                psrcAttr->width = 1920;
                psrcAttr->height = 1080;
                psrcAttr->u32Fps = 25;
            }
            break;
        case 4: // 720p/30
            {
                psrcAttr->width = 1280;
                psrcAttr->height = 720;
                psrcAttr->u32Fps = 30;
            }
            break;
        case 5: // 720p/25
            {
                psrcAttr->width = 1280;
                psrcAttr->height = 720;
                psrcAttr->u32Fps = 25;
            }
            break;
        case 6: // SD
        case 7: // Other formats
        default:
            psrcAttr->eStatus = CUS_SNR_ANADEC_STATUS_DISCNT;
            break;
        }


        switch ((tmp2 & SYWD) >> 3) {
        case 0:
            psrcAttr->eTransferMode = CUS_SNR_ANADEC_TRANSFERMODE_TVI;
            break;
        case 1:
            psrcAttr->eTransferMode = CUS_SNR_ANADEC_TRANSFERMODE_AHD;
            break;
        default:
            psrcAttr->eTransferMode = CUS_SNR_ANADEC_TRANSFERMODE_TVI;
            break;
        }
    } else {
        // No video
        psrcAttr->eStatus = CUS_SNR_ANADEC_STATUS_DISCNT;
    }

    if (last_src_attr.eStatus != psrcAttr->eStatus
        || last_src_attr.width != psrcAttr->width
        || last_src_attr.height != psrcAttr->height
        || last_src_attr.u32Fps != psrcAttr->u32Fps) {
        memcpy(&last_src_attr, psrcAttr, sizeof(CUS_SNR_Anadec_SrcAttr_t));
        try_cnt = 0;
        goto L_RETRY;
    }

    if (last_src_attr.eStatus == psrcAttr->eStatus
        && last_src_attr.width == psrcAttr->width
        && last_src_attr.height == psrcAttr->height
        && last_src_attr.u32Fps == psrcAttr->u32Fps) {
        try_cnt ++;
    }

    if (try_cnt < 2)
        goto L_RETRY;

    return SUCCESS;
}

static int pCus_Anadec_SetSrcAttr(ss_cus_sensor *handle, u32 plane_id, CUS_SNR_Anadec_SrcAttr_t *psrcAttr)
{
    if (psrcAttr && plane_id < 4) {
        if (psrcAttr->width == 1920 && psrcAttr->height == 1080) {
            if (psrcAttr->u32Fps == 25)
                fmt[plane_id] = FHD25;
            else if (psrcAttr->u32Fps == 30)
                fmt[plane_id] = FHD30;
            else if (psrcAttr->u32Fps == 55)
                fmt[plane_id] = FHD50;
            else if (psrcAttr->u32Fps == 60)
                fmt[plane_id] = FHD60;
        } else if (psrcAttr->width == 1280 && psrcAttr->height == 720) {
            if (psrcAttr->u32Fps == 25)
                fmt[plane_id] = HD25;
            else if (psrcAttr->u32Fps == 30)
                fmt[plane_id] = HD30;
        } else if (psrcAttr->width == 2560 && psrcAttr->height == 1440) {
            if (psrcAttr->u32Fps == 25)
                fmt[plane_id] = QHD25;
            else if (psrcAttr->u32Fps == 30)
                fmt[plane_id] = QHD30;
        } else if (psrcAttr->width == 1280 && psrcAttr->height == 960) {
            if (psrcAttr->u32Fps == 25)
                fmt[plane_id] = UVGA25;
            else if (psrcAttr->u32Fps == 30)
                fmt[plane_id] = UVGA30;
        } else {
            SENSOR_EMSG("tp2855 not support resolution\n");
            return SUCCESS;
        }

        tp2855_cus_camsensor_init_res(handle, psrcAttr->width, psrcAttr->height, psrcAttr->u32Fps);

        if (psrcAttr->eTransferMode == CUS_SNR_ANADEC_TRANSFERMODE_TVI) {
            std[plane_id] = STD_TVI;
        } else if (psrcAttr->eTransferMode == CUS_SNR_ANADEC_TRANSFERMODE_AHD) {
            std[plane_id] = STD_HDA;
        } else {
            SENSOR_EMSG("tp2855 not support mode\n");
            return SUCCESS;
        }
    }
    return SUCCESS;
}

int tp2855_cus_camsensor_init_handle_ch0(ss_cus_sensor* handle)
{
    tp2855_params *params;
    if (!handle)
    {
        SENSOR_EMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (tp2855_params *)handle->private_data;

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "tp2855_MIPI_CH0");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    tp2855_cus_camsensor_init_res(handle, 1920, 1080, 25);

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //200000;


    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;


    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/25;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    ///calibration
    handle->pCus_sensor_init        = pCus_init    ;
    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    handle->pCus_sensor_GetVideoResNum    = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes    = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode    = tp2855_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify   = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs       = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs       = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain        = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain        = pCus_SetAEGain;

    //Src type
    handle->pCus_sensor_GetSrcAttr       = pCus_Anadec_GetSrcAttr;
    handle->pCus_sensor_SetSrcAttr       = pCus_Anadec_SetSrcAttr;
    handle->pCus_sensor_GetAEInfo        = pCus_sensor_GetAEInfo;
    //sensor calibration
    // params->expo.vts  = vts_30fps;
    // params->expo.fps  = 30;
    // params->reg_dirty = false;

    //MIPI config
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_YUV422; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = CUS_SENSOR_YUV_ORDER_CY; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;
    return SUCCESS;
}

int tp2855_cus_camsensor_init_handle_ch1(ss_cus_sensor* handle)
{
    tp2855_params *params;
    if (!handle)
    {
        SENSOR_EMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (tp2855_params *)handle->private_data;

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "tp2855_MIPI_CH1");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    tp2855_cus_camsensor_init_res(handle, 1920, 1080, 25);

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //200000;


    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/25;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    ///calibration
    handle->pCus_sensor_init        = pCus_init_dummy    ;
    handle->pCus_sensor_poweron     = pCus_poweron_dummy ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_dummy;

    // Normal
    handle->pCus_sensor_GetVideoResNum    = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes    = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode    = tp2855_SetPatternMode;
    handle->pCus_sensor_GetAEInfo         = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify   = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs       = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs       = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain        = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain        = pCus_SetAEGain;

    //Src type
    handle->pCus_sensor_GetSrcAttr       = pCus_Anadec_GetSrcAttr;
    handle->pCus_sensor_SetSrcAttr       = pCus_Anadec_SetSrcAttr;

    //sensor calibration
    // params->expo.vts  = vts_30fps;
    // params->expo.fps  = 30;
    // params->reg_dirty = false;

    //MIPI config
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_YUV422; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = CUS_SENSOR_YUV_ORDER_CY; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1;
    return SUCCESS;
}

int tp2855_cus_camsensor_init_handle_ch2(ss_cus_sensor* handle)
{
    tp2855_params *params;
    if (!handle)
    {
        SENSOR_EMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (tp2855_params *)handle->private_data;

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "tp2855_MIPI_CH2");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    tp2855_cus_camsensor_init_res(handle, 1920, 1080, 25);

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //200000;


    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/25;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;

    ///calibration
    handle->pCus_sensor_init        = pCus_init_dummy    ;
    handle->pCus_sensor_poweron     = pCus_poweron_dummy ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_dummy;

    // Normal
    handle->pCus_sensor_GetVideoResNum    = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes    = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode    = tp2855_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify   = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs       = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs       = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain        = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain        = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo        = pCus_sensor_GetAEInfo;
    //Src type
    handle->pCus_sensor_GetSrcAttr       = pCus_Anadec_GetSrcAttr;
    handle->pCus_sensor_SetSrcAttr       = pCus_Anadec_SetSrcAttr;

    //sensor calibration
    // params->expo.vts  = vts_30fps;
    // params->expo.fps  = 30;
    // params->reg_dirty = false;

    //MIPI config
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_YUV422; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = CUS_SENSOR_YUV_ORDER_CY; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 2;
    return SUCCESS;
}

int tp2855_cus_camsensor_init_handle_ch3(ss_cus_sensor* handle)
{
    tp2855_params *params;
    if (!handle)
    {
        SENSOR_EMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]\n", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (tp2855_params *)handle->private_data;

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "tp2855_MIPI_CH3");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    tp2855_cus_camsensor_init_res(handle, 1920, 1080, 25);

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //200000;


    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/25;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->pCus_sensor_GetAEInfo        = pCus_sensor_GetAEInfo;
    ///calibration
    handle->pCus_sensor_init        = pCus_init_dummy    ;
    handle->pCus_sensor_poweron     = pCus_poweron_dummy ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_dummy;

    // Normal
    handle->pCus_sensor_GetVideoResNum    = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes    = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode    = tp2855_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify   = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs       = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs       = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain        = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain        = pCus_SetAEGain;
    //Src type
    handle->pCus_sensor_GetSrcAttr       = pCus_Anadec_GetSrcAttr;
    handle->pCus_sensor_SetSrcAttr       = pCus_Anadec_SetSrcAttr;

    //sensor calibration
    // params->expo.vts  = vts_30fps;
    // params->expo.fps  = 30;
    // params->reg_dirty = false;

    //MIPI config
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_YUV422; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = CUS_SENSOR_YUV_ORDER_CY; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 3;
    return SUCCESS;
}

SENSOR_DRV_ENTRY_4CHAHD_IMPL_END_EX(tp2855,
                                    tp2855_cus_camsensor_init_handle_ch0,
                                    tp2855_cus_camsensor_init_handle_ch0,
                                    tp2855_cus_camsensor_init_handle_ch1,
                                    tp2855_cus_camsensor_init_handle_ch2,
                                    tp2855_cus_camsensor_init_handle_ch3,
                                    tp2855_params
                                    );
