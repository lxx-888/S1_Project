/*
 * hal_emac.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef __HAL_EMAC__
#define __HAL_EMAC__

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
////  Define Enable for use padmux driver
////-------------------------------------------------------------------------------------------------
#define EMAC_PADMUX_ENABLE

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
// MAC
#define TX_DELAY_INT                1 // 1:USE Hadrware Tx Delay  0:USE Hadrware TCOM
#define TX_DELAY_INT_DEBUG          0 // Do not open it during normal use
#define RX_DELAY_INT_DEBUG          0 // Do not open it during normal use
#define HW_FLOW_CONTROL             0 // 1:USE Hadrware Flow Control  0:USE Software Flow Control
#define HW_FLOW_CONTROL_DEBUG       0 // enable debug node for HW flow control debug this define depend on HW_FLOW_CONTROL
#define ETHTOOL_DEBUG               1 // eth status and the switch of flowctrol
#define NETWORK_STORM_PROTECT       1 // 1:enable network storm protect 0:disable
#define NETWORK_STORM_PROTECT_DEBUG 0 // en network storm protect debug nod
#define EAMC_RX_CHECKSUM_DEBUG      0 // enable debug for HW RX CHECKSUM
#define VLAD_TAG_TEST_DEBUG         0 // vlan tag nod test
// PHY
#define PHY_REGISTER_DEBUG 1 // en phy_r phy_w nod
#define PHY_LED_CTL        0 // SW led ctrl
#define PHY_LED_INVERT     0 // Switch led from high-level lighting to low-level lighting
#define PHY_MDI_CTL        0 // MDI/X auto switch
#define PHY_FORCE_NEG_SET  1 // internal phy force negotiation mode reg set
// Capture
#define EMAC_DEBUG_RX    0 // Used to capture rx packets at the link layer
#define EMAC_DEBUG_TX    0 // Used to capture tx packets at the link layer
#define EMAC_PERFORMANCE 0 // Used to calculate the link layer traffic
// Verify
#define EMAC_LOCAL_TEST 0 // Used to verify test subfunction
#define EXT_PHY_PATCH   1

#define SOFTWARE_DESCRIPTOR
#define RX_CHECKSUM 1
#define TX_CHECKSUM 0
//#define LAN_ESD_CARRIER_INTERRUPT

/*
#define EMAC_FLOW_CONTROL_TX    EMAC_FLOW_CONTROL_TX_NA
#define EMAC_FLOW_CONTROL_TX_NA 0
#define EMAC_FLOW_CONTROL_TX_SW 1
#define EMAC_FLOW_CONTROL_TX_HW 2

#define EMAC_FLOW_CONTROL_RX    0
*/

/*
#ifdef NEW_TX_QUEUE_INTERRUPT_THRESHOLD
// #define EMAC_INT_MASK       (0x07000dff)
#else
#define EMAC_INT_MASK       (0x00000dff)
#endif
*/

// Compiler Switches
#define URANUS_ETHER_ADDR_CONFIGURABLE /* MAC address can be changed? */
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define TRUE  1
#define FALSE 0

// #define REG_BANK_EFUSE                      0x0020
#define REG_BANK_CLKGEN0  0x1038
#define REG_BANK_MCMSCGP  0x1132
#define REG_BANK_SCGPCTRL 0x1133
#define REG_BANK_CHIPTOP  0x101E
#define REG_BANK_PMSLEEP  0x000E

#if 0
#define REG_BANK_EMAC0     0x1510 // 0x1020
#define REG_BANK_EMAC1     0x1511 // 0x1021
#define REG_BACK_EMAC2     0x1512 // 0x1022
#define REG_BANK_EMAC3     0x1513 // 0x1023
#define REG_BANK_X32_EMAC0 0x1A1E
#define REG_BANK_X32_EMAC2 0x1A1F
#define REG_BANK_X32_EMAC3 0x1A20
#define REG_BANK_ALBANY0   0x0031
#define REG_BANK_ALBANY1   0x0032
#define REG_BANK_ALBANY2   0x0033
#else
#define REG_BANK_EMAC0     0x0000 // 0x1510 //0x1020
#define REG_BANK_EMAC1     0x0001 // 0x1511 //0x1021
#define REG_BACK_EMAC2     0x0002 // 0x1512 //0x1022
#define REG_BANK_EMAC3     0x0003 // 0x1513 //0x1023
#define REG_BANK_X32_EMAC0 0x0000 // 0x1A1E
#define REG_BANK_X32_EMAC2 0x0001 // 0x1A1F
#define REG_BANK_X32_EMAC3 0x0002 // 0x1A20
#define REG_BANK_ALBANY0   0x0000 // 0x0031
#define REG_BANK_ALBANY1   0x0001 // 0x0032
#define REG_BANK_ALBANY2   0x0002 // 0x0033
#endif

#define SOFTWARE_DESCRIPTOR_ENABLE 0x0001
#define CHECKSUM_ENABLE            0x0FE
#define RX_CHECKSUM_ENABLE         0x000E
#define CONFIG_EMAC_MOA            1 // System Type
// #define EMAC_SPEED_10                       10
// #define EMAC_SPEED_100                      100

