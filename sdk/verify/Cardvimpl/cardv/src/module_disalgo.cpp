/*
 * module_disalgo.cpp- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "module_config.h"

#if (CARDV_DISALGO_ENABLE)

#include <pthread.h>
#include <sys/ioctl.h>
#include "mid_common.h"
#include "module_disalgo.h"
#include "kd_stabilize_handle.h"

using namespace kandao;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

MI_S32           g_s32DisFd = -1;
MI_BOOL          g_bDisThreadExit;
pthread_t        g_pDisThread;
StabilizeHandle *g_stDisAlgo = NULL;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static void *_DisAlgoThread(void *args)
{
    MI_S32          s32Ret = 0;
    Dis_DispMap_t   stDispMap;
    Dis_FrameInfo_t stFrameInfo;
    Dis_ImuInfo_t   stImuInfo;
    MI_U64          u64LastImuPts = 0;
    fd_set          read_fds;
    struct timeval  TimeoutVal;

    stImuInfo.pu8FifoBuf = (MI_U8 *)MALLOC(4096);

    while (!g_bDisThreadExit)
    {
        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;
        FD_ZERO(&read_fds);
        FD_SET(g_s32DisFd, &read_fds);
        s32Ret = select(g_s32DisFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            printf("Dis select failed\n");
            continue;
        }
        else if (0 == s32Ret)
        {
            continue;
        }
        else if (!FD_ISSET(g_s32DisFd, &read_fds))
        {
            printf("Dis not detect event\n");
            continue;
        }

        s32Ret = ioctl(g_s32DisFd, DIS_CMD_GET_IMUINFO, &stImuInfo);
        if (s32Ret == 0)
        {
            // printf("fifo [%d] pts [%llu] sample [%d]\n", stImuInfo.u32FifoSize, stImuInfo.u64FrameLastPts,
            //        stImuInfo.u16SampleRate);

            std::array<float, 3>              acc;
            std::array<float, 3>              gyro;
            float                             time;
            std::vector<std::array<float, 3>> acc_data;
            std::vector<std::array<float, 3>> gyro_data;
            std::vector<float>                gyro_time;
            MI_U32                            u32SampleCnt = stImuInfo.u32FifoSize / IMU_PER_SIZE;
            MI_BOOL                           bSetImuData  = FALSE;
            MI_S16                            s16Acc[3], s16Gyro[3];

            for (MI_U32 i = 0, j = 0; i < u32SampleCnt; i++, j += IMU_PER_SIZE)
            {
#if (IMU_MODULE == IMU_ICM40607)
                if (stImuInfo.pu8FifoBuf[j] != 0x68)
                {
                    continue;
                }

                s16Acc[0]  = (MI_S16)((stImuInfo.pu8FifoBuf[j + 0x01] << 8) | stImuInfo.pu8FifoBuf[j + 0x02]);
                s16Acc[1]  = (MI_S16)((stImuInfo.pu8FifoBuf[j + 0x03] << 8) | stImuInfo.pu8FifoBuf[j + 0x04]);
                s16Acc[2]  = (MI_S16)((stImuInfo.pu8FifoBuf[j + 0x05] << 8) | stImuInfo.pu8FifoBuf[j + 0x06]);
                s16Gyro[0] = (MI_S16)((stImuInfo.pu8FifoBuf[j + 0x07] << 8) | stImuInfo.pu8FifoBuf[j + 0x08]);
                s16Gyro[1] = (MI_S16)((stImuInfo.pu8FifoBuf[j + 0x09] << 8) | stImuInfo.pu8FifoBuf[j + 0x0A]);
                s16Gyro[2] = (MI_S16)((stImuInfo.pu8FifoBuf[j + 0x0B] << 8) | stImuInfo.pu8FifoBuf[j + 0x0C]);

                acc  = {(float)s16Acc[0] / 16384.0f, (float)s16Acc[1] / 16384.0f, (float)s16Acc[2] / 16384.0f};
                gyro = {(float)s16Gyro[0] / (16.4f * 57.3f), (float)s16Gyro[1] / (16.4f * 57.3f),
                        (float)s16Gyro[2] / (16.4f * 57.3f)};
#elif (IMU_MODULE == IMU_QMI8658)
                acc  = {(float)(MI_S16)((stImuInfo.pu8FifoBuf[j + 0x00]) | (stImuInfo.pu8FifoBuf[j + 0x01] << 8)),
                       (float)(MI_S16)((stImuInfo.pu8FifoBuf[j + 0x02]) | (stImuInfo.pu8FifoBuf[j + 0x03] << 8)),
                       (float)(MI_S16)((stImuInfo.pu8FifoBuf[j + 0x04]) | (stImuInfo.pu8FifoBuf[j + 0x05] << 8))};
                gyro = {(float)(MI_S16)((stImuInfo.pu8FifoBuf[j + 0x06]) | (stImuInfo.pu8FifoBuf[j + 0x07] << 8)),
                        (float)(MI_S16)((stImuInfo.pu8FifoBuf[j + 0x08]) | (stImuInfo.pu8FifoBuf[j + 0x09] << 8)),
                        (float)(MI_S16)((stImuInfo.pu8FifoBuf[j + 0x0A]) | (stImuInfo.pu8FifoBuf[j + 0x0B] << 8))};
#endif
                if (u64LastImuPts == 0)
                {
                    time =
                        (float)(stImuInfo.u64FrameLastPts - 1000000 / stImuInfo.u16SampleRate * (u32SampleCnt - 1 - i))
                        / 1000000.0;
                }
                else
                {
                    MI_U64 u64Interval = (stImuInfo.u64FrameLastPts - u64LastImuPts) / u32SampleCnt;
                    time = (float)(stImuInfo.u64FrameLastPts - u64Interval * (u32SampleCnt - 1 - i)) / 1000000.0;
                }
                acc_data.push_back(acc);
                gyro_data.push_back(gyro);
                gyro_time.push_back(time);
                bSetImuData = TRUE;
            }

            if (bSetImuData)
            {
                u64LastImuPts = stImuInfo.u64FrameLastPts;
                g_stDisAlgo->setImuData(acc_data, gyro_data, gyro_time);
            }
        }

        ioctl(g_s32DisFd, DIS_CMD_GET_FRAMEINFO, &stFrameInfo);

        std::array<std::vector<float>, 2> maps;
        std::vector<float>                mapX;
        std::vector<float>                mapY;
        g_stDisAlgo->getMap(maps, (float)stFrameInfo.u64FramePts / 1000000.0,
                            (float)stFrameInfo.u32ShutterDuration / 1000000.0);
        mapX = maps.front();
        mapY = maps.back();

        stDispMap.u32MapXSize = mapX.size() * sizeof(float);
        stDispMap.u32MapYSize = mapY.size() * sizeof(float);
        stDispMap.pDispMapX   = &mapX[0];
        stDispMap.pDispMapY   = &mapY[0];
        ioctl(g_s32DisFd, DIS_CMD_WRITE_DISPMAP, &stDispMap);

        FD_CLR(g_s32DisFd, &read_fds);
    }

    FREEIF(stImuInfo.pu8FifoBuf);
    return NULL;
}

MI_S32 CarDV_DisAlgoInit(MI_U16 u16Width, MI_U16 u16Height)
{
    if (g_s32DisFd != -1)
    {
        printf("DIS algo only support one channel now\n");
        return -1;
    }

    g_s32DisFd = open("/dev/kandao_dis", O_RDWR);
    if (g_s32DisFd < 0)
    {
        return -1;
    }

    g_stDisAlgo = new StabilizeHandle();
    if (g_stDisAlgo)
    {
        g_stDisAlgo->init(u16Width, u16Height, 16, 16, 0.05f /* , "/mnt/mmc/imu.txt" */);
    }

    g_bDisThreadExit = FALSE;
    pthread_create(&g_pDisThread, NULL, _DisAlgoThread, NULL);
    pthread_setname_np(g_pDisThread, "dis_algo");

    return 0;
}

MI_S32 CarDV_DisAlgoUnInit(void)
{
    if (g_pDisThread)
    {
        g_bDisThreadExit = TRUE;
        pthread_join(g_pDisThread, NULL);
        g_pDisThread = 0;
    }

    if (g_stDisAlgo)
    {
        delete g_stDisAlgo;
        g_stDisAlgo = NULL;
    }

    if (g_s32DisFd >= 0)
    {
        close(g_s32DisFd);
        g_s32DisFd = -1;
    }

    return 0;
}

#endif