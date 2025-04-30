/*
 * module_vdec.cpp- Sigmastar
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

#include "mid_common.h"
#include "module_common.h"
#include "module_vdec.h"
#include <pthread.h>

#if (CARDV_VDEC_ENABLE)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_VdecModAttr_t gstVdecModule = {0};
extern MI_S32       g_s32SocId;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static int FindStartCode2(unsigned char *Buf)
{
    if ((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 1))
        return 0;
    else
        return 1;
}

static int FindStartCode3(unsigned char *Buf)
{
    if ((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 0) || (Buf[3] != 1))
        return 0;
    else
        return 1;
}

NALU_t *AllocNALU(int buffersize)
{
    NALU_t *n;
    if ((n = (NALU_t *)calloc(1, sizeof(NALU_t))) == NULL)
    {
        printf("AllocNALU: n");
        exit(0);
    }
    n->max_size = buffersize;
    if ((n->buf = (char *)calloc(buffersize, sizeof(char))) == NULL)
    {
        free(n);
        printf("AllocNALU: n->buf");
        exit(0);
    }
    return n;
}

void FreeNALU(NALU_t *n)
{
    if (n)
    {
        if (n->buf)
        {
            free(n->buf);
            n->buf = NULL;
        }
        free(n);
    }
}

int GetAnnexbNALU(NALU_t *nalu, FILE *readfp)
{
    int            pos = 0;
    int            StartCodeFound, rewind;
    unsigned char *Buf;
    int            info2 = 0, info3 = 0;

    if ((Buf = (unsigned char *)calloc(nalu->max_size, sizeof(char))) == NULL)
        printf("Could not allocate Buf memory\n");

    if (3 != fread(Buf, 1, 3, readfp))
    {
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2(Buf);
    if (info2 != 1)
    {
        if (1 != fread(Buf + 3, 1, 1, readfp))
        {
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3(Buf);
        if (info3 != 1)
        {
            free(Buf);
            return -1;
        }
        else
        {
            pos                       = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else
    {
        nalu->startcodeprefix_len = 3;
        pos                       = 3;
    }
    StartCodeFound = 0;
    info2          = 0;
    info3          = 0;
    while (!StartCodeFound)
    {
        if (feof(readfp))
        {
            nalu->len = (pos - 1) - nalu->startcodeprefix_len;
            memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            free(Buf);
            fseek(readfp, 0, 0);
            return pos - 1;
        }
        Buf[pos++] = fgetc(readfp);
        info3      = FindStartCode3(&Buf[pos - 4]);
        if (info3 != 1)
            info2 = FindStartCode2(&Buf[pos - 3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }
    rewind = (info3 == 1) ? -4 : -3;
    if (0 != fseek(readfp, rewind, SEEK_CUR))
    {
        free(Buf);
        printf("Cannot fseek in the bit stream file\n");
    }
    nalu->len = (pos + rewind);
    memcpy(nalu->buf, &Buf[0], nalu->len);
    free(Buf);
    return (pos + rewind);
}

static void *_PutVdecInputDataThread(void *args)
{
    MI_S32                s32Ret         = MI_SUCCESS;
    MI_S32                s32TimeOutMs   = 0;
    CarDV_VdecChnAttr_t * pstVdecChnAttr = (CarDV_VdecChnAttr_t *)args;
    MI_VDEC_DEV           u32VdecDevId   = CARDV_VDEC_DEV_ID;
    MI_VDEC_CHN           u32VdecChn     = pstVdecChnAttr->u32ChnId;
    MI_U64                u64Pts;
    FILE *                readfp = NULL;
    MI_VDEC_VideoStream_t stVdecStream;
    MI_U32                u32FpBackLen = 0; // if send stream failed, file pointer back length
    NALU_t *              n            = NULL;

    n = AllocNALU(512 * 1024);
    if (!n)
    {
        printf("alloc fail\n");
        return NULL;
    }

    readfp = fopen(pstVdecChnAttr->szFileName, "rb");
    if (!readfp)
    {
        printf("open %s fail\n", pstVdecChnAttr->szFileName);
        FreeNALU(n);
        return NULL;
    }

    printf("open %s success, vdec chn:%d\n", pstVdecChnAttr->szFileName, u32VdecChn);

    while (!pstVdecChnAttr->bThreadExit)
    {
        MI_SYS_GetCurPts(g_s32SocId, &u64Pts);
        GetAnnexbNALU(n, readfp);
        stVdecStream.pu8Addr = (MI_U8 *)n->buf;
        if (9 == n->len && 0 == *(n->buf) && 0 == *(n->buf + 1) && 0 == *(n->buf + 2) && 1 == *(n->buf + 3)
            && 0x68 == *(n->buf + 4) && 0 == *(n->buf + n->len - 1))
        {
            printf("pps data:0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", *(n->buf), *(n->buf + 1),
                   *(n->buf + 2), *(n->buf + 3), *(n->buf + 4), *(n->buf + 5), *(n->buf + 6), *(n->buf + 7),
                   *(n->buf + 8));
            stVdecStream.u32Len = 8;
        }
        else
        {
            stVdecStream.u32Len = n->len;
        }
        stVdecStream.u64PTS       = u64Pts;
        stVdecStream.bEndOfFrame  = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len; // back length

        if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2]
            && 0x01 == stVdecStream.pu8Addr[3]
            && (0x65 == stVdecStream.pu8Addr[4] || 0x61 == stVdecStream.pu8Addr[4] || 0x26 == stVdecStream.pu8Addr[4]
                || 0x02 == stVdecStream.pu8Addr[4] || 0x41 == stVdecStream.pu8Addr[4]))
        {
            usleep(1000000 / pstVdecChnAttr->u32Fps);
        }

        if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(u32VdecDevId, u32VdecChn, &stVdecStream, s32TimeOutMs)))
        {
            printf("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", u32VdecChn, s32Ret);
            fseek(readfp, -u32FpBackLen, SEEK_CUR);
        }
    }

    FreeNALU(n);
    fclose(readfp);

    return NULL;
}

MI_S32 CarDV_VdecModuleInit(MI_VDEC_CHN u32VdecChn)
{
    MI_VDEC_DEV          u32VdecDevId = CARDV_VDEC_DEV_ID;
    MI_VDEC_ChnAttr_t    stVdecChnAttr;
    CarDV_VdecChnAttr_t *pstVdecChnAttr = &gstVdecModule.stVdecChnAttr[u32VdecChn];
    char                 taskName[64];

    if (pstVdecChnAttr->bUsed && pstVdecChnAttr->bCreate == FALSE)
    {
        memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 1;
        stVdecChnAttr.eVideoMode                     = E_MI_VDEC_VIDEO_MODE_FRAME;
        stVdecChnAttr.u32PicWidth                    = pstVdecChnAttr->u32MaxW;
        stVdecChnAttr.u32PicHeight                   = pstVdecChnAttr->u32MaxH;
        stVdecChnAttr.eCodecType                     = pstVdecChnAttr->eCodecType;
        stVdecChnAttr.eDpbBufMode                    = E_MI_VDEC_DPB_MODE_INPLACE_ONE_BUF;
        stVdecChnAttr.u32Priority                    = 0;
        stVdecChnAttr.u32BufSize                     = pstVdecChnAttr->u32StreamBufSize;
        CARDVCHECKRESULT(MI_VDEC_CreateChn(u32VdecDevId, u32VdecChn, &stVdecChnAttr));
        CARDVCHECKRESULT(MI_VDEC_StartChn(u32VdecDevId, u32VdecChn));
        pstVdecChnAttr->bCreate = TRUE;

        pstVdecChnAttr->bThreadExit = FALSE;
        pthread_create(&pstVdecChnAttr->pPutDataThread, NULL, _PutVdecInputDataThread, (void *)pstVdecChnAttr);
        sprintf(taskName, "vdec%d_task", u32VdecChn);
        pthread_setname_np(pstVdecChnAttr->pPutDataThread, taskName);
    }

    return 0;
}

MI_S32 CarDV_VdecModuleUnInit(MI_VDEC_CHN u32VdecChn)
{
    MI_VDEC_DEV          u32VdecDevId   = CARDV_VDEC_DEV_ID;
    CarDV_VdecChnAttr_t *pstVdecChnAttr = &gstVdecModule.stVdecChnAttr[u32VdecChn];

    if (pstVdecChnAttr->bUsed && pstVdecChnAttr->bCreate == TRUE)
    {
        pstVdecChnAttr->bThreadExit = TRUE;
        pthread_join(pstVdecChnAttr->pPutDataThread, NULL);

        CARDVCHECKRESULT(MI_VDEC_StopChn(u32VdecDevId, u32VdecChn));
        CARDVCHECKRESULT(MI_VDEC_DestroyChn(u32VdecDevId, u32VdecChn));
        pstVdecChnAttr->bCreate = FALSE;
    }

    return 0;
}
#endif