#if 0
#define EMAC_ALLFF         0xFFFFFFFF
#define EMAC_ABSO_MEM_BASE 0xA0000000 // EMAC_ABSO_MEM_BASE                  0xA0000000
#endif
// #define INTERNEL_PHY_REG_BASE               (EMAC_RIU_REG_BASE+REG_BANK_ALBANY0*0x200)//0xFD243000
// #define EMAC_RIU_REG_BASE                   0xFD000000

#if 0
#define EMAC_ABSO_PHY_BASE   0x80000000 // EMAC_ABSO_MEM_BASE
#define EMAC_ABSO_MEM_SIZE   0x30000 // 0x16000//0x180000//0x16000//(48 * 1024)     // More than: (32 + 16(0x3FFF)) KB
#define EMAC_MEM_SIZE_SQU    4       // More than: (32 + 16(0x3FFF)) KB
#define EMAC_BUFFER_MEM_SIZE 0x0004000

// Base address here:
#endif

#define EMAC_RIU_REG_BASE (MS_IO_OFFSET + 0x1f000000)
#define EMAC_MAX_TX_QUEUE 1000
#define MIU0_BUS_BASE     0x20000000

// #define REG_EMAC0_ADDR_BASE                 (EMAC_RIU_REG_BASE+REG_BANK_EMAC0*0x200)//0xFD204000      // The register
// address base. Depends on system define.
#if 0
#define RBQP_LENG         0x0100    // 0x40//                // ==?descriptors
#define MAX_RX_DESCR      RBQP_LENG // 32   /* max number of receive buffers */
#define SOFTWARE_DESC_LEN 0x600
#endif

/*
#ifdef SOFTWARE_DESCRIPTOR
#define RX_BUFFER_SEL                       0x0001          // 0x0=2KB,0x1=4KB,0x2=8KB,0x3=16KB, 0x09=1MB
#define RX_BUFFER_SIZE                      (RBQP_LENG*SOFTWARE_DESC_LEN)  //0x10000//0x20000//
#else
#define RX_BUFFER_SEL                       0x0003          // 0x0=2KB,0x1=4KB,0x2=8KB,0x3=16KB, 0x09=1MB
#define RX_BUFFER_SIZE                      (0x2000<<RX_BUFFER_SEL)//0x10000//0x20000//
#endif
*/

/*
#define RBQP_HW_BYTES    					0x0008
#define RBQP_SIZE                           (RBQP_HW_BYTES*RBQP_LENG)      // ==8bytes*?==?descriptors
*/

// #define MAX_INT_COUNTER                     100
/*
#ifdef CONFIG_ETHERNET_ALBANY
    #define JULIAN_100_VAL                      (0x0000F011)
#else
    //#define JULIAN_100_VAL                      (0x0000F017) //for old board
    #define JULIAN_100_VAL                      (0x0000F015) //for GPHY
#endif
*/

// #define JULIAN_104_VAL                      (0x01010080UL)

#define TX_CHECKSUM_ENABLE (0x00000470)
// PHY Register
#define PHY_REG_BASIC  (0)
#define PHY_REG_STATUS (1)
// PHY setting
#define PHY_REGADDR_OFFSET (18)
#define PHY_ADDR_OFFSET    (23)

//-------------------------------------------------------------------------------------------------
//  Bit Define
//-------------------------------------------------------------------------------------------------
#define EMAC_MIU_RW (0x3 << 10) // EMAC power on clk

// -------- EMAC_CTL : (EMAC Offset: 0x0)  --------
#define EMAC_LB  (0x1 << 0) // (EMAC) Loopback. Optional. When set, loopback signal is at high level.
#define EMAC_LBL (0x1 << 1) // (EMAC) Loopback local.
#define EMAC_RE  (0x1 << 2) // (EMAC) Receive enable.
#define EMAC_TE  (0x1 << 3) // (EMAC) Transmit enable.
#define EMAC_MPE (0x1 << 4) // (EMAC) Management port enable.
#define EMAC_CSR (0x1 << 5) // (EMAC) Clear statistics registers.
#define EMAC_ISR (0x1 << 6) // (EMAC) Increment statistics registers.
#define EMAC_WES (0x1 << 7) // (EMAC) Write enable for statistics registers.
#define EMAC_BP  (0x1 << 8) // (EMAC) Back pressure.
// -------- EMAC_CFG : (EMAC Offset: 0x4) Network Configuration Register --------
#define EMAC_SPD         (0x1 << 0)    // (EMAC) Speed.
#define EMAC_FD          (0x1 << 1)    // (EMAC) Full duplex.
#define EMAC_BR          (0x1 << 2)    // (EMAC) Bit rate.
#define EMAC_CAF         (0x1 << 4)    // (EMAC) Copy all frames.
#define EMAC_NBC         (0x1 << 5)    // (EMAC) No broadcast.
#define EMAC_MTI         (0x1 << 6)    // (EMAC) Multicast hash enable
#define EMAC_UNI         (0x1 << 7)    // (EMAC) Unicast hash enable.
#define EMAC_RLF         (0x1 << 8)    // (EMAC) Receive Long Frame.
#define EMAC_EAE         (0x1 << 9)    // (EMAC) External address match enable.
#define EMAC_CLK         (0x3 << 10)   // (EMAC)
#define EMAC_CLK_HCLK_8  (0x0 << 10)   // (EMAC) HCLK divided by 8
#define EMAC_CLK_HCLK_16 (0x1 << 10)   // (EMAC) HCLK divided by 16
#define EMAC_CLK_HCLK_32 (0x2 << 10)   // (EMAC) HCLK divided by 32
#define EMAC_CLK_HCLK_64 (0x3 << 10)   // (EMAC) HCLK divided by 64
#define EMAC_RT          (0x1 << 12)   // (EMAC) Retry test
#define EMAC_CAMMEG      (0x1 << 13)   // (EMAC)
#define EMAC_VLAD_ID_EN  (0x1 << 19)   // (EMAC) VLAD_ID_EN
#define EMAC_VLAN_ID     (0xFFF << 20) // (EMAC) VLAD ID
#define EMAC_VLAD_ID_OF  (20)          // (EMAC) VLAD ID OFFSET

