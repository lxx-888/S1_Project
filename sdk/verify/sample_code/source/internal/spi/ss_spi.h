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
#ifndef _ST_GLASSES_PSPI_H
#define _ST_GLASSES_PSPI_H
#include <linux/spi/spidev.h>
#include <pthread.h>

#define APP_BUF_LEN 0x10000

#define POLYNOMIAL      0x04C11DB7
#define INIT_CRC        0xFFFFFFFF
#define FINAL_XOR_VALUE 0xFFFFFFFF

/* debug level */
#define DEBUG_LEVEL_NONE  0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARN  2
#define DEBUG_LEVEL_INFO  3
#define DEBUG_LEVEL_DEBUG 4

/* default debug level  */
#ifndef DEBUG
#define DEBUG DEBUG_LEVEL_ERROR
#endif

/* debug  */
#define LOG_ERROR(fmt, ...)                                                                                \
    do                                                                                                     \
    {                                                                                                      \
        if (DEBUG >= DEBUG_LEVEL_ERROR)                                                                    \
            fprintf(stderr, "\033[31m[ERROR] %s:%d: " fmt "\033[0m\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define LOG_WARN(fmt, ...)                                                      \
    do                                                                          \
    {                                                                           \
        if (DEBUG >= DEBUG_LEVEL_WARN)                                          \
            fprintf(stderr, "\033[33m[WARN]  " fmt "\033[0m\n", ##__VA_ARGS__); \
    } while (0)

#define LOG_INFO(fmt, ...)                                                                      \
    do                                                                                          \
    {                                                                                           \
        if (DEBUG >= DEBUG_LEVEL_INFO)                                                          \
            fprintf(stdout, "\033[32m[INFO] %s:" fmt "\033[0m\n", __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_DEBUG(fmt, ...)                                                                 \
    do                                                                                      \
    {                                                                                       \
        if (DEBUG >= DEBUG_LEVEL_DEBUG)                                                     \
            fprintf(stdout, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

struct SS_SPI_Config_t
{
    unsigned int  clk_mode;
    unsigned int  lsb;
    unsigned int  use_3wire;
    unsigned int  cs_polarity; /**0 :  active low or  1: active high */
    unsigned char bits_per_word;
    unsigned int  speed_hz;
    char *        spidev;
};

struct SS_SPI_Packet_t
{
    void *             buf;
    unsigned long long buf_num;
};

enum
{
    PSPI_MODE_0 = SPI_MODE_0, /* Equal to SPI_CPOL_0_CPHA_0 */
    PSPI_MODE_1,              /* Equal to SPI_CPOL_0_CPHA_1 */
    PSPI_MODE_2,              /* Equal to SPI_CPOL_1_CPHA_0 */
    PSPI_MODE_3,              /* Equal to SPI_CPOL_1_CPHA_1 */
};

int SS_SPI_Init(void);
int SS_SPI_DeInit(void);
int SS_SPI_Read(struct SS_SPI_Packet_t *pstPacket);
int SS_SPI_Write(struct SS_SPI_Packet_t *pstPacket);
int SS_SPI_Setup(struct SS_SPI_Config_t *cfg);

#endif
