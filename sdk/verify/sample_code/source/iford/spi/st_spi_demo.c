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
#include <elf.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <getopt.h>
#include "ss_spi.h"

static char *sSrcFileName = "/customer/slot_mark.sh";
int          s32TesrCnt   = 5;

static void print_usage(const char *sProg)
{
    printf("Usage: %s [-F]\n", sProg);
    printf("  -F --src file  specify a src file to send and read\n");
    printf("  -c --cnt   specify loop times\n");
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1)
    {
        static const struct option lopts[] = {
            {"input", 1, 0, 'F'},
            {"cnt", 1, 0, 'c'},
            {NULL, 0, 0, 0},
        };
        int c;

        c = getopt_long(argc, argv, "F:c:", lopts, NULL);

        if (c == -1)
            break;

        switch (c)
        {
            case 'F':
                sSrcFileName = optarg;
                break;
            case 'c':
                s32TesrCnt = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
        }
    }
}

int main(int argc, char **argv)
{
    int                    s32Ret;
    char *                 sTxBuf       = NULL;
    unsigned long long     u64TxBuf_Num = 0;
    unsigned long long     u64RxNum;
    int                    s32SrcFileFd = -1;
    off_t                  FileSize;
    unsigned long long     u64Bytes;
    struct SS_SPI_Config_t stSPIConfig;

    parse_opts(argc, argv);
    struct SS_SPI_Packet_t *pstPacket = malloc(sizeof(struct SS_SPI_Packet_t));
    if (!pstPacket)
    {
        printf("can't allocate packet\n");
        return -1;
    }
    s32Ret = SS_SPI_Init();
    if (s32Ret != 0)
    {
        printf("SPI init fail.\n");
        goto error_malloc_packet;
    }

    stSPIConfig.speed_hz      = 12500000;
    stSPIConfig.clk_mode      = PSPI_MODE_3;
    stSPIConfig.lsb           = 0;
    stSPIConfig.use_3wire     = 0;
    stSPIConfig.bits_per_word = 8;
    stSPIConfig.cs_polarity   = 0;
    stSPIConfig.spidev        = "/dev/spidev1.0";
    SS_SPI_Setup(&stSPIConfig);
    s32SrcFileFd = open(sSrcFileName, O_RDONLY);
    if (s32SrcFileFd < 0)
    {
        printf("open %s  file failed", sSrcFileName);
        goto error_open;
    }

    FileSize = lseek(s32SrcFileFd, 0, SEEK_END);
    if (FileSize == -1)
    {
        printf("set file offset end fail.\n");
        goto error_lseek;
    }
    s32Ret = lseek(s32SrcFileFd, 0, SEEK_SET);
    if (s32Ret == -1)
    {
        printf("set file offset begin fail.\n");
        goto error_lseek;
    }
    sTxBuf = malloc(FileSize);
    if (!sTxBuf)
    {
        printf("can't allocate tx buffer");
        goto error_lseek;
    }

    u64Bytes = read(s32SrcFileFd, sTxBuf, FileSize);
    if (u64Bytes != FileSize)
    {
        printf("failed to read input file %s (%llu:%lu).", sSrcFileName, u64Bytes, FileSize);

        goto error_malloc;
    }
    u64TxBuf_Num = u64Bytes;

    while (s32TesrCnt--)
    {
        // tx
        pstPacket->buf     = &u64TxBuf_Num;
        pstPacket->buf_num = sizeof(u64TxBuf_Num);
        s32Ret             = SS_SPI_Write(pstPacket);
        if (s32Ret < 0)
        {
            printf("tx send-data num failed");
            goto error_malloc;
        }

        pstPacket->buf     = sTxBuf;
        pstPacket->buf_num = u64TxBuf_Num;
        s32Ret             = SS_SPI_Write(pstPacket);
        if (s32Ret < 0)
        {
            printf("tx send-data failed");
            goto error_malloc;
        }

        // rx
        pstPacket->buf     = &u64RxNum;
        pstPacket->buf_num = sizeof(u64RxNum);
        s32Ret             = SS_SPI_Read(pstPacket);
        if (s32Ret < 0)
        {
            printf("rx recv-data num failed");
            goto error_malloc;
        }
        printf("app data,num %llu\n", u64RxNum);

        pstPacket->buf = malloc(u64RxNum);
        if (!pstPacket->buf)
        {
            printf("can't allocate rx buffer");
            goto error_malloc;
        }
        pstPacket->buf_num = u64RxNum;
        s32Ret             = SS_SPI_Read(pstPacket);
        if (s32Ret < 0)
        {
            printf("rx recv-data failed");
            goto error_transfer;
        }

        if (!memcmp(pstPacket->buf, sTxBuf, pstPacket->buf_num))
        {
            free(pstPacket->buf);
            printf("data  compare success!\n");
        }
        else
        {
            printf("data  compare fail,test cnt %d!\n", s32TesrCnt);
            s32Ret = -1;
            goto error_transfer;
        }
    }

    free(sTxBuf);
    close(s32SrcFileFd);
    free(pstPacket);
    SS_SPI_DeInit();
    return 0;

error_transfer:
    free(pstPacket->buf);
error_malloc:
    free(sTxBuf);
error_lseek:
    close(s32SrcFileFd);
error_open:
    SS_SPI_DeInit();
error_malloc_packet:
    free(pstPacket);
    return s32Ret;
}