// -------- EMAC_SR : (EMAC Offset: 0x8) Network Status Register --------
#define EMAC_MDIO (0x1 << 1) // (EMAC)
#define EMAC_IDLE (0x1 << 2) // (EMAC)
// -------- EMAC_TCR : (EMAC Offset: 0x10) Transmit Control Register --------
#define EMAC_LEN  (0x7FF << 0) // (EMAC)
#define EMAC_NCRC (0x1 << 15)  // (EMAC)
// -------- EMAC_TSR : (EMAC Offset: 0x14) Transmit Control Register --------
#define EMAC_OVR (0x1 << 0) // (EMAC)
#define EMAC_COL (0x1 << 1) // (EMAC)
#define EMAC_RLE (0x1 << 2) // (EMAC)
//#define EMAC_TXIDLE     ( 0x1 <<  3) // (EMAC)
#define EMAC_IDLETSR   (0x1 << 3)  // (EMAC)
#define EMAC_BNQ       (0x1 << 4)  // (EMAC)
#define EMAC_COMP      (0x1 << 5)  // (EMAC)
#define EMAC_UND       (0x1 << 6)  // (EMAC)
#define EMAC_TBNQ      (0x1 << 7)  // (EMAC)
#define EMAC_FBNQ      (0x1 << 8)  // (EMAC)
#define EMAC_FIFO1IDLE (0x1 << 9)  // (EMAC)
#define EMAC_FIFO2IDLE (0x1 << 10) // (EMAC)
#define EMAC_FIFO3IDLE (0x1 << 11) // (EMAC)
#define EMAC_FIFO4IDLE (0x1 << 12) // (EMAC)
// -------- EMAC_RSR : (EMAC Offset: 0x20) Receive Status Register --------
#define EMAC_DNA    (0x1 << 0) // (EMAC)
#define EMAC_REC    (0x1 << 1) // (EMAC)
#define EMAC_RSROVR (0x1 << 2) // (EMAC)
#define EMAC_BNA    (0x1 << 3) // (EMAC)
// -------- EMAC_ISR : (EMAC Offset: 0x24) Interrupt Status Register --------
#define EMAC_INT_DONE  (0x1 << 0)  // (EMAC)
#define EMAC_INT_RCOM  (0x1 << 1)  // (EMAC)
#define EMAC_INT_RBNA  (0x1 << 2)  // (EMAC)
#define EMAC_INT_TOVR  (0x1 << 3)  // (EMAC)
#define EMAC_INT_TUND  (0x1 << 4)  // (EMAC)
#define EMAC_INT_RTRY  (0x1 << 5)  // (EMAC)
#define EMAC_INT_TBRE  (0x1 << 6)  // (EMAC)
#define EMAC_INT_TCOM  (0x1 << 7)  // (EMAC)
#define EMAC_INT_TIDLE (0x1 << 8)  // (EMAC)
#define EMAC_INT_LINK  (0x1 << 9)  // (EMAC)
#define EMAC_INT_ROVR  (0x1 << 10) // (EMAC)
#define EMAC_INT_HRESP (0x1 << 11) // (EMAC)
#if TX_DELAY_INT
#define EMAC_INT_TX_DELAY         (0x1 << 13) // (EMAC)
#define EMAC_INT_TX_DELAY_TIMEOUT (0x1 << 14) // (EMAC)
#endif
#define EMAC_INT_TXQUEUE_THRESHOLD (0x1 << 24) // (EMAC)
#define EMAC_INT_TXQUEUE_EMPTY     (0x1 << 25) // (EMAC)
#define EMAC_INT_TXQUEUE_DROP      (0x1 << 26) // (EMAC)
#define EMAC_INT_RCOM_DELAY        (0x1 << 31) // a virtual bit for JULIAN_D RX delay interrupt
// -------- EMAC_IER : (EMAC Offset: 0x28) Interrupt Enable Register --------
// -------- EMAC_IDR : (EMAC Offset: 0x2c) Interrupt Disable Register --------
// -------- EMAC_IMR : (EMAC Offset: 0x30) Interrupt Mask Register --------
// -------- EMAC_MAN : (EMAC Offset: 0x34) PHY Maintenance Register --------
#define EMAC_DATA       (0xFFFF << 0) // (EMAC)
#define EMAC_CODE       (0x3 << 16)   // (EMAC)
#define EMAC_CODE_802_3 (0x2 << 16)   // (EMAC) Write Operation
#define EMAC_REGA       (0x1F << 18)  // (EMAC)
#define EMAC_PHYA       (0x1F << 23)  // (EMAC)
#define EMAC_RW         (0x3 << 28)   // (EMAC)
#define EMAC_RW_R       (0x2 << 28)   // (EMAC) Read Operation
#define EMAC_RW_W       (0x1 << 28)   // (EMAC) Write Operation
#define EMAC_HIGH       (0x1 << 30)   // (EMAC)
#define EMAC_LOW        (0x1 << 31)   // (EMAC)
// -------- EMAC_RBRP: (EMAC Offset: 0x38) Receive Buffer First full pointer--------
#define EMAC_WRAP_R (0x1 << 14) // Wrap bit
// -------- EMAC_RBWP: (EMAC Offset: 0x3C) Receive Buffer Current pointer--------
#define EMAC_WRAP_W (0x1 << 14) // Wrap bit
// ........................................................................ //
// URANUS PHY //
// #define MII_URANUS_ID                   0x01111//Test value
// URANUS specific registers //
#define MII_USCR_REG   16
#define MII_USCSR_REG  17
#define MII_USINTR_REG 21
/* ........................................................................ */
#define EMAC_DESC_DONE 0x00000001 /* bit for if DMA is done */
#define EMAC_DESC_WRAP 0x00000002 /* bit for wrap */
#define EMAC_BROADCAST 0x80000000 /* broadcast address */
#define EMAC_MULTICAST 0x40000000 /* multicast address */
#define EMAC_UNICAST   0x20000000 /* unicast address */

