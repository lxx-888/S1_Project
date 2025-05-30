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

#ifndef _SC431HAI_INIT_TABLE_H_

#define _SC431HAI_PDWN_ON_ 0
#define _SC431HAI_RST_ON_ 0

//Linear
#define _SC431HAI_RES_4M30_ 1
//HDR
#define _SC431HAI_RES_4M25_HDR 1

#if _SC431HAI_RES_4M30_
	static SENSOR_INIT_TABLE Sensor_init_table_4M30[] __attribute__((aligned(8)))=
	{
		#if defined(SENSOR_INIT_CMDQ_MODE)

			/*set cmdq0 busy*/
			CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
			VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

			/*sensor reset*/
			SNR_RST(_SC431HAI_RST_ON_),       //reset  off
			CMDQ_DELAY_MS(1),            // delay 1ms
			SNR_PDWN(_SC431HAI_PDWN_ON_),     //power off
			CMDQ_DELAY_MS(1),            // delay 1ms
			SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0x7=24MHz
			SNR_PDWN(~_SC431HAI_PDWN_ON_),    //power on
			CMDQ_DELAY_MS(1),            // delay 1ms
			SNR_RST(~_SC431HAI_RST_ON_),      //reset on
			CMDQ_DELAY_MS(4),            // delay 1ms
			//SNR_MCLK_EN(0x4),               //0x0=MCLK 27MhZ, 0xB=37.125MHz

			/*I2C timing*/
			//I2CM_CLK_EN(0x2),  //12MHz
			/*
			I2CM_REGW(reg_lcnt,16),
			I2CM_REGW(reg_hcnt,11),
			I2CM_REGW(reg_start_cnt,50),
			I2CM_REGW(reg_stop_cnt,50),
			I2CM_REGW(reg_data_lat_cnt,0),
			I2CM_REGW(reg_sda_cnt,11),
			*/
			// I2CM_PARAM(16,11,50,50,0,11),  //from sc4238
			//I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
			// I2CM_PARAM(0x39,0x38,0x71,0x71,0x1C,0x1C), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
			// I2CM_PARAM(0x1b,0x1b,0x3c,0x3c,0x0f,0x0f), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt

		#endif
        I2CM_2A1D_W(0x60,0x0103,0x01),
		//I2CM_2A1D_W(0x60,0x0100,0x00),
		I2CM_2A1D_W(0x60,0x36e9,0x80),
		I2CM_2A1D_W(0x60,0x37f9,0x80),
		I2CM_2A1D_W(0x60,0x3018,0x3a),
		I2CM_2A1D_W(0x60,0x3019,0x0c),
		I2CM_2A1D_W(0x60,0x301f,0x05),
		I2CM_2A1D_W(0x60,0x3058,0x21),
		I2CM_2A1D_W(0x60,0x3059,0x53),
		I2CM_2A1D_W(0x60,0x305a,0x40),
		I2CM_2A1D_W(0x60,0x3250,0x00),
		I2CM_2A1D_W(0x60,0x3301,0x0c),
		I2CM_2A1D_W(0x60,0x3304,0x50),
		I2CM_2A1D_W(0x60,0x3305,0x00),
		I2CM_2A1D_W(0x60,0x3306,0x50),
		I2CM_2A1D_W(0x60,0x3307,0x04),
		I2CM_2A1D_W(0x60,0x3308,0x0a),
		I2CM_2A1D_W(0x60,0x3309,0x60),
		I2CM_2A1D_W(0x60,0x330b,0xc8),
		I2CM_2A1D_W(0x60,0x330d,0x08),
		I2CM_2A1D_W(0x60,0x330e,0x38),
		I2CM_2A1D_W(0x60,0x331e,0x41),
		I2CM_2A1D_W(0x60,0x331f,0x51),
		I2CM_2A1D_W(0x60,0x3333,0x10),
		I2CM_2A1D_W(0x60,0x3334,0x40),
		I2CM_2A1D_W(0x60,0x3364,0x5e),
		I2CM_2A1D_W(0x60,0x338e,0xe2),
		I2CM_2A1D_W(0x60,0x338f,0x80),
		I2CM_2A1D_W(0x60,0x3390,0x08),
		I2CM_2A1D_W(0x60,0x3391,0x18),
		I2CM_2A1D_W(0x60,0x3392,0xb8),
		I2CM_2A1D_W(0x60,0x3393,0x12),
		I2CM_2A1D_W(0x60,0x3394,0x14),
		I2CM_2A1D_W(0x60,0x3395,0x10),
		I2CM_2A1D_W(0x60,0x3396,0x88),
		I2CM_2A1D_W(0x60,0x3397,0x98),
		I2CM_2A1D_W(0x60,0x3398,0xb8),
		I2CM_2A1D_W(0x60,0x3399,0x10),
		I2CM_2A1D_W(0x60,0x339a,0x16),
		I2CM_2A1D_W(0x60,0x339b,0x1c),
		I2CM_2A1D_W(0x60,0x339c,0x40),
		I2CM_2A1D_W(0x60,0x33ac,0x0a),
		I2CM_2A1D_W(0x60,0x33ad,0x10),
		I2CM_2A1D_W(0x60,0x33ae,0x4f),
		I2CM_2A1D_W(0x60,0x33af,0x5e),
		I2CM_2A1D_W(0x60,0x33b2,0x50),
		I2CM_2A1D_W(0x60,0x33b3,0x10),
		I2CM_2A1D_W(0x60,0x33f8,0x00),
		I2CM_2A1D_W(0x60,0x33f9,0x50),
		I2CM_2A1D_W(0x60,0x33fa,0x00),
		I2CM_2A1D_W(0x60,0x33fb,0x50),
		I2CM_2A1D_W(0x60,0x33fc,0x48),
		I2CM_2A1D_W(0x60,0x33fd,0x78),
		I2CM_2A1D_W(0x60,0x349f,0x03),
		I2CM_2A1D_W(0x60,0x34a6,0x40),
		I2CM_2A1D_W(0x60,0x34a7,0x58),
		I2CM_2A1D_W(0x60,0x34a8,0x10),
		I2CM_2A1D_W(0x60,0x34a9,0x10),
		I2CM_2A1D_W(0x60,0x34f8,0x78),
		I2CM_2A1D_W(0x60,0x34f9,0x10),
		I2CM_2A1D_W(0x60,0x3633,0x44),
		I2CM_2A1D_W(0x60,0x363b,0x8f),
		I2CM_2A1D_W(0x60,0x363c,0x02),
		I2CM_2A1D_W(0x60,0x3641,0x08),
		I2CM_2A1D_W(0x60,0x3654,0x20),
		I2CM_2A1D_W(0x60,0x3674,0xc2),
		I2CM_2A1D_W(0x60,0x3675,0xb4),
		I2CM_2A1D_W(0x60,0x3676,0x88),
		I2CM_2A1D_W(0x60,0x367c,0x88),
		I2CM_2A1D_W(0x60,0x367d,0xb8),
		I2CM_2A1D_W(0x60,0x3690,0x34),
		I2CM_2A1D_W(0x60,0x3691,0x44),
		I2CM_2A1D_W(0x60,0x3692,0x54),
		I2CM_2A1D_W(0x60,0x3693,0x88),
		I2CM_2A1D_W(0x60,0x3694,0x98),
		I2CM_2A1D_W(0x60,0x3696,0x80),
		I2CM_2A1D_W(0x60,0x3697,0x83),
		I2CM_2A1D_W(0x60,0x3698,0x81),
		I2CM_2A1D_W(0x60,0x3699,0x81),
		I2CM_2A1D_W(0x60,0x369a,0x84),
		I2CM_2A1D_W(0x60,0x369b,0x82),
		I2CM_2A1D_W(0x60,0x36a2,0x80),
		I2CM_2A1D_W(0x60,0x36a3,0x88),
		I2CM_2A1D_W(0x60,0x36a4,0xf8),
		I2CM_2A1D_W(0x60,0x36a5,0xb8),
		I2CM_2A1D_W(0x60,0x36a6,0x98),
		I2CM_2A1D_W(0x60,0x36d0,0x15),
		I2CM_2A1D_W(0x60,0x36ec,0x55),
		I2CM_2A1D_W(0x60,0x36ed,0x18),
		I2CM_2A1D_W(0x60,0x370f,0x01),
		I2CM_2A1D_W(0x60,0x3722,0x03),
		I2CM_2A1D_W(0x60,0x3724,0x92),
		I2CM_2A1D_W(0x60,0x3727,0x14),
		I2CM_2A1D_W(0x60,0x37b0,0x17),
		I2CM_2A1D_W(0x60,0x37b1,0x9b),
		I2CM_2A1D_W(0x60,0x37b2,0x9b),
		I2CM_2A1D_W(0x60,0x37b3,0x88),
		I2CM_2A1D_W(0x60,0x37b4,0xb8),
		I2CM_2A1D_W(0x60,0x37fa,0x23),
		I2CM_2A1D_W(0x60,0x37fb,0x54),
		I2CM_2A1D_W(0x60,0x37fc,0x21),
		I2CM_2A1D_W(0x60,0x391f,0x41),
		I2CM_2A1D_W(0x60,0x3926,0xe0),
		I2CM_2A1D_W(0x60,0x3933,0x80),
		I2CM_2A1D_W(0x60,0x3934,0xf8),
		I2CM_2A1D_W(0x60,0x3935,0x00),
		I2CM_2A1D_W(0x60,0x3936,0x45),
		I2CM_2A1D_W(0x60,0x3937,0x66),
		I2CM_2A1D_W(0x60,0x3938,0x66),
		I2CM_2A1D_W(0x60,0x3939,0x00),
		I2CM_2A1D_W(0x60,0x393a,0x03),
		I2CM_2A1D_W(0x60,0x393b,0x00),
		I2CM_2A1D_W(0x60,0x393c,0x00),
		I2CM_2A1D_W(0x60,0x393d,0x02),
		I2CM_2A1D_W(0x60,0x393e,0x80),
		I2CM_2A1D_W(0x60,0x3e00,0x00),
		I2CM_2A1D_W(0x60,0x3e01,0xba),
		I2CM_2A1D_W(0x60,0x3e02,0xd0),
		I2CM_2A1D_W(0x60,0x3e16,0x00),
		I2CM_2A1D_W(0x60,0x3e17,0xc5),
		I2CM_2A1D_W(0x60,0x3e18,0x00),
		I2CM_2A1D_W(0x60,0x3e19,0xc5),
		I2CM_2A1D_W(0x60,0x4509,0x20),
		I2CM_2A1D_W(0x60,0x450d,0x0b),
		I2CM_2A1D_W(0x60,0x4819,0x08),
		I2CM_2A1D_W(0x60,0x481b,0x05),
		I2CM_2A1D_W(0x60,0x481d,0x11),
		I2CM_2A1D_W(0x60,0x481f,0x04),
		I2CM_2A1D_W(0x60,0x4821,0x09),
		I2CM_2A1D_W(0x60,0x4823,0x04),
		I2CM_2A1D_W(0x60,0x4825,0x04),
		I2CM_2A1D_W(0x60,0x4827,0x04),
		I2CM_2A1D_W(0x60,0x4829,0x07),
		I2CM_2A1D_W(0x60,0x5780,0x76),
		I2CM_2A1D_W(0x60,0x5784,0x0a),
		I2CM_2A1D_W(0x60,0x5785,0x04),
		I2CM_2A1D_W(0x60,0x5787,0x0a),
		I2CM_2A1D_W(0x60,0x5788,0x0a),
		I2CM_2A1D_W(0x60,0x5789,0x08),
		I2CM_2A1D_W(0x60,0x578a,0x0a),
		I2CM_2A1D_W(0x60,0x578b,0x0a),
		I2CM_2A1D_W(0x60,0x578c,0x08),
		I2CM_2A1D_W(0x60,0x578d,0x40),
		I2CM_2A1D_W(0x60,0x5790,0x08),
		I2CM_2A1D_W(0x60,0x5791,0x04),
		I2CM_2A1D_W(0x60,0x5792,0x04),
		I2CM_2A1D_W(0x60,0x5793,0x08),
		I2CM_2A1D_W(0x60,0x5794,0x04),
		I2CM_2A1D_W(0x60,0x5795,0x04),
		I2CM_2A1D_W(0x60,0x57ac,0x00),
		I2CM_2A1D_W(0x60,0x57ad,0x00),
		I2CM_2A1D_W(0x60,0x36e9,0x44),
		I2CM_2A1D_W(0x60,0x37f9,0x44),
		//I2CM_2A1D_W(0x60,0x0100,0x01),
		
		#if defined(SENSOR_INIT_CMDQ_MODE)
			/*Sensor runtime parameter*/
			//I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
			SNR_SHUTTER_FPS_2A1D(0x60, 10000, 30000),   //shutter 10ms , fps 30
			SNR_GAIN_2A1D(0x60, 1024),
			//I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end
		#endif
		I2CM_2A1D_W(0x60,0x0100,0x01),   //set manual sleep mode disable

		#if defined(SENSOR_INIT_CMDQ_MODE)
			/*set cmdq0 idle*/
			CMDQ_REG_DUMMY(CMDQ_STATUS_READY),
			VIF_REG_DUMMY(CMDQ_STATUS_READY),

			/*append dummy*/
			//CMDQ_TAG('EOT'),    //end of table
			CMDQ_NULL(),
			CMDQ_NULL(),
			CMDQ_NULL(),
			CMDQ_NULL(),
		#endif
	};
