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
#include <pthread.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "ss_spi.h"

static int              u32SPIDevFd = -1;
static struct spi_data *g_PData;

#define PSPI_CTRL_MSG_SIZE sizeof(struct ctrl_mesg)
#define CHUNK_SIZE         4096

struct ctrl_mesg
{
    unsigned char  spi_state;
    unsigned char  cmd;
    unsigned short next_rbuf_len;
};

struct spi_data
{
    char *           ctrlbuf;
    struct ctrl_mesg cur_ctrlmesg;
    pthread_mutex_t  cur_ctrlmesg_lock;
    void *           slvxmit_buf;
    int              slvxmit_len;
    void *           slvrecv_buf;
    int              slvrecv_len;
};

enum
{
    SPI_STATE_IDLE = 0,
    SPI_STATE_READ,
    SPI_STATE_WRITE,

} SS_SPI_STATE;

enum
{
    SPI_CMD_READ  = 0X3A,
    SPI_CMD_WRITE = 0X4B,
    SPI_CMD_RESET = 0X6d,
} SS_CMD;

enum
{
    ACK   = 0xa3,
    NACK  = 0Xa4,
    RETRY = 0xa6,
};

static int _SS_SPI_Read(struct spi_data *pdata, void *rxbuf, size_t len)
{
    int                     s32Ret;
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)NULL,
        .rx_buf        = (unsigned long)rxbuf,
        .len           = len,
        .bits_per_word = 8,
    };

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_MESSAGE(1), &tr);
    if (s32Ret < 1)
    {
        LOG_ERROR("read  failed(%d)\n", s32Ret);
        return -1;
    }

    return 0;
}

static int _SS_SPI_Write(struct spi_data *pdata, void *txbuf, size_t len)
{
    int                     s32Ret;
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)txbuf,
        .rx_buf        = (unsigned long)NULL,
        .len           = len,
        .bits_per_word = 8,
    };

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_MESSAGE(1), &tr);
    if (s32Ret < 1)
    {
        LOG_ERROR("write failed(%d)\n", s32Ret);
        return -1;
    }

    return 0;
}

static int _SS_SPI_SetClkMode(unsigned int u32clkMode)
{
    int          s32Ret;
    unsigned int u32SetMode;
    unsigned int u32CompareMode;
    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_MODE32, &u32SetMode);
    if (s32Ret == -1)
        return s32Ret;

    u32SetMode |= u32clkMode;
    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_WR_MODE32, &u32SetMode);
    if (s32Ret == -1)
        return s32Ret;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_MODE32, &u32CompareMode);
    if (s32Ret == -1)
        return s32Ret;

    if (u32CompareMode != u32SetMode)
        return -1;

    return 0;
}

static int _SS_SPI_SetBitPerWord(unsigned char u8bpw)
{
    int           s32Ret;
    unsigned char u8Compare;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_WR_BITS_PER_WORD, &u8bpw);
    if (s32Ret == -1)
        return s32Ret;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_BITS_PER_WORD, &u8Compare);
    if (s32Ret == -1)
        return s32Ret;

    if (u8Compare != u8bpw)
    {
        LOG_ERROR("set bpw %d,read %d\n", u8bpw, u8Compare);
        return -1;
    }
    return 0;
}

static int _SS_SPI_SetCsPolarity(unsigned int u32csPolarity)
{
    int          s32Ret;
    unsigned int u32SetMode;
    unsigned int u32Compare;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_MODE32, &u32SetMode);
    if (s32Ret == -1)
        return s32Ret;
    if (u32csPolarity)
        u32SetMode |= SPI_CS_HIGH;
    else
        u32SetMode &= ~SPI_CS_HIGH;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_WR_MODE32, &u32SetMode);
    if (s32Ret == -1)
        return s32Ret;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_MODE32, &u32Compare);
    if (s32Ret == -1)
        return s32Ret;

    if (u32Compare != u32SetMode)
        return -1;

    return 0;
}