#define EMAC_DESC_IPV4         (0x1 << 11)
#define EMAC_DESC_UDP          (0x1 << 18)
#define EMAC_DESC_TCP          (0x1 << 19)
#define EMAC_DESC_IP_CSUM      (0x1 << 20)
#define EMAC_DESC_TCP_UDP_CSUM (0x1 << 21)
// Constant: ----------------------------------------------------------------
// Register MAP EMAC0:
#define REG_ETH_CTL           0x00000000 // Network control register
#define REG_ETH_CFG           0x00000004 // Network configuration register
#define REG_ETH_SR            0x00000008 // Network status register
#define REG_ETH_TAR           0x0000000C // Transmit address register
#define REG_ETH_TCR           0x00000010 // Transmit control register
#define REG_ETH_TSR           0x00000014 // Transmit status register
#define REG_ETH_RBQP          0x00000018 // Receive buffer queue pointer
#define REG_ETH_BUFF          0x0000001C // Receive Buffer Configuration
#define REG_ETH_RSR           0x00000020 // Receive status register
#define REG_ETH_ISR           0x00000024 // Interrupt status register
#define REG_ETH_IER           0x00000028 // Interrupt enable register
#define REG_ETH_IDR           0x0000002C // Interrupt disable register
#define REG_ETH_IMR           0x00000030 // Interrupt mask register
#define REG_ETH_MAN           0x00000034 // PHY maintenance register
#define REG_ETH_BUFFRDPTR     0x00000038 // Receive First Full Pointer
#define REG_ETH_BUFFWRPTR     0x0000003C // Receive Current pointer
#define REG_ETH_FRA           0x00000040 // Frames transmitted OK
#define REG_ETH_SCOL          0x00000044 // Single collision frames
#define REG_ETH_MCOL          0x00000048 // Multiple collision frames
#define REG_ETH_OK            0x0000004C // Frames received OK
#define REG_ETH_SEQE          0x00000050 // Frame check sequence errors
#define REG_ETH_ALE           0x00000054 // Alignment errors
#define REG_ETH_DTE           0x00000058 // Deferred transmission frames
#define REG_ETH_LCOL          0x0000005C // Late collisions
#define REG_ETH_ECOL          0x00000060 // Excessive collisions
#define REG_ETH_TUE           0x00000064 // Transmit under-run errors
#define REG_ETH_CSE           0x00000068 // Carrier sense errors
#define REG_ETH_RE            0x0000006C // Receive resource error
#define REG_ETH_ROVR          0x00000070 // Received overrun
#define REG_ETH_SE            0x00000074 // Received symbols error
#define REG_ETH_ELR           0x00000078 // Excessive length errors
#define REG_ETH_RJB           0x0000007C // Receive jabbers
#define REG_ETH_USF           0x00000080 // Undersize frames
#define REG_ETH_SQEE          0x00000084 // SQE test errors
#define REG_ETH_HSL           0x00000090 // Hash register [31:0]
#define REG_ETH_HSH           0x00000094 // Hash register [63:32]
#define REG_ETH_SA1L          0x00000098 // Specific address 1 first 4 bytes
#define REG_ETH_SA1H          0x0000009C // Specific address 1 last  2 bytes
#define REG_ETH_SA2L          0x000000A0 // Specific address 2 first 4 bytes
#define REG_ETH_SA2H          0x000000A4 // Specific address 2 last  2 bytes
#define REG_ETH_SA3L          0x000000A8 // Specific address 3 first 4 bytes
#define REG_ETH_SA3H          0x000000AC // Specific address 3 last  2 bytes
#define REG_ETH_SA4L          0x000000B0 // Specific address 4 first 4 bytes
#define REG_ETH_SA4H          0x000000B4 // Specific address 4 last  2 bytes
#define REG_TAG_TYPE          0x000000B8 // tag type of the frame
#define REG_TXQUEUE_INT_LEVEL 0x000000D0 // thresholad for TX queue
#define REG_TXQUEUE_CNT       0x000000D8 // thresholad for TX queue
#if TX_DELAY_INT
#define REG_TXDELAY_MAX_CNT    0x000000E0 // tx delay packet max counter
#define REG_TXDELAY_CNT        0x000000E2 // tx delay packet count
#define REG_TXDELAY_TO_MAX_CNT 0x000000E4 // tx delay timeout max counter
#define REG_TXDELAY_TO_CNT     0x000000E3 // tx delay timeout count
#endif

