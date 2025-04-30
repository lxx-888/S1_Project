/*
 * module_md.cpp- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
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

#include "module_common.h"
#include "module_config.h"
#include "module_system.h"

#if (CARDV_VDF_ENABLE)

#include "module_md.h"
#include "module_vdf.h"
#include "module_scl.h"
#include "module_vdisp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "mid_utils.h"
#include "mid_common.h"

static pthread_t g_pthread;
static BOOL      g_bExit = FALSE;

MI_S32 g_MdtLevel = 0;
MI_S32 g_MdSens   = 0;

int MD_VDF_Set_Attr(int param)
{
    MI_U32                  u32VdfChn    = 0;
    CarDV_VdfModAttr_t *    pstVdfModule = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;
    MI_VDF_ChnAttr_t *      pstAttr;

    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        if (pstVdfChnAttr->bUsed)
        {
            pstAttr                                           = &pstVdfChnAttr->stChnAttr;
            pstAttr->enWorkMode                               = E_MI_VDF_WORK_MODE_MD;
            pstAttr->stMdAttr.u8Enable                        = 1;
            pstAttr->stMdAttr.u8MdBufCnt                      = 4;
            pstAttr->stMdAttr.u8VDFIntvl                      = 0;
            pstAttr->stMdAttr.ccl_ctrl.u16InitAreaThr         = 8;
            pstAttr->stMdAttr.ccl_ctrl.u16Step                = 2;
            pstAttr->stMdAttr.stMdDynamicParamsIn.sensitivity = 80;
            pstAttr->stMdAttr.stMdDynamicParamsIn.md_thr      = 16;
            pstAttr->stMdAttr.stMdDynamicParamsIn.obj_num_max = 0;
            pstAttr->stMdAttr.stMdStaticParamsIn.width        = pstVdfChnAttr->stSize.u16Width;
            pstAttr->stMdAttr.stMdStaticParamsIn.height       = pstVdfChnAttr->stSize.u16Height;
            pstAttr->stMdAttr.stMdStaticParamsIn.stride       = pstVdfChnAttr->stSize.u16Width;
            pstAttr->stMdAttr.stMdStaticParamsIn.color        = 1;
            pstAttr->stMdAttr.stMdStaticParamsIn.mb_size      = MDMB_MODE_MB_8x8;
            if (param == 2)
            {
                pstAttr->stMdAttr.stMdStaticParamsIn.md_alg_mode = MDALG_MODE_FG;
                pstAttr->stMdAttr.stMdDynamicParamsIn.learn_rate = 2000;
            }
            else
            {
                pstAttr->stMdAttr.stMdStaticParamsIn.md_alg_mode = MDALG_MODE_FRAMEDIFF; // MDALG_MODE_SAD;
                pstAttr->stMdAttr.stMdDynamicParamsIn.learn_rate = 128;
            }
            pstAttr->stMdAttr.stMdStaticParamsIn.sad_out_ctrl    = MDSAD_OUT_CTRL_8BIT_SAD;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.num      = 4;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x = 0;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y = 0;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x = pstVdfChnAttr->stSize.u16Width - 1;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y = 0;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x = pstVdfChnAttr->stSize.u16Width - 1;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y = pstVdfChnAttr->stSize.u16Height - 1;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x = 0;
            pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y = pstVdfChnAttr->stSize.u16Height - 1;

            printf("MD chn:%d width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)\n", pstVdfChnAttr->u8ChnId,
                   pstAttr->stMdAttr.stMdStaticParamsIn.width, pstAttr->stMdAttr.stMdStaticParamsIn.height,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x,
                   pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y);
        }
    }

    return 0;
}

int MD_SetResolution(void)
{
    MI_U32                  u32VdfChn    = 0;
    CarDV_VdfModAttr_t *    pstVdfModule = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;

    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        if (pstVdfChnAttr->bUsed)
        {
            MI_U32        u32BindDevId = pstVdfChnAttr->stBindParam.stChnPort.u32DevId;
            MI_U32        u32BindChn   = pstVdfChnAttr->stBindParam.stChnPort.u32ChnId;
            MI_U32        u32BindPort  = pstVdfChnAttr->stBindParam.stChnPort.u32PortId;
            MI_ModuleId_e eBindModule  = pstVdfChnAttr->stBindParam.stChnPort.eModId;

            if (E_MI_MODULE_ID_SCL == eBindModule)
            {
                MI_SCL_OutPortParam_t stSclOutputParam;
                memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
                CARDVCHECKRESULT(
                    MI_SCL_GetOutputPortParam((MI_SCL_DEV)u32BindDevId, u32BindChn, u32BindPort, &stSclOutputParam));
                pstVdfChnAttr->stSize.u16Width  = stSclOutputParam.stSCLOutputSize.u16Width;
                pstVdfChnAttr->stSize.u16Height = stSclOutputParam.stSCLOutputSize.u16Height;
            }
#if (CARDV_VDISP_ENABLE)
            else if (E_MI_MODULE_ID_VDISP == eBindModule)
            {
                MI_VDISP_OutputPortAttr_t stVdispOutputAttr;
                memset(&stVdispOutputAttr, 0x0, sizeof(MI_VDISP_OutputPortAttr_t));
                CARDVCHECKRESULT(MI_VDISP_GetOutputPortAttr((MI_VDISP_DEV)u32BindDevId, (MI_VDISP_PORT)u32BindPort,
                                                            &stVdispOutputAttr));
                pstVdfChnAttr->stSize.u16Width  = stVdispOutputAttr.u32Width;
                pstVdfChnAttr->stSize.u16Height = stVdispOutputAttr.u32Height;
            }
#endif
            else
            {
                printf("NOT support MD chn [%d] input module [%d]\n", u32VdfChn, eBindModule);
                return -1;
            }
        }
    }

    return 0;
}

int MD_BindVDF(MI_U8 u8ChnId, MI_BOOL bBind)
{
    CarDV_VdfModAttr_t *    pstVdfModule  = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u8ChnId];
    CarDV_Sys_BindInfo_T    stBindInfo;

    memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId    = pstVdfChnAttr->stBindParam.stChnPort.eModId;
    stBindInfo.stSrcChnPort.u32DevId  = pstVdfChnAttr->stBindParam.stChnPort.u32DevId;
    stBindInfo.stSrcChnPort.u32ChnId  = pstVdfChnAttr->stBindParam.stChnPort.u32ChnId;
    stBindInfo.stSrcChnPort.u32PortId = pstVdfChnAttr->stBindParam.stChnPort.u32PortId;
    stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_VDF;
    stBindInfo.stDstChnPort.u32DevId  = 0;
    stBindInfo.stDstChnPort.u32ChnId  = pstVdfChnAttr->u8ChnId;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate          = pstVdfChnAttr->stBindParam.u32SrcFrmrate;
    stBindInfo.u32DstFrmrate          = pstVdfChnAttr->stBindParam.u32DstFrmrate;
    stBindInfo.eBindType              = pstVdfChnAttr->stBindParam.eBindType;
    if (bBind && pstVdfChnAttr->bBind == FALSE)
    {
        CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
        pstVdfChnAttr->bBind = TRUE;
    }
    else if (bBind == FALSE && pstVdfChnAttr->bBind)
    {
        CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
        pstVdfChnAttr->bBind = FALSE;
    }
    return 0;
}

int MD_VDF_Config(int param)
{
    CARDVCHECKRESULT(MD_SetResolution());
    CARDVCHECKRESULT(MD_VDF_Set_Attr(param));
    return 0;
}

void MD_MdRstData_display(MI_U8 *pu8MdRstData, int buffer_size, int col, int row)
{
    int i, j;

    printf("row %d, col %d---------------\n", row, col);
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            printf("%02d ", pu8MdRstData[i * col + j]);
        }
        printf("\n");
    }
}

// int MD_VDFMDSadMdNumCal(MI_U8 *pu8MdRstData, int i, int j, int col, int row, int cusCol, int cusRow)
// {
//     int c, r;
//     int rowIdx     = 0;
//     int sad8BitThr = 20;
//     int mdNum      = 0;

//     for (r = 0; r < cusRow; r++)
//     {
//         rowIdx = (i + r) * col + j;

//         for (c = 0; c < cusCol; c++)
//         {
//             if (pu8MdRstData[rowIdx + c] > sad8BitThr)
//                 mdNum++;
//         }
//     }

//     return mdNum;
// }

int MD_VDFMDToRectBase(MI_U8 *pu8MdRstData, int col, int row)
{
    int i, j, i2;
    int md_detect_cnt = 0;

    // int cusCol = 4; // 4 macro block Horizontal
    // int cusRow = 2; // 3 macro block vertical

    if (pu8MdRstData)
    {
        for (i = 0, i2 = 0; i < row; i++, i2 += col)
        {
            for (j = 0; j < col; j++)
            {
                // calc all macro block result
                if (pu8MdRstData[i2 + j])
                {
                    md_detect_cnt++;
                }
            }
        }
    }

    return md_detect_cnt;
}

void *MD_Task(void *argu)
{
    int                     sMdSADFlag[VDF_CHANNEL_NUM] = {0};
    int                     sMdFGFlag[VDF_CHANNEL_NUM]  = {0};
    MI_U32                  col[VDF_CHANNEL_NUM] = {0}, row[VDF_CHANNEL_NUM] = {0}, buffer_size[VDF_CHANNEL_NUM] = {0};
    int                     sShakeDelay[VDF_CHANNEL_NUM] = {0};
    MI_U8 *                 pSadData[VDF_CHANNEL_NUM]    = {NULL};
    MI_U8 *                 pFgData[VDF_CHANNEL_NUM]     = {NULL};
    CarDV_VdfModAttr_t *    pstVdfModule                 = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;
    MI_U32                  u32VdfChn;
    MI_MD_static_param_t *  pstMdStaticParamsIn;

    CARDV_THREAD();

    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        if (pstVdfChnAttr->bUsed && pstVdfChnAttr->bCreate == TRUE)
        {
            pstMdStaticParamsIn = &pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn;
            if (pstMdStaticParamsIn->mb_size == MDMB_MODE_MB_4x4)
            {
                col[u32VdfChn] = pstVdfChnAttr->stSize.u16Width >> 2;
                row[u32VdfChn] = pstVdfChnAttr->stSize.u16Height >> 2;
            }
            else if (pstMdStaticParamsIn->mb_size == MDMB_MODE_MB_8x8)
            {
                col[u32VdfChn] = pstVdfChnAttr->stSize.u16Width >> 3;  // 48
                row[u32VdfChn] = pstVdfChnAttr->stSize.u16Height >> 3; // 36
            }
            else // MDMB_MODE_MB_16x16
            {
                col[u32VdfChn] = pstVdfChnAttr->stSize.u16Width >> 4;
                row[u32VdfChn] = pstVdfChnAttr->stSize.u16Height >> 4;
            }

            buffer_size[u32VdfChn] = col[u32VdfChn] * row[u32VdfChn]; // MDSAD_OUT_CTRL_8BIT_SAD
            // if (pstMdStaticParamsIn->sad_out_ctrl == MDSAD_OUT_CTRL_16BIT_SAD)
            //     buffer_size[u32VdfChn] *= 2;

            pSadData[u32VdfChn] = (MI_U8 *)MALLOC(buffer_size[u32VdfChn]);
            if (pSadData[u32VdfChn] == NULL)
                goto L_EXIT;

            pFgData[u32VdfChn] = (MI_U8 *)MALLOC(col[u32VdfChn] * row[u32VdfChn]);
            if (pFgData[u32VdfChn] == NULL)
                goto L_EXIT;
        }
    }

    while (!g_bExit)
    {
        MI_S32          ret          = 0;
        MI_U8 *         pu8MdRstData = NULL;
        MI_VDF_Result_t stVdfResult;
        MI_S32          s32DetectCnt = 0, s32DetectMaxCnt = 0;

        usleep(1000 * 10);

        for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
        {
            pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
            if (pstVdfChnAttr->bUsed && pstVdfChnAttr->bCreate == TRUE)
            {
                sShakeDelay[u32VdfChn]++;

                memset(&stVdfResult, 0x00, sizeof(MI_VDF_Result_t));
                stVdfResult.enWorkMode = E_MI_VDF_WORK_MODE_MD;
                ret                    = MI_VDF_GetResult(pstVdfChnAttr->u8ChnId, &stVdfResult, 0);
                if ((0 == ret) && (1 == stVdfResult.stMdResult.u8Enable))
                {
                    if (stVdfResult.stMdResult.pstMdResultSad != NULL
                        && stVdfResult.stMdResult.pstMdResultStatus != NULL
                        && stVdfResult.stMdResult.pstMdResultSad->enOutCtrl == MDSAD_OUT_CTRL_8BIT_SAD
                        && (pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode == MDALG_MODE_SAD
                            || pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode
                                   == MDALG_MODE_FRAMEDIFF))
                    {
                        pu8MdRstData = (MI_U8 *)stVdfResult.stMdResult.pstMdResultStatus->paddr;
                        memcpy(pSadData[u32VdfChn], pu8MdRstData, buffer_size[u32VdfChn]);
                        sMdSADFlag[u32VdfChn] = 1;
                    }
                    else if (stVdfResult.stMdResult.pstMdResultObj != NULL
                             && pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode == MDALG_MODE_FG)
                    {
                        memcpy(pFgData[u32VdfChn], stVdfResult.stMdResult.pstMdResultStatus->paddr,
                               col[u32VdfChn] * row[u32VdfChn]);
                        sMdFGFlag[u32VdfChn] = 1;
                    }

                    MI_VDF_PutResult(pstVdfChnAttr->u8ChnId, &stVdfResult);

                    if (sMdSADFlag[u32VdfChn]
                        && (pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode == MDALG_MODE_SAD
                            || pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode
                                   == MDALG_MODE_FRAMEDIFF))
                    {
                        s32DetectCnt = MD_VDFMDToRectBase(pSadData[u32VdfChn], col[u32VdfChn], row[u32VdfChn]);
                        // if (s32DetectCnt > 0)
                        //     printf("MD [%d] detect cnt %d\n", u32VdfChn, s32DetectCnt);
                        if (s32DetectCnt > s32DetectMaxCnt)
                            s32DetectMaxCnt = s32DetectCnt;
                    }

                    if (sMdFGFlag[u32VdfChn]
                        && pstVdfChnAttr->stChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode == MDALG_MODE_FG)
                    {
                        // TODO
                    }
                    sShakeDelay[u32VdfChn] = 0;
                }
                else
                {
                    if (sShakeDelay[u32VdfChn] > 40)
                    {
                        if (sMdSADFlag[u32VdfChn])
                            sMdSADFlag[u32VdfChn] = 0;
                        if (sMdFGFlag[u32VdfChn])
                            sMdFGFlag[u32VdfChn] = 0;
                        sShakeDelay[u32VdfChn] = 0;
                    }
                }
            }
        }

        if (s32DetectMaxCnt > g_MdSens)
        {
            printf("MD rec [%d,%d]\n", s32DetectMaxCnt, g_MdSens);
            cardv_send_to_fifo(CARDV_CMD_MD_REC, sizeof(CARDV_CMD_MD_REC));
        }
    }

L_EXIT:
    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        FREEIF(pSadData[u32VdfChn]);
        FREEIF(pFgData[u32VdfChn]);
    }
    return NULL;
}

int MD_Initial(int param)
{
    MI_U32                  u32VdfChn    = 0;
    CarDV_VdfModAttr_t *    pstVdfModule = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;

    g_bExit = FALSE;
    CARDVCHECKRESULT(MD_VDF_Config(param));
    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        if (pstVdfChnAttr->bUsed && pstVdfChnAttr->bCreate == FALSE)
        {
            CARDVCHECKRESULT(MI_VDF_CreateChn(pstVdfChnAttr->u8ChnId, &pstVdfChnAttr->stChnAttr));
            CARDVCHECKRESULT(MD_BindVDF(pstVdfChnAttr->u8ChnId, TRUE));
            pstVdfChnAttr->bCreate = TRUE;
        }
    }
    CARDVCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_MD));
    sleep(1);
    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        if (pstVdfChnAttr->bUsed && pstVdfChnAttr->bCreate == TRUE)
        {
            CARDVCHECKRESULT(MI_VDF_EnableSubWindow(pstVdfChnAttr->u8ChnId, 0, 0, 1));
        }
    }
    pthread_create(&g_pthread, NULL, MD_Task, NULL);
    pthread_setname_np(g_pthread, "MD_Task");
    return 0;
}

int MD_Uninitial(void)
{
    MI_U32                  u32VdfChn    = 0;
    CarDV_VdfModAttr_t *    pstVdfModule = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;

    CARDVCHECKRESULT(MI_VDF_Stop(E_MI_VDF_WORK_MODE_MD));
    g_bExit = TRUE;
    pthread_join(g_pthread, NULL);
    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        if (pstVdfChnAttr->bUsed && pstVdfChnAttr->bCreate == TRUE)
        {
            CARDVCHECKRESULT(MD_BindVDF(pstVdfChnAttr->u8ChnId, FALSE));
            CARDVCHECKRESULT(MI_VDF_DestroyChn(pstVdfChnAttr->u8ChnId));
            pstVdfChnAttr->bCreate = FALSE;
        }
    }
    return 0;
}

int MD_Param_Change(MI_S8 *param, MI_S32 paramLen)
{
    U8                      mdParam[8];
    CarDV_VdfModAttr_t *    pstVdfModule  = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[0];

    memcpy(mdParam, param, paramLen);

    if (mdParam[0] > 0)
    {
        MI_VDF_GetChnAttr(pstVdfChnAttr->u8ChnId, &pstVdfChnAttr->stChnAttr);

        if (mdParam[2] != 0xff)
            pstVdfChnAttr->stChnAttr.stMdAttr.u8Enable = mdParam[2];
        if (mdParam[3] != 0xff)
            pstVdfChnAttr->stChnAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max = mdParam[3];
        if (mdParam[4] != 0xff)
            pstVdfChnAttr->stChnAttr.stMdAttr.stMdDynamicParamsIn.md_thr = mdParam[4];
        if (mdParam[5] != 0xff)
            pstVdfChnAttr->stChnAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = mdParam[5];
        if (0xff != mdParam[6] || 0xff != mdParam[7])
            pstVdfChnAttr->stChnAttr.stMdAttr.stMdDynamicParamsIn.learn_rate = (mdParam[6] << 8) | mdParam[7];

        printf("VdfChn=%d,u8Enable=%d,obj_num_max=%d,md_thr=%d,sensitivity=%d,learn_rate=%d\n", pstVdfChnAttr->u8ChnId,
               mdParam[2], mdParam[3], mdParam[4], mdParam[5],
               pstVdfChnAttr->stChnAttr.stMdAttr.stMdDynamicParamsIn.learn_rate);
        pstVdfChnAttr->stChnAttr.enWorkMode = E_MI_VDF_WORK_MODE_MD;
        MI_VDF_SetChnAttr(pstVdfChnAttr->u8ChnId, &pstVdfChnAttr->stChnAttr);

        if (pstVdfChnAttr->stChnAttr.stMdAttr.u8Enable)
        {
            MI_VDF_Run(pstVdfChnAttr->stChnAttr.enWorkMode);
        }
        else
        {
            MI_VDF_Stop(pstVdfChnAttr->stChnAttr.enWorkMode);
        }
    }

    return 0;
}
#endif