static int _SS_SPI_SetLsb(unsigned int u32lsb)
{
    int          s32Ret;
    unsigned int u32Compare;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_WR_LSB_FIRST, &u32lsb);
    if (s32Ret == -1)
        return s32Ret;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_LSB_FIRST, &u32Compare);
    if (s32Ret == -1)
        return s32Ret;

    if (u32Compare != u32lsb)
        return -1;

    return 0;
}

static int _SS_SPI_SetSpeed(unsigned int u32Speed)
{
    int          s32Ret;
    unsigned int u32Compare;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_WR_MAX_SPEED_HZ, &u32Speed);
    if (s32Ret == -1)
        return s32Ret;

    s32Ret = ioctl(u32SPIDevFd, SPI_IOC_RD_MAX_SPEED_HZ, &u32Compare);
    if (s32Ret == -1)
        return s32Ret;

    if (u32Compare != u32Speed)
        return -1;

    return 0;
}

int SS_SPI_Read(struct SS_SPI_Packet_t *pstPacket)
{
    int                s32Ret;
    char *             ctrlbuf = g_PData->ctrlbuf;
    struct ctrl_mesg * pstcmesg;
    unsigned long long u64Offet        = 0;
    unsigned char      bDataAlmostDone = 0;
    unsigned char      u8Ack           = 0;
    unsigned int       u32AckCnt       = 3;
    pthread_mutex_lock(&g_PData->cur_ctrlmesg_lock);
    memset(ctrlbuf, 0, PSPI_CTRL_MSG_SIZE);

    pstcmesg = (struct ctrl_mesg *)ctrlbuf;
    LOG_INFO("enter Pspi_Recv ,slave state(%d)\n", g_PData->cur_ctrlmesg.spi_state);
    /*master first read the slave state*/
    s32Ret = _SS_SPI_Write(g_PData, &g_PData->cur_ctrlmesg, PSPI_CTRL_MSG_SIZE);
    if (s32Ret < 0)
    {
        LOG_ERROR("write ctrl failed\n");
        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
        return -1;
    }

    LOG_INFO("success send the slave state\n");

    pstcmesg = (struct ctrl_mesg *)ctrlbuf;
    memset(ctrlbuf, 0, PSPI_CTRL_MSG_SIZE);
    memset(pstcmesg, 0, PSPI_CTRL_MSG_SIZE);
    s32Ret = _SS_SPI_Read(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
    if (s32Ret < 0)
    {
        LOG_ERROR("read ctrl failed\n");
        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
        return -1;
    }

    LOG_INFO("slave state =%x cmd=0x%x next_rbuf_len=0x%x\n", pstcmesg->spi_state, pstcmesg->cmd,
             pstcmesg->next_rbuf_len);

IDENITIFY_CMD:
    switch (pstcmesg->cmd)
    {
        case SPI_CMD_WRITE:
            LOG_INFO("master want write.\n");
            g_PData->cur_ctrlmesg.spi_state = SPI_STATE_WRITE;
            memset(pstcmesg, 0, PSPI_CTRL_MSG_SIZE);

            while (1)
            {
                memset(ctrlbuf, 0, PSPI_CTRL_MSG_SIZE);

                s32Ret = _SS_SPI_Read(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
                if (s32Ret < 0)
                {
                    LOG_ERROR("read ctrl failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }
                LOG_INFO("offset %llu\n", u64Offet);
                LOG_INFO("next buffer %u\n", pstcmesg->next_rbuf_len);

                s32Ret = _SS_SPI_Read(g_PData, pstPacket->buf + u64Offet, pstcmesg->next_rbuf_len);
                if (s32Ret < 0)
                {
                    LOG_ERROR("read ctrl failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }

                u64Offet += pstcmesg->next_rbuf_len;
                if (u64Offet == pstPacket->buf_num)
                {
                    bDataAlmostDone = 1;
                }

                if (bDataAlmostDone)
                {
                    g_PData->cur_ctrlmesg.spi_state = SPI_STATE_IDLE;
                    LOG_INFO("read all data done!\n");
                    bDataAlmostDone = 0;
                    break;
                }
            }
            free(g_PData->slvrecv_buf);
            break;
        case SPI_CMD_RESET:
            LOG_INFO("master want reset.\n");
            pstPacket->buf     = NULL;
            pstPacket->buf_num = 0;
            break;
        default:
            LOG_INFO("%s unknown pspi cmd\n", __func__);
            u32AckCnt = 2;
            /*slave ack the cmd*/
            u8Ack  = NACK;
            s32Ret = _SS_SPI_Write(g_PData, &u8Ack, 1);
            if (s32Ret < 0)
            {
                LOG_ERROR("write cmd NACK failed\n");
                return -1;
            }

            memset(pstcmesg, 0, PSPI_CTRL_MSG_SIZE);
            s32Ret = _SS_SPI_Read(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
            if (s32Ret < 0)
            {
                LOG_ERROR("retry read cmd failed\n");
                return -1;
            }
            if (u32AckCnt)
            {
                u32AckCnt--;
                goto IDENITIFY_CMD;
            }
            if (u32AckCnt == 0)
            {
                LOG_ERROR("retry read cmd 2 times still fail.\n");
                return -1;
            }
            break;
    }
    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
    return 0;
}

int SS_SPI_Write(struct SS_SPI_Packet_t *pstPacket)
{
    int               s32Ret;
    char *            ctrlbuf = g_PData->ctrlbuf;
    struct ctrl_mesg *pstcmesg;
    unsigned char     u8Ack = 0;
    struct timespec   stStart, stStop;
    if (g_PData == NULL)
    {
        LOG_ERROR("Please init pspi slave\n");
        return -1;
    }

    if (pstPacket->buf == NULL)
    {
        LOG_ERROR("Please check buf\n");
        return -1;
    }

    if (pstPacket->buf_num <= 0)
    {
        LOG_ERROR("Please check buf length\n");
        return -1;
    }
    LOG_INFO("enter\n");
    pthread_mutex_lock(&g_PData->cur_ctrlmesg_lock);
    LOG_INFO("get mutex\n");
    if (g_PData->cur_ctrlmesg.spi_state == SPI_STATE_READ || g_PData->cur_ctrlmesg.spi_state == SPI_STATE_WRITE)
    {
        LOG_INFO("slave already is read or write.\n");
        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
        return 0;
    }
retry:
    clock_gettime(CLOCK_MONOTONIC, &stStart);
    g_PData->cur_ctrlmesg.spi_state = SPI_STATE_READ;
    g_PData->slvxmit_buf            = pstPacket->buf;
    g_PData->slvxmit_len            = pstPacket->buf_num;
    memset(ctrlbuf, 0, PSPI_CTRL_MSG_SIZE);

    pstcmesg = (struct ctrl_mesg *)ctrlbuf;

    /*master first read the slave state*/
    s32Ret = _SS_SPI_Write(g_PData, &g_PData->cur_ctrlmesg, PSPI_CTRL_MSG_SIZE);
    if (s32Ret < 0)
    {
        LOG_ERROR("write ctrl failed\n");
        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
        return -1;
    }
    LOG_INFO("success send the slave state\n");
    pstcmesg = (struct ctrl_mesg *)ctrlbuf;
    memset(ctrlbuf, 0, PSPI_CTRL_MSG_SIZE);
    memset(pstcmesg, 0, PSPI_CTRL_MSG_SIZE);
    s32Ret = _SS_SPI_Read(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
    if (s32Ret < 0)
    {
        LOG_ERROR("read ctrl failed\n");
        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
        return -1;
    }

    LOG_INFO("slave state =%x cmd=0x%x next_rbuf_len=0x%x \n", pstcmesg->spi_state, pstcmesg->cmd,
             pstcmesg->next_rbuf_len);

    if (pstcmesg->cmd == SPI_CMD_RESET)
    {
        LOG_INFO("master want reset.\n");
        goto retry;
    }
    memset(ctrlbuf, 0, PSPI_CTRL_MSG_SIZE);
    /*slave ack the cmd*/
    u8Ack  = ACK;
    s32Ret = _SS_SPI_Write(g_PData, &u8Ack, 1);
    if (s32Ret < 0)
    {
        LOG_ERROR("write cmd ack failed\n");
        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
        return -1;
    }

    if (g_PData->cur_ctrlmesg.spi_state == SPI_STATE_READ)
    {
        size_t num_full_chunks = (g_PData->slvxmit_len) / CHUNK_SIZE;
        size_t remaining_bytes = (g_PData->slvxmit_len) % CHUNK_SIZE;
        LOG_INFO("num_full_chunks:%d,remaining_bytes :%d\n", num_full_chunks, remaining_bytes);
        if (g_PData->slvxmit_buf != NULL)
        {
            // process 4KB chunk
            for (size_t i = 0; i < num_full_chunks; ++i)
            {
                size_t start = i * CHUNK_SIZE;

                // LOG_INFO("Chunk %zu: Offset %zu, Size %zu\n", i + 1, start, CHUNK_SIZE);
                pstcmesg->cmd           = SPI_CMD_READ;
                pstcmesg->next_rbuf_len = CHUNK_SIZE;
                s32Ret                  = _SS_SPI_Write(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
                if (s32Ret < 0)
                {
                    LOG_ERROR("write  ctrl in data xmit failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }
                // LOG_INFO("Chunk %zu ctrl send success\n", i + 1);

                /*read next_rbuf_len ack*/
                s32Ret = _SS_SPI_Read(g_PData, &u8Ack, 1);
                if (s32Ret < 0)
                {
                    LOG_ERROR("read next_rbuf_len  ack failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }

                if (u8Ack == RETRY)
                {
                    s32Ret = _SS_SPI_Write(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
                    if (s32Ret < 0)
                    {
                        LOG_ERROR("write  ctrl in data xmit\n");
                        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                        return -1;
                    }

                    s32Ret = _SS_SPI_Read(g_PData, &u8Ack, 1);
                    if (s32Ret < 0 || u8Ack != ACK)
                    {
                        LOG_ERROR("read next_rbuf_len  ack failed\n");
                        pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                        return -1;
                    }
                }

                // here to handle the 4KB buffer of ximit buff
                s32Ret = _SS_SPI_Write(g_PData, g_PData->slvxmit_buf + start, CHUNK_SIZE);
                if (s32Ret < 0)
                {
                    LOG_ERROR("write data  in xmit\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }
                /*read data ack*/
                s32Ret = _SS_SPI_Read(g_PData, &u8Ack, 1);
                if (s32Ret < 0 || u8Ack == NACK)
                {
                    LOG_ERROR("read next_rbuf_len  ack failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }

                // LOG_INFO("Chunk %zu data send success\n", i + 1);
            }

            // process last chunk that less than 4KB
            if (remaining_bytes > 0)
            {
                size_t start = num_full_chunks * CHUNK_SIZE;
                // LOG_INFO("Last Chunk %zu: Offset %zu, Size %zu\n", num_full_chunks + 1, start,
                // remaining_bytes);
                pstcmesg->cmd           = SPI_CMD_READ;
                pstcmesg->next_rbuf_len = remaining_bytes;
                s32Ret                  = _SS_SPI_Write(g_PData, ctrlbuf, PSPI_CTRL_MSG_SIZE);
                if (s32Ret < 0)
                {
                    LOG_ERROR("write	ctrl in data xmit\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }
                // LOG_INFO("Last Chunk success send ctrl\n");

                /*read next_rbuf_len ack*/
                s32Ret = _SS_SPI_Read(g_PData, &u8Ack, 1);
                if (s32Ret < 0 || u8Ack == NACK)
                {
                    LOG_ERROR("read next_rbuf_len  ack failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }

                // here to handle the 4KB buffer of ximit buff

                s32Ret = _SS_SPI_Write(g_PData, g_PData->slvxmit_buf + start, remaining_bytes);
                if (s32Ret < 0)
                {
                    LOG_ERROR("write data  in xmit\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }
                /*read data ack*/
                s32Ret = _SS_SPI_Read(g_PData, &u8Ack, 1);
                if (s32Ret < 0 || u8Ack == NACK)
                {
                    LOG_ERROR("read next_rbuf_len  ack failed\n");
                    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);
                    return -1;
                }
            }
        }

        g_PData->cur_ctrlmesg.spi_state = SPI_STATE_IDLE;
        clock_gettime(CLOCK_MONOTONIC, &stStop);
    }
    pthread_mutex_unlock(&g_PData->cur_ctrlmesg_lock);

    return 0;
}

int SS_SPI_Setup(struct SS_SPI_Config_t *pstCfg)
{
    int s32Ret;

    if (pstCfg->spidev == NULL)
    {
        LOG_ERROR("Not specify spi device.\n");
        return -1;
    }
    u32SPIDevFd = open(pstCfg->spidev, O_RDWR);
    if (u32SPIDevFd == -1)
    {
        LOG_ERROR("open pspi failed\n");
        return -1;
    }
    s32Ret = _SS_SPI_SetClkMode(pstCfg->clk_mode);
    if (s32Ret != 0)
    {
        LOG_ERROR("clock mode set failed.\n");
        return -1;
    }

    s32Ret = _SS_SPI_SetBitPerWord(pstCfg->bits_per_word);
    if (s32Ret != 0)
    {
        LOG_ERROR("bits per  word set failed.\n");
        return -1;
    }

    s32Ret = _SS_SPI_SetCsPolarity(pstCfg->cs_polarity);
    if (s32Ret != 0)
    {
        LOG_ERROR("cs_polarity set failed.\n");
        return -1;
    }

    s32Ret = _SS_SPI_SetLsb(pstCfg->lsb);
    if (s32Ret != 0)
    {
        LOG_ERROR("lsb set failed.\n");
        return -1;
    }

    s32Ret = _SS_SPI_SetSpeed(pstCfg->speed_hz);
    if (s32Ret != 0)
    {
        LOG_ERROR("speed set failed.\n");
        return -1;
    }

    return s32Ret;
}

int SS_SPI_Init(void)
{
    g_PData = malloc(sizeof(struct spi_data));
    if (!g_PData)
    {
        LOG_ERROR("can't allocate internal spi_data.\n");
        return -1;
    }
    g_PData->ctrlbuf = malloc(PSPI_CTRL_MSG_SIZE);
    if (!g_PData)
    {
        LOG_ERROR("can't allocate internal ctrlbuf.\n");
        return -1;
    }
    g_PData->slvxmit_buf = NULL;
    g_PData->slvrecv_buf = NULL;

    pthread_mutex_init(&g_PData->cur_ctrlmesg_lock, NULL);
    memset(&g_PData->cur_ctrlmesg, 0x0, sizeof(struct ctrl_mesg));

    return 0;
}

int SS_SPI_DeInit(void)
{
    if (u32SPIDevFd > 0)
    {
        close(u32SPIDevFd);
    }

    pthread_mutex_destroy(&g_PData->cur_ctrlmesg_lock);
    if (g_PData->ctrlbuf != NULL)
    {
        free(g_PData->ctrlbuf);
    }

    memset(g_PData, 0x0, sizeof(struct spi_data));
    if (g_PData != NULL)
    {
        free(g_PData);
    }
    return 0;
}