#define REG_EMAC_JULIAN_0100 0x00000100
#define REG_EMAC_JULIAN_0104 0x00000104
#define REG_EMAC_JULIAN_0108 0x00000108
#define REG_EMAC_JULIAN_011C 0x0000011C
#define REG_EMAC_JULIAN_0120 0x00000120
#define REG_EMAC_JULIAN_0124 0x00000124
#define REG_EMAC_JULIAN_0134 0x00000134
#define REG_EMAC_JULIAN_0138 0x00000138
#define REG_EMAC_JULIAN_0146 0x00000146 // REG_ETH_EMAC1_h23

// TX FIX WRONG PACKET
#define RED_EMAC_JULIAN_01D2 0x000001D2

// register of NetWork Stom Protect
#define REG_EMAA_JULIAN_01A0 0x000001A0 // REG_ETH_EMAC1_h50  //unicast tc max
#define REG_EMAA_JULIAN_01A4 0x000001A4 // REG_ETH_EMAC1_h52  //unicast tc consume
#define REG_EMAA_JULIAN_01B0 0x000001B0 // REG_ETH_EMAC1_h58  //multicast tc max
#define REG_EMAA_JULIAN_01B4 0x000001B4 // REG_ETH_EMAC1_h5A  //multicast tc consume
#define REG_EMAA_JULIAN_01C0 0x000001C0 // REG_ETH_EMAC1_h60  // broadcast tc max
#define REG_EMAA_JULIAN_01C4 0x000001C4 // REG_ETH_EMAC1_h62  // broadcast tc consume

#define PIPE_LINE_DELAY (0x0001 << 4)
#define EMAC_PATCH_LOCK (0x0001 << 15)

//#define REG_CAMA0_l                     0x00000200         // 16 LSB of CAM address  0
//#define REG_CAMA0_h                     0x00000204         // 32 MSB of CAM address  0
//#define REG_CAMA62_l                    0x000003F0         // 16 LSB of CAM address 62
//#define REG_CAMA62_h                    0x000003F4         // 32 MSB of CAM address 62

// Register MAP EMAC1:
#define REG_ETH_EMAC1_h23 0x00000046
#define TXQUEUE_CNT_LATCH (0x0001 << 3)
#define REG_ETH_EMAC1_h24 0x00000048 // new_tx_queue setting
#define REG_ETH_EMAC1_h2F 0x0000005E // new_tx_queue size and interrupt status
#define REG_ETH_EMAC1_h4D 0x0000009A // reg_pause

