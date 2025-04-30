/*
 * module_ldc.cpp- Sigmastar
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
#if (CARDV_LDC_ENABLE)

#include "mid_common.h"
#include "module_common.h"
#include "module_ldc.h"
#include "module_scl.h"
#include "module_isp.h"
#include "module_disalgo.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_LdcModAttr_t gstLdcModule = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 ReadBufFromFile(char *path, void **buf, MI_U32 *Length)
{
    MI_S32        s32Ret = 0, s32ReadSize = 0;
    struct stat64 statbuff = {};
    FILE *        fp       = NULL;

    *buf = NULL;
    if (stat64(path, &statbuff) < 0)
    {
        printf("Read file not exit:(%s) !\n", path);
        s32Ret = -1;
        goto EXIT;
    }

    fp = fopen(path, "rb");
    if (!s32Ret && fp)
    {
        *buf = malloc(statbuff.st_size);
        if (*buf)
        {
            s32ReadSize = fread(*buf, 1, statbuff.st_size, fp);
        }
        if (NULL == *buf || s32ReadSize != statbuff.st_size)
        {
            printf("Failede to malloc buf:%s !\n", path);
            s32Ret = -1;
        }
        *Length = statbuff.st_size;
    }

EXIT:
    if (fp)
        fclose(fp);
    if (s32Ret && *buf)
        free(*buf);
    return s32Ret;
}

MI_S32 CarDV_LdcChannelSetCfg(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId, const char *CfgFilePath)
{
    CarDV_LdcDevAttr_t *    pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
    CarDV_LdcChannelAttr_t *pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];

    strcpy(pstLdcChnAttr->CfgFilePath, CfgFilePath);
    return 0;
}

MI_S32 CarDV_LdcChannelInit(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId)
{
    CarDV_LdcDevAttr_t *    pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
    CarDV_LdcChannelAttr_t *pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];
    MI_S32                  s32Ret        = 0;

    if (pstLdcChnAttr->bUsed == TRUE && pstLdcChnAttr->bCreate == FALSE && pstLdcChnAttr->bBypass == FALSE)
    {
        MI_LDC_ChnAttr_t stLdcChnAttr;
        memset(&stLdcChnAttr, 0x00, sizeof(MI_LDC_ChnAttr_t));

        stLdcChnAttr.eMode = pstLdcChnAttr->eWorkMode;
        if (MI_LDC_WORKMODE_DIS_GYRO == stLdcChnAttr.eMode)
        {
            stLdcChnAttr.bUseProjection3x3Matrix = pstLdcChnAttr->bUseProjection3x3Matrix;
            if (stLdcChnAttr.bUseProjection3x3Matrix)
            {
                MI_U32 u32Index = 0;
                for (u32Index = 0; u32Index < LDC_MAXTRIX_NUM; u32Index++)
                    stLdcChnAttr.as32Projection3x3Matrix[u32Index] = pstLdcChnAttr->afProjection3x3Matrix[u32Index];
            }
            stLdcChnAttr.u16FocalLength  = pstLdcChnAttr->u16FocalLength;
            stLdcChnAttr.u16UserSliceNum = pstLdcChnAttr->u16UserSliceNum;
        }

        s32Ret |= ReadBufFromFile(pstLdcChnAttr->CfgFilePath, &stLdcChnAttr.pConfigAddr, &stLdcChnAttr.u32ConfigSize);

        stLdcChnAttr.eInfoType = pstLdcChnAttr->eMapMode;
        if (MI_LDC_MAPINFOTYPE_DISPMAP == pstLdcChnAttr->eMapMode)
        {
            s32Ret |= ReadBufFromFile(pstLdcChnAttr->Xmap, &stLdcChnAttr.stDispMapInfo.pXmapAddr,
                                      &stLdcChnAttr.stDispMapInfo.u32XmapSize);
            s32Ret |= ReadBufFromFile(pstLdcChnAttr->Ymap, &stLdcChnAttr.stDispMapInfo.pYmapAddr,
                                      &stLdcChnAttr.stDispMapInfo.u32YmapSize);
        }
        else if (MI_LDC_MAPINFOTYPE_SENSORCALIB == pstLdcChnAttr->eMapMode)
        {
            s32Ret |= ReadBufFromFile(pstLdcChnAttr->SensorCalibBinPath, &stLdcChnAttr.stCalibInfo.pCalibPolyBinAddr,
                                      &stLdcChnAttr.stCalibInfo.u32CalibPolyBinSize);
        }

        if (!s32Ret)
        {
            CARDVCHECKRESULT(MI_LDC_CreateChannel(u32LdcDevId, u32LdcChnId, &stLdcChnAttr));
        }

        if (stLdcChnAttr.pConfigAddr)
            free(stLdcChnAttr.pConfigAddr);

        if (MI_LDC_MAPINFOTYPE_DISPMAP == pstLdcChnAttr->eMapMode)
        {
            if (stLdcChnAttr.stDispMapInfo.pXmapAddr)
                free(stLdcChnAttr.stDispMapInfo.pXmapAddr);
            if (stLdcChnAttr.stDispMapInfo.pYmapAddr)
                free(stLdcChnAttr.stDispMapInfo.pYmapAddr);
        }
        else if (MI_LDC_MAPINFOTYPE_SENSORCALIB == pstLdcChnAttr->eMapMode)
        {
            if (stLdcChnAttr.stCalibInfo.pCalibPolyBinAddr)
                free(stLdcChnAttr.stCalibInfo.pCalibPolyBinAddr);
        }

#if (CARDV_DISALGO_ENABLE)
        if (MI_LDC_WORKMODE_DIS_GYRO == pstLdcChnAttr->eWorkMode)
        {
            MI_LDC_OutputPortAttr_t stOutputAttr;
            CARDVCHECKRESULT(MI_LDC_GetOutputPortAttr(u32LdcDevId, u32LdcChnId, &stOutputAttr));
            CARDVCHECKRESULT(CarDV_DisAlgoInit(stOutputAttr.u16Width, stOutputAttr.u16Height));
        }
#endif
        CARDVCHECKRESULT(MI_LDC_StartChannel(u32LdcDevId, u32LdcChnId));
        pstLdcChnAttr->bCreate = TRUE;
    }

    return 0;
}

MI_S32 CarDV_LdcChannelUnInit(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId)
{
    CarDV_LdcDevAttr_t *    pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
    CarDV_LdcChannelAttr_t *pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];

    if (pstLdcChnAttr->bUsed == TRUE && pstLdcChnAttr->bCreate == TRUE)
    {
        CARDVCHECKRESULT(MI_LDC_StopChannel(u32LdcDevId, u32LdcChnId));
#if (CARDV_DISALGO_ENABLE)
        if (MI_LDC_WORKMODE_DIS_GYRO == pstLdcChnAttr->eWorkMode)
        {
            CARDVCHECKRESULT(CarDV_DisAlgoUnInit());
        }
#endif
        CARDVCHECKRESULT(MI_LDC_DestroyChannel(u32LdcDevId, u32LdcChnId));
        pstLdcChnAttr->bCreate = FALSE;
    }

    return 0;
}

MI_S32 CarDV_LdcModuleInit(MI_LDC_DEV u32LdcDevId)
{
    CarDV_LdcDevAttr_t *pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
    MI_LDC_CHN          u32LdcChnId   = 0;

    if (pstLdcDevAttr->bUsed == TRUE && pstLdcDevAttr->bCreate == FALSE)
    {
        MI_LDC_DevAttr_t stCreateDevAttr;
        memset(&stCreateDevAttr, 0x00, sizeof(MI_LDC_DevAttr_t));
        CARDVCHECKRESULT(MI_LDC_CreateDevice(u32LdcDevId, &stCreateDevAttr));
        pstLdcDevAttr->bCreate = TRUE;
    }

    for (u32LdcChnId = 0; u32LdcChnId < CARDV_MAX_LDC_CHN_NUM; u32LdcChnId++)
    {
        CARDVCHECKRESULT(CarDV_LdcChannelInit(u32LdcDevId, u32LdcChnId));
    }

    return 0;
}

MI_S32 CarDV_LdcModuleUnInit(MI_LDC_DEV u32LdcDevId)
{
    CarDV_LdcDevAttr_t *pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
    MI_LDC_CHN          u32LdcChnId   = 0;

    for (u32LdcChnId = 0; u32LdcChnId < CARDV_MAX_LDC_CHN_NUM; u32LdcChnId++)
    {
        CARDVCHECKRESULT(CarDV_LdcChannelUnInit(u32LdcDevId, u32LdcChnId));
    }

    if (pstLdcDevAttr->bUsed == TRUE && pstLdcDevAttr->bCreate == TRUE)
    {
        CARDVCHECKRESULT(MI_LDC_DestroyDevice(u32LdcDevId));
        pstLdcDevAttr->bCreate = FALSE;
    }

    return 0;
}

MI_S32 CarDV_LdcModuleBind(MI_LDC_DEV u32LdcDevId, MI_BOOL bBind)
{
    MI_LDC_CHN u32LdcChnId = 0;

    for (u32LdcChnId = 0; u32LdcChnId < CARDV_MAX_LDC_CHN_NUM; u32LdcChnId++)
    {
        CARDVCHECKRESULT(CarDV_LdcChannelBind(u32LdcDevId, u32LdcChnId, bBind));
    }

    return 0;
}

MI_S32 CarDV_LdcChannelBind(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId, MI_BOOL bBind)
{
    CarDV_LdcDevAttr_t *    pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
    CarDV_LdcChannelAttr_t *pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];

    if (pstLdcChnAttr->bUsed == TRUE && pstLdcChnAttr->bCreate == TRUE)
    {
        CarDV_Sys_BindInfo_T stBindInfo;
        memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
        stBindInfo.stSrcChnPort.eModId    = pstLdcChnAttr->stBindParam.stChnPort.eModId;
        stBindInfo.stSrcChnPort.u32DevId  = pstLdcChnAttr->stBindParam.stChnPort.u32DevId;
        stBindInfo.stSrcChnPort.u32ChnId  = pstLdcChnAttr->stBindParam.stChnPort.u32ChnId;
        stBindInfo.stSrcChnPort.u32PortId = pstLdcChnAttr->stBindParam.stChnPort.u32PortId;
        stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
        stBindInfo.stDstChnPort.u32DevId  = u32LdcDevId;
        stBindInfo.stDstChnPort.u32ChnId  = u32LdcChnId;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate          = pstLdcChnAttr->stBindParam.u32SrcFrmrate;
        stBindInfo.u32DstFrmrate          = pstLdcChnAttr->stBindParam.u32DstFrmrate;
        stBindInfo.eBindType              = pstLdcChnAttr->stBindParam.eBindType;
        if (bBind)
        {
            CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
            pstLdcChnAttr->bBind = TRUE;
        }
        else if (bBind == FALSE && pstLdcChnAttr->bBind)
        {
            CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
            pstLdcChnAttr->bBind = FALSE;
        }
    }

    return 0;
}

MI_S32 CarDV_LdcGetDevChnByVifDev(MI_VIF_DEV u32VifDev, MI_LDC_DEV *pu32LdcDevId, MI_LDC_CHN *pu32LdcChnId)
{
    MI_LDC_DEV u32LdcDevId  = 0;
    MI_LDC_CHN u32LdcChnId  = 0;
    MI_VIF_DEV u32VifDevTmp = 0xFF;

    if (pu32LdcDevId == NULL || pu32LdcChnId == NULL)
        return -1;

    for (u32LdcDevId = 0; u32LdcDevId < CARDV_MAX_LDC_DEV_NUM; u32LdcDevId++)
    {
        CarDV_LdcDevAttr_t *pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
        if (pstLdcDevAttr->bUsed == TRUE)
        {
            for (u32LdcChnId = 0; u32LdcChnId < CARDV_MAX_LDC_CHN_NUM; u32LdcChnId++)
            {
                CarDV_LdcChannelAttr_t *pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];
                if (pstLdcChnAttr->bUsed == TRUE)
                {
                    if (pstLdcChnAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_ISP)
                    {
                        CarDV_IspGetVifDev(&u32VifDevTmp, pstLdcChnAttr->stBindParam.stChnPort.u32DevId,
                                           pstLdcChnAttr->stBindParam.stChnPort.u32ChnId);
                    }
                    else if (pstLdcChnAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_SCL)
                    {
                        CarDV_SclGetVifDev(&u32VifDevTmp, pstLdcChnAttr->stBindParam.stChnPort.u32DevId,
                                           pstLdcChnAttr->stBindParam.stChnPort.u32ChnId);
                    }

                    if (u32VifDevTmp == u32VifDev)
                    {
                        *pu32LdcDevId = u32LdcDevId;
                        *pu32LdcChnId = u32LdcChnId;
                        return 0;
                    }
                }
            }
        }
    }

    return -1;
}

#endif