#endif //end of _SC431HAI_RES_4M30_

#if _SC431HAI_RES_4M25_HDR
	static SENSOR_INIT_TABLE Sensor_init_table_4M25_hdr[] __attribute__((aligned(8)))=
	{
		#if defined(SENSOR_INIT_CMDQ_MODE)

			/*set cmdq0 busy*/
			CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
			VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

			/*sensor reset*/
			SNR_RST(_SC431HAI_RST_ON_),       //reset  off
			CMDQ_DELAY_MS(1),            // delay 1ms
			SNR_PDWN(_SC431HAI_PDWN_ON_),     //power off
			CMDQ_DELAY_MS(1),            // delay 1ms
			SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0x7=24MHz
			SNR_PDWN(~_SC431HAI_PDWN_ON_),    //power on
			CMDQ_DELAY_MS(1),            // delay 1ms
			SNR_RST(~_SC431HAI_RST_ON_),      //reset on
			CMDQ_DELAY_MS(4),            // delay 1ms
			//SNR_MCLK_EN(0x4),               //0x0=MCLK 27MhZ, 0xB=37.125MHz

			/*I2C timing*/
			//I2CM_CLK_EN(0x2),  //12MHz
			/*
			I2CM_REGW(reg_lcnt,16),
			I2CM_REGW(reg_hcnt,11),
			I2CM_REGW(reg_start_cnt,50),
			I2CM_REGW(reg_stop_cnt,50),
			I2CM_REGW(reg_data_lat_cnt,0),
			I2CM_REGW(reg_sda_cnt,11),
			*/
			// I2CM_PARAM(16,11,50,50,0,11),  //from sc4238
			//I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
			// I2CM_PARAM(0x39,0x38,0x71,0x71,0x1C,0x1C), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
			// I2CM_PARAM(0x1b,0x1b,0x3c,0x3c,0x0f,0x0f), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt

		#endif

        I2CM_2A1D_W(0x60,0x0103,0x01),
		//I2CM_2A1D_W(0x60,0x0100,0x00),
		I2CM_2A1D_W(0x60,0x36e9,0x80),
		I2CM_2A1D_W(0x60,0x37f9,0x80),
		I2CM_2A1D_W(0x60,0x3018,0x3a),
		I2CM_2A1D_W(0x60,0x3019,0x0c),
		I2CM_2A1D_W(0x60,0x301f,0x09),
		I2CM_2A1D_W(0x60,0x3058,0x21),
		I2CM_2A1D_W(0x60,0x3059,0x53),
		I2CM_2A1D_W(0x60,0x305a,0x40),
		I2CM_2A1D_W(0x60,0x320c,0x05),
		I2CM_2A1D_W(0x60,0x320d,0xa0),
		I2CM_2A1D_W(0x60,0x320e,0x0b),
		I2CM_2A1D_W(0x60,0x320f,0xb8),
		I2CM_2A1D_W(0x60,0x3250,0x00),
		I2CM_2A1D_W(0x60,0x3281,0x01),
		I2CM_2A1D_W(0x60,0x3301,0x12),
		I2CM_2A1D_W(0x60,0x3304,0x50),
		I2CM_2A1D_W(0x60,0x3305,0x00),
		I2CM_2A1D_W(0x60,0x3306,0x68),
		I2CM_2A1D_W(0x60,0x3307,0x04),
		I2CM_2A1D_W(0x60,0x3308,0x0a),
		I2CM_2A1D_W(0x60,0x3309,0x88),
		I2CM_2A1D_W(0x60,0x330b,0xd8),
		I2CM_2A1D_W(0x60,0x330d,0x08),
		I2CM_2A1D_W(0x60,0x330e,0x28),
		I2CM_2A1D_W(0x60,0x331e,0x41),
		I2CM_2A1D_W(0x60,0x331f,0x79),
		I2CM_2A1D_W(0x60,0x3333,0x10),
		I2CM_2A1D_W(0x60,0x3334,0x40),
		I2CM_2A1D_W(0x60,0x3364,0x5e),
		I2CM_2A1D_W(0x60,0x338e,0xe2),
		I2CM_2A1D_W(0x60,0x338f,0x80),
		I2CM_2A1D_W(0x60,0x3390,0x08),
		I2CM_2A1D_W(0x60,0x3391,0x18),
		I2CM_2A1D_W(0x60,0x3392,0xb8),
		I2CM_2A1D_W(0x60,0x3393,0x12),
		I2CM_2A1D_W(0x60,0x3394,0x14),
		I2CM_2A1D_W(0x60,0x3395,0x10),
		I2CM_2A1D_W(0x60,0x3396,0x88),
		I2CM_2A1D_W(0x60,0x3397,0x98),
		I2CM_2A1D_W(0x60,0x3398,0xb8),
		I2CM_2A1D_W(0x60,0x3399,0x0e),
		I2CM_2A1D_W(0x60,0x339a,0x18),
		I2CM_2A1D_W(0x60,0x339b,0x20),
		I2CM_2A1D_W(0x60,0x339c,0x24),
		I2CM_2A1D_W(0x60,0x33ac,0x0a),
		I2CM_2A1D_W(0x60,0x33ad,0x10),
		I2CM_2A1D_W(0x60,0x33ae,0x4c),
		I2CM_2A1D_W(0x60,0x33af,0x84),
		I2CM_2A1D_W(0x60,0x33b2,0x50),
		I2CM_2A1D_W(0x60,0x33b3,0x10),
		I2CM_2A1D_W(0x60,0x33f8,0x00),
		I2CM_2A1D_W(0x60,0x33f9,0x68),
		I2CM_2A1D_W(0x60,0x33fa,0x00),
		I2CM_2A1D_W(0x60,0x33fb,0x68),
		I2CM_2A1D_W(0x60,0x33fc,0x48),
		I2CM_2A1D_W(0x60,0x33fd,0x78),
		I2CM_2A1D_W(0x60,0x349f,0x03),
		I2CM_2A1D_W(0x60,0x34a6,0x40),
		I2CM_2A1D_W(0x60,0x34a7,0x58),
		I2CM_2A1D_W(0x60,0x34a8,0x10),
		I2CM_2A1D_W(0x60,0x34a9,0x10),
		I2CM_2A1D_W(0x60,0x34f8,0x78),
		I2CM_2A1D_W(0x60,0x34f9,0x10),
		I2CM_2A1D_W(0x60,0x3633,0x44),
		I2CM_2A1D_W(0x60,0x363b,0x8f),
		I2CM_2A1D_W(0x60,0x363c,0x02),
		I2CM_2A1D_W(0x60,0x3641,0x08),
		I2CM_2A1D_W(0x60,0x3654,0x20),
		I2CM_2A1D_W(0x60,0x3674,0xc2),
		I2CM_2A1D_W(0x60,0x3675,0xb4),
		I2CM_2A1D_W(0x60,0x3676,0x88),
		I2CM_2A1D_W(0x60,0x367c,0x88),
		I2CM_2A1D_W(0x60,0x367d,0xb8),
		I2CM_2A1D_W(0x60,0x3690,0x34),
		I2CM_2A1D_W(0x60,0x3691,0x45),
		I2CM_2A1D_W(0x60,0x3692,0x55),
		I2CM_2A1D_W(0x60,0x3693,0x88),
		I2CM_2A1D_W(0x60,0x3694,0xb8),
		I2CM_2A1D_W(0x60,0x3696,0x80),
		I2CM_2A1D_W(0x60,0x3697,0x83),
		I2CM_2A1D_W(0x60,0x3698,0x81),
		I2CM_2A1D_W(0x60,0x3699,0x81),
		I2CM_2A1D_W(0x60,0x369a,0x84),
		I2CM_2A1D_W(0x60,0x369b,0x82),
		I2CM_2A1D_W(0x60,0x36a2,0x80),
		I2CM_2A1D_W(0x60,0x36a3,0x88),
		I2CM_2A1D_W(0x60,0x36a4,0xf8),
		I2CM_2A1D_W(0x60,0x36a5,0xb8),
		I2CM_2A1D_W(0x60,0x36a6,0x98),
		I2CM_2A1D_W(0x60,0x36d0,0x15),
		I2CM_2A1D_W(0x60,0x36ea,0x0a),
		I2CM_2A1D_W(0x60,0x36eb,0x0c),
		I2CM_2A1D_W(0x60,0x36ec,0x45),
		I2CM_2A1D_W(0x60,0x36ed,0x18),
		I2CM_2A1D_W(0x60,0x370f,0x01),
		I2CM_2A1D_W(0x60,0x3722,0x03),
		I2CM_2A1D_W(0x60,0x3724,0x92),
		I2CM_2A1D_W(0x60,0x3727,0x14),
		I2CM_2A1D_W(0x60,0x37b0,0x17),
		I2CM_2A1D_W(0x60,0x37b1,0x9b),
		I2CM_2A1D_W(0x60,0x37b2,0x9b),
		I2CM_2A1D_W(0x60,0x37b3,0x88),
		I2CM_2A1D_W(0x60,0x37b4,0xb8),
		I2CM_2A1D_W(0x60,0x37fa,0x0a),
		I2CM_2A1D_W(0x60,0x37fb,0x44),
		I2CM_2A1D_W(0x60,0x37fc,0x20),
		I2CM_2A1D_W(0x60,0x37fd,0x1c),
		I2CM_2A1D_W(0x60,0x391f,0x41),
		I2CM_2A1D_W(0x60,0x3926,0xe0),
		I2CM_2A1D_W(0x60,0x3933,0x80),
		I2CM_2A1D_W(0x60,0x3934,0xf8),
		I2CM_2A1D_W(0x60,0x3935,0x00),
		I2CM_2A1D_W(0x60,0x3936,0x2d),
		I2CM_2A1D_W(0x60,0x3937,0x64),
		I2CM_2A1D_W(0x60,0x3938,0x62),
		I2CM_2A1D_W(0x60,0x393d,0x02),
		I2CM_2A1D_W(0x60,0x393e,0x00),
		I2CM_2A1D_W(0x60,0x3e00,0x01),
		I2CM_2A1D_W(0x60,0x3e01,0x5c),
		I2CM_2A1D_W(0x60,0x3e02,0x00),
		I2CM_2A1D_W(0x60,0x3e04,0x15),
		I2CM_2A1D_W(0x60,0x3e05,0xc0),
		I2CM_2A1D_W(0x60,0x3e16,0x00),
		I2CM_2A1D_W(0x60,0x3e17,0xc5),
		I2CM_2A1D_W(0x60,0x3e18,0x00),
		I2CM_2A1D_W(0x60,0x3e19,0xc5),
		I2CM_2A1D_W(0x60,0x3e23,0x00),
		I2CM_2A1D_W(0x60,0x3e24,0xb8),
		I2CM_2A1D_W(0x60,0x4509,0x20),
		I2CM_2A1D_W(0x60,0x450d,0x0b),
		I2CM_2A1D_W(0x60,0x4816,0x71),
		I2CM_2A1D_W(0x60,0x4819,0x0d),
		I2CM_2A1D_W(0x60,0x481b,0x07),
		I2CM_2A1D_W(0x60,0x481d,0x1d),
		I2CM_2A1D_W(0x60,0x481f,0x06),
		I2CM_2A1D_W(0x60,0x4821,0x0c),
		I2CM_2A1D_W(0x60,0x4823,0x07),
		I2CM_2A1D_W(0x60,0x4825,0x06),
		I2CM_2A1D_W(0x60,0x4827,0x06),
		I2CM_2A1D_W(0x60,0x4829,0x0b),
		I2CM_2A1D_W(0x60,0x5780,0x76),
		I2CM_2A1D_W(0x60,0x5784,0x0a),
		I2CM_2A1D_W(0x60,0x5785,0x04),
		I2CM_2A1D_W(0x60,0x5787,0x0a),
		I2CM_2A1D_W(0x60,0x5788,0x0a),
		I2CM_2A1D_W(0x60,0x5789,0x08),
		I2CM_2A1D_W(0x60,0x578a,0x0a),
		I2CM_2A1D_W(0x60,0x578b,0x0a),
		I2CM_2A1D_W(0x60,0x578c,0x08),
		I2CM_2A1D_W(0x60,0x578d,0x40),
		I2CM_2A1D_W(0x60,0x5790,0x08),
		I2CM_2A1D_W(0x60,0x5791,0x04),
		I2CM_2A1D_W(0x60,0x5792,0x04),
		I2CM_2A1D_W(0x60,0x5793,0x08),
		I2CM_2A1D_W(0x60,0x5794,0x04),
		I2CM_2A1D_W(0x60,0x5795,0x04),
		I2CM_2A1D_W(0x60,0x57ac,0x00),
		I2CM_2A1D_W(0x60,0x57ad,0x00),
		I2CM_2A1D_W(0x60,0x36e9,0x04),
		I2CM_2A1D_W(0x60,0x37f9,0x04),
		//I2CM_2A1D_W(0x60,0x0100,0x01),
		
		#if defined(SENSOR_INIT_CMDQ_MODE)
			/*Sensor runtime parameter*/
			//I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
			SNR_SHUTTER_FPS_2A1D(0x60, 10000, 30000),   //shutter 10ms , fps 30
			SNR_GAIN_2A1D(0x60, 1024),
			//I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end
		#endif
		I2CM_2A1D_W(0x60,0x0100,0x01),   //set manual sleep mode disable

		#if defined(SENSOR_INIT_CMDQ_MODE)
			/*set cmdq0 idle*/
			CMDQ_REG_DUMMY(CMDQ_STATUS_READY),
			VIF_REG_DUMMY(CMDQ_STATUS_READY),

			/*append dummy*/
			//CMDQ_TAG('EOT'),    //end of table
			CMDQ_NULL(),
			CMDQ_NULL(),
			CMDQ_NULL(),
			CMDQ_NULL(),
		#endif
	};
#endif //end of _SC431HAI_RES_4M25_HDR

#endif