// register of NetWork Stom Protect
#if NETWORK_STORM_PROTECT || NETWORK_STORM_PROTECT_DEBUG
#define REG_ETH_EMAC1_h50                       0x000000A0 // unicast tc max
#define REG_ETH_EMAC1_h51                       0x000000A2 // unicast tc max
#define REG_ETH_EMAC1_h52                       0x000000A4 // unicast tc consume
#define REG_ETH_EMAC1_h53                       0x000000A6 // unicast tc consume
#define REG_ETH_EMAC1_h54                       0x000000A8 // unicast tc token
#define REG_ETH_EMAC1_h55                       0x000000AA // unicast tc token
#define REG_ETH_EMAC1_h56                       0x000000AC // unicast tc drop
#define REG_ETH_EMAC1_h57                       0x000000AE // unicast tc drop
#define REG_ETH_EMAC1_h58                       0x000000B0 // multicast tc max
#define REG_ETH_EMAC1_h59                       0x000000B2 // multicast tc max
#define REG_ETH_EMAC1_h5A                       0x000000B4 // multicast tc consume
#define REG_ETH_EMAC1_h5B                       0x000000B6 // multicast tc consume
#define REG_ETH_EMAC1_h5C                       0x000000B8 // multicast tc token
#define REG_ETH_EMAC1_h5D                       0x000000BA // multicast tc token
#define REG_ETH_EMAC1_h5E                       0x000000BC // multicast tc drop
#define REG_ETH_EMAC1_h5F                       0x000000BE // multicast tc drop
#define REG_ETH_EMAC1_h60                       0x000000C0 // broadcast tc max
#define REG_ETH_EMAC1_h61                       0x000000C2 // broadcast tc max
#define REG_ETH_EMAC1_h62                       0x000000C4 // broadcast tc consume
#define REG_ETH_EMAC1_h63                       0x000000C6 // broadcast tc consume
#define REG_ETH_EMAC1_h64                       0x000000C8 // broadcast tc token
#define REG_ETH_EMAC1_h65                       0x000000CA // broadcast tc token
#define REG_ETH_EMAC1_h66                       0x000000CC // broadcast tc drop
#define REG_ETH_EMAC1_h67                       0x000000CE // broadcast tc drop
#define REG_ETH_EMAC1_h68                       0x000000D0 // nstsp status
#define EMAC_RX_NETSP_UNICAST_EN                (0x1 << 0) //
#define EMAC_RX_NETSP_MULTICAST_EN              (0x1 << 1) //
#define EMAC_RX_NETSP_BROADCAST_EN              (0x1 << 2) //
#define EMAC_FIX_MIU_WRITE_OUT_OF_RANGE_CORRECT (0x1 << 14)
#define EMAC_RX_802p1q_CHK_DDISABLE             (0x1 << 15)
#endif

// register of flow control
#if HW_FLOW_CONTROL
#define REG_ETH_EMAC1_h6A          0x000000D4 // Pause Patter High 16bits
#define REG_ETH_EMAC1_h6B          0x000000D6 // Pause Patter Low 16bits
#define REG_ETH_EMAC1_h6C          0x000000D8 // Pause Length
#define REG_ETH_EMAC1_h6D          0x000000DA // Des Addr Hght 16bits
#define REG_ETH_EMAC1_h6E          0x000000DC // Des Addr Mid 16bits
#define REG_ETH_EMAC1_h6F          0x000000DE // Des Addr low 16bits
#define REG_ETH_EMAC1_h7A          0x000000F4 // Multicast addr high 16bits
#define REG_ETH_EMAC1_h7B          0x000000F6 // Multicast addr mid 16bits
#define REG_ETH_EMAC1_h7C          0x000000F8 // Multicast addr low 16bits
#define REG_ETH_EMAC1_h7D          0x000000fA // Pause status
#define REG_PAUSE_RX_EN            (0x1 << 0)
#define REG_PAUSE_TX_TRIG          (0x1 << 1)
#define REG_PAUSE_TX_AUTORIG_EN    (0x1 << 2)
#define REG_PAUSE_TX_WAIT          (0x1 << 3)
#define REG_PAUSE_AUTORIG_TH_POS   4
#define REG_PAUSE_AUTORIG_TH       (0x3ff << REG_PAUSE_AUTORIG_TH_POS)
#define REG_PAUSE_RXDONE_TRIG      (0x1 << 15)
#define REG_ETH_EMAC1_h7E          0x000000FC // RX QUEUE CNT FOR PAUSE
#define REG_RX_QUEUE_CNT_FOR_PAUSE (0x3ff << 0)
#endif

/// TX desc
#define REG_TXD_BASE  0x0000015E // emac1 : 32 bits : 0x2F, 0x30
#define REG_TXD_PTR_L 0x00000162

#define REG_TXD_STAT  0x00000164 // emac1 : 16 bits : 0x32
#define TXD_NUM_MASK  0x3FFFF
#define TXD_OVR       (0x1 << 15)
#define REG_TXD_CFG   0x00000166 // emac1 : 16 bits : 0x33
#define TXD_NUM_MASK  0x3FFFF
#define TXD_ENABLE    (0x1 << 15)
#define REG_TXD_XMIT0 0x00000180
#define REG_TXD_XMIT1 0x00000182

#if 0
#define REG_TXD_PTR_L 0x00000162
#define REG_TXD_PTR_H 0x0000016c

#define REG_TXD_BASE_H 0x00000168 // emac1 : 32 bits : 0x2F, 0x30

#define REG_TXD_XMIT2 0x00000184
#define REG_TXD_XMIT3 0x00000186
#endif

#if 0
#define REG_RW32_LOW_PRI_TX_DESC_BASE 0x0000015E // nba : emac1 : 0x2F, 0x30 : 32 bits [ tx_desc_base ]
// #define REG_RO16_LOW_PRI_TX_DESC_PTR        0x00000162          // nba : what is this for?
#define REG_RO16_LOW_PRI_TX_DESC_QUEUED \
    0x00000164 // nba : emac1 : 0x32
               //                  bit 15 : [ tx_desc_number/overrun status ] ?? incude "4"? probably 4 is excluded
               //                  bit [13, 0] : # of TX packet used
#define REG_RW16_LOW_PRI_TX_DESC_THRESHOLD \
    0x00000166 // nba : emac1 : 0x33
               //                  bit 15 : tx desc enable
               //                  bit [13, 0] : number of TX descriptor
#endif

/* ........................................................................ */
u8  Hal_EMAC_ReadReg8(unsigned long base, u32 bank, u32 reg);
u32 Hal_EMAC_ReadReg32(void*, u32 xoffset);
// u32 Hal_EMAC_NegotiationPHY(void*, unsigned char phyaddr);
void Hal_EMAC_ConfigPHY(void*, int);
int  Hal_EMAC_GetPHY(void*);
u32  Hal_EMAC_get_SA1H_addr(void*);
u32  Hal_EMAC_get_SA1L_addr(void*);
u32  Hal_EMAC_get_SA2H_addr(void*);
u32  Hal_EMAC_get_SA2L_addr(void*);
u32  Hal_EMAC_Read_CTL(void*);
u32  Hal_EMAC_Read_CFG(void*);
u32  Hal_EMAC_Read_RBQP(void*);

// u32 Hal_EMAC_Read_RDPTR(void);
// u32 Hal_EMAC_Read_BUFF(void);
u32 Hal_EMAC_Read_FRA(void*);
u32 Hal_EMAC_Read_SCOL(void*);
u32 Hal_EMAC_Read_MCOL(void*);
u32 Hal_EMAC_Read_OK(void*);
u32 Hal_EMAC_Read_SEQE(void*);
u32 Hal_EMAC_Read_ALE(void*);
u32 Hal_EMAC_Read_LCOL(void*);
u32 Hal_EMAC_Read_ECOL(void*);
u32 Hal_EMAC_Read_TUE(void*);
// u32 Hal_EMAC_Read_RSR(void);
u32 Hal_EMAC_Read_CSE(void*);
// u32 Hal_EMAC_Read_RE(void);
u32 Hal_EMAC_Read_ROVR(void*);
u32 Hal_EMAC_Read_SE(void*);
u32 Hal_EMAC_Read_ELR(void*);
u32 Hal_EMAC_Read_RJB(void*);
// u32 Hal_EMAC_Read_USF(void);
u32 Hal_EMAC_Read_SQEE(void*);
// u32 Hal_EMAC_Read_JULIAN_0100(void*);
void Hal_EMAC_WritReg32(void*, u32 xoffset, u32 xval);
void Hal_EMAC_update_HSH(void*, u32 mc0, u32 mc1);
void Hal_EMAC_Write_CTL(void*, u32 xval);
void Hal_EMAC_Write_CFG(void*, u32 xval);
void Hal_EMAC_Write_RBQP(void*, u32 u32des);
void Hal_EMAC_Write_BUFF(void*, u32 xval);
void Hal_EMAC_Write_MAN(void*, u32 xval);
void Hal_EMAC_Write_RSR(void*, u32 xval);
// void Hal_EMAC_Write_RDPTR(u32 xval);
// void Hal_EMAC_Write_WRPTR(u32 xval);
void Hal_EMAC_Write_SA1H(void*, u32 xval);
void Hal_EMAC_Write_SA1L(void*, u32 xval);
void Hal_EMAC_Write_SA2H(void*, u32 xval);
void Hal_EMAC_Write_SA2L(void*, u32 xval);
// void Hal_EMAC_Write_JULIAN_0100(void*, u32 xval);
void Hal_EMAC_Write_JULIAN_0100(void*, int bMIU_reset); // , int rmii); // , int phy_internal);
void Hal_EMAC_Set_Tx_JULIAN_T(void*, u32 xval);
// u32 Hal_EMAC_Get_Tx_FIFO_Threshold(void);
void Hal_EMAC_Set_Rx_FIFO_Enlarge(void*, u32 xval);
u32  Hal_EMAC_Get_Rx_FIFO_Enlarge(void*);
void Hal_EMAC_Set_Miu_Priority(void*, u32 xval);
// u32 Hal_EMAC_Get_Miu_Priority(void);
void Hal_EMAC_Set_Tx_Hang_Fix_ECO(void*, u32 xval);
void Hal_EMAC_Set_MIU_Out_Of_Range_Fix(void*, u32 xval);
void Hal_EMAC_Set_Rx_Tx_Burst16_Mode(void*, u32 xval);
// void Hal_EMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval);
void Hal_EMAC_Set_Rx_Byte_Align_Offset(void*, u32 xval);
// void Hal_EMAC_Write_Protect(u32 start_addr, u32 length);
void Hal_EMAC_Set_Pipe_Line_Delay(void*, int enable);

void* Hal_EMAC_Alloc(unsigned long riu, unsigned long x32, unsigned long riu_phy);
void  Hal_EMAC_Free(void*);

void Hal_EMAC_HW_init(void*);
void Hal_EMAC_Power_On_Clk(void*, struct device*);
void Hal_EMAC_Power_Off_Clk(void*, struct device*);
void Hal_EMAC_enable_mdi(void*);
void Hal_EMAC_disable_mdi(void*);
int  Hal_EMAC_write_phy(void*, unsigned char phy_addr, u32 address, u32 value);
int  Hal_EMAC_read_phy(void*, unsigned char phy_addr, u32 address, u32* value);
void Hal_EMAC_enable_phyirq(void*);
void Hal_EMAC_disable_phyirq(void*);
void Hal_EMAC_update_speed_duplex(void*, int speed, int duplex);
void Hal_EMAC_Write_SA1_MAC_Address(void*, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5);
void Hal_EMAC_Write_SA2_MAC_Address(void*, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5);
void Hal_EMAC_Write_SA3_MAC_Address(void*, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5);
void Hal_EMAC_Write_SA4_MAC_Address(void*, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5);
void Hal_EMAC_Set_Reverse_LED(void*, u32 xval);
u8   Hal_EMAC_Get_Reverse_LED(void*);
void Hal_EMAC_Set_Invert_LED(void* hal);

int Hal_EMAC_TXQ_Mode(void*);
int Hal_EMAC_TXQ_Size(void*);
int Hal_EMAC_TXQ_Free(void*);
int Hal_EMAC_TXQ_Used(void*);
int Hal_EMAC_TXQ_Empty(void*);
int Hal_EMAC_TXQ_Full(void*);
int Hal_EMAC_TXQ_Insert(void*, u32 phy, u32 len);
// int Hal_EMAC_TXQ_Remove(void);
// int Hal_EMAC_TXQ_Done(void*);

int Hal_EMAC_TXD_Cfg(void*, u32 TXD_num);
int Hal_EMAC_TXD_Buf(void*, void* p, dma_addr_t bus, u32 len);

u32  Hal_EMAC_IntStatus(void*);
void Hal_EMAC_IntEnable(void*, u32 intMsk, int bEnable);
u32  Hal_EMAC_RX_ParamSet(void*, u32 frm_no, u32 frm_cyc);
u32  Hal_EMAC_RX_ParamRestore(void*);

void Hal_EMAC_Pad(void*, unsigned long reg, unsigned long msk, unsigned long val);
void Hal_EMAC_PadLed(void*, unsigned long reg, unsigned long msk, unsigned long val);
void Hal_EMAC_PhyMode(void*, u32 phy_mode);

int Hal_EMAC_FlowControl_TX(void*);

void Hal_EMAC_MIU_Protect_RX(void* hal, u32 start, u32 end);

#if TX_DELAY_INT
u32 Hal_EMAC_PACKET_CNT(void*);
#endif

// for NetWork Storm Protect
#if NETWORK_STORM_PROTECT || NETWORK_STORM_PROTECT_DEBUG
void Hal_EMAC_Netsp_Unicast_Setting(void* hal, u32 max, u32 consume, u32 en);
void Hal_EMAC_Netsp_Multicast_Setting(void* hal, u32 max, u32 consume, u32 en);
void Hal_EMAC_Netsp_Broadcast_Setting(void* hal, u32 max, u32 consume, u32 en);
#endif

// for flow contol
#if HW_FLOW_CONTROL
void Hal_EMAC_Flow_Control_Mac_Address(void* hal, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5);
void Hal_EMAC_Flow_Control_Init(void* hal, u32 isWait, u16 pauseLength);
void Hal_EMAC_TX_Flow_control_sw_triger(void* hal, u32 en);
void Hal_EMAC_TX_Flow_control_hw_triger(void* hal, u32 en);
void Hal_EMAC_Flow_control_Rx_Done_Trig(void* hal);
void Hal_EMAC_Flow_control_Auto_Trig_th(void* hal, u16 th);
void Hal_EMAC_RX_Flow_control_hw_en(void* hal, u32 en);
u16  Hal_EMAC_Read_Rx_Queue_Cnt(void* hal);
void Hal_EMAC_TX_Flow_Ctrl_Enable(void* hal, int enable);
void Hal_EMAC_RX_Flow_Ctrl_Enable(void* hal, int enable);
#endif

#if VLAD_TAG_TEST_DEBUG
void Hal_EMAC_Vlan_Id_Init(void* hal, u16 en, u16 vlan_id);
void Hal_EMAC_Update_Vlan_Id(void* hal, u16 vlan_id);
#endif

#if PHY_FORCE_NEG_SET
// internal phy set reg
void Hal_EMAC_PHY_FORCE_MODE_REG_SET(void* hal, const struct ethtool_link_ksettings* cmd);
#endif

#if PHY_MDI_CTL
u32 Hal_EMAC_Phy_MDI_MDIX(void* hal, int bSwitch);
#endif

// for fpga
void Hal_EMAC_mdio_path(void* hal, int mdio_path);

void Hal_EMAC_Phy_Restart_An(void* hal);

#endif
// -----------------------------------------------------------------------------
// Linux EMAC.c End
// -----------------------------------------------------------------------------
