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

/*
*   mi_isp_cus3a_api.h

*
*   Created on: June 27, 2018
*       Author: Jeffrey Chou
*/

#ifndef _MI_ISPIQ_H_
#define _MI_ISPIQ_H_
//#include <pthread.h>
#include "mi_common.h"
#include "mi_isp_hw_dep_datatype.h"
#include "mi_isp_datatype.h"
#include "mi_isp_iq.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MI_ISP_OK                     (0)
#define MI_ISP_NOT_SUPPORT            (1)
#define MI_ISP_FAILURE                (-1)
#define MI_ISP_API_MAX_PARM_NUMBRE    (6)
#define MI_ISP_API_ISPMID_NOT_SUPPORT (5)

    /************************************* Customer 3A API ***********************************/
    MI_S32 MI_ISP_AE_GetAeHwAvgStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_AE_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AWB_GetAwbHwAvgStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AWB_GetAwbHwAvgStatsShort(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AE_GetHisto0HwStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_HISTO_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AE_GetHisto1HwStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_HISTO_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AE_GetHisto0HwStatsShort(MI_U32 DevId, MI_U32 Channel, MI_ISP_HISTO_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AE_GetHisto1HwStatsShort(MI_U32 DevId, MI_U32 Channel, MI_ISP_HISTO_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AE_GetRgbIrHistoHwStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_RGBIR_HISTO_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_AE_GetYuvHistoHwStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_YUV_HISTO_HW_STATISTICS_t *data);
    MI_S32 MI_ISP_CUS3A_InjectModeEnable(MI_U32 DevId, MI_U32 Channel, CusInject3AEnable_t *data);
    MI_S32 MI_ISP_CUS3A_Enable(MI_U32 DevId, MI_U32 Channel, Cus3AEnable_t *data);
    MI_S32 MI_ISP_CUS3A_GetAeInitStatus(MI_U32 DevId, MI_U32 Channel, CusAEInitParam_t *data);
    MI_S32 MI_ISP_CUS3A_GetAeStatus(MI_U32 DevId, MI_U32 Channel, CusAEInfo_t *data);
    MI_S32 MI_ISP_CUS3A_SetAeParam(MI_U32 DevId, MI_U32 Channel, CusAEResult_t *data);
    MI_S32 MI_ISP_CUS3A_GetAwbStatus(MI_U32 DevId, MI_U32 Channel, CusAWBInfo_t *data);
    MI_S32 MI_ISP_CUS3A_SetAwbParam(MI_U32 DevId, MI_U32 Channel, CusAWBResult_t *data);
    MI_S32 MI_ISP_CUS3A_SetAEWindowBlockNumber(MI_U32 DevId, MI_U32 Channel, MS_CUST_AE_WIN_BLOCK_NUM_TYPE_e *data);
    MI_S32 MI_ISP_CUS3A_SetAEHistogramWindow(MI_U32 DevId, MI_U32 Channel, CusAEHistWin_t *data);
    MI_S32 MI_ISP_CUS3A_SetAWBSampling(MI_U32 DevId, MI_U32 Channel, CusAWBSample_t *data);
    MI_S32 MI_ISP_CUS3A_SetAECropSize(MI_U32 DevId, MI_U32 Channel, CusAEAWBCropSize_t *data);
    MI_S32 MI_ISP_CUS3A_SetAWBCropSize(MI_U32 DevId, MI_U32 Channel, CusAEAWBCropSize_t *data);

    MI_S32 MI_ISP_CUS3A_GetAFStats(MI_U32 DevId, MI_U32 Channel, CusAFStats_t *data);
    MI_S32 MI_ISP_CUS3A_GetPdafStats(MI_U32 DevId, MI_U32 Channel, CusPdafStats_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFWindow(MI_U32 DevId, MI_U32 Channel, CusAFWin_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFWindow(MI_U32 DevId, MI_U32 Channel, CusAFWin_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFWindowPixelCount(MI_U32 DevId, MI_U32 Channel, CusAFWinPxCnt_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFFilter(MI_U32 DevId, MI_U32 Channel, CusAFFilter_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFFilter(MI_U32 DevId, MI_U32 Channel, CusAFFilter_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFFilterSq(MI_U32 DevId, MI_U32 Channel, CusAFFilterSq_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFFilterSq(MI_U32 DevId, MI_U32 Channel, CusAFFilterSq_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFBNR(MI_U32 DevId, MI_U32 Channel, CusAFBNR_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFBNR(MI_U32 DevId, MI_U32 Channel, CusAFBNR_t *data);
    MI_S32 MI_ISP_CUS3A_GetAfStatus(MI_U32 DevId, MI_U32 Channel, CusAFInfo_t *data);
    MI_S32 MI_ISP_CUS3A_SetAfParam(MI_U32 DevId, MI_U32 Channel, CusAfResult_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFYParam(MI_U32 DevId, MI_U32 Channel, CusAFYParam_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFYParam(MI_U32 DevId, MI_U32 Channel, CusAFYParam_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFSource(MI_U32 DevId, MI_U32 Channel, CusAFSource_e *data);
    MI_S32 MI_ISP_CUS3A_GetAFSource(MI_U32 DevId, MI_U32 Channel, CusAFSource_e *data);
    MI_S32 MI_ISP_CUS3A_SetAESource(MI_U32 DevId, MI_U32 Channel, CusAESource_e *data);
    MI_S32 MI_ISP_CUS3A_GetAESource(MI_U32 DevId, MI_U32 Channel, CusAESource_e *data);
    MI_S32 MI_ISP_CUS3A_SetHISTOSource(MI_U32 DevId, MI_U32 Channel, CusHISTOSource_e *data);
    MI_S32 MI_ISP_CUS3A_GetHISTOSource(MI_U32 DevId, MI_U32 Channel, CusHISTOSource_e *data);
    MI_S32 MI_ISP_CUS3A_SetAWBSource(MI_U32 DevId, MI_U32 Channel, CusAWBSourceExposure_t *data);
    MI_S32 MI_ISP_CUS3A_GetAWBSource(MI_U32 DevId, MI_U32 Channel, CusAWBSourceExposure_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFPreFilter(MI_U32 DevId, MI_U32 Channel, CusAFPreFilter_t *data); // ikayaki not support
    MI_S32 MI_ISP_CUS3A_GetAFPreFilter(MI_U32 DevId, MI_U32 Channel, CusAFPreFilter_t *data); // ikayaki not support
    MI_S32 MI_ISP_CUS3A_SetAFFirFilter(MI_U32 DevId, MI_U32 Channel, CusAFFirFilter_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFFirFilter(MI_U32 DevId, MI_U32 Channel, CusAFFirFilter_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFYMap(MI_U32 DevId, MI_U32 Channel, CusAFYMap_t *data); // ikayaki not support
    MI_S32 MI_ISP_CUS3A_GetAFYMap(MI_U32 DevId, MI_U32 Channel, CusAFYMap_t *data); // ikayaki not support
    MI_S32 MI_ISP_CUS3A_SetAFLdg(MI_U32 DevId, MI_U32 Channel, CusAFLdg_t *data);   // ikayaki not support
    MI_S32 MI_ISP_CUS3A_GetAFLdg(MI_U32 DevId, MI_U32 Channel, CusAFLdg_t *data);   // ikayaki not support
    MI_S32 MI_ISP_CUS3A_SetAFPeakMode(MI_U32 DevId, MI_U32 Channel, CusAFPeakMode_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFPeakMode(MI_U32 DevId, MI_U32 Channel, CusAFPeakMode_t *data);
    MI_S32 MI_ISP_CUS3A_SetAFGMode(MI_U32 DevId, MI_U32 Channel, CusAFGMode_t *data);
    MI_S32 MI_ISP_CUS3A_GetAFGMode(MI_U32 DevId, MI_U32 Channel, CusAFGMode_t *data);
    MI_S32 MI_ISP_CUS3A_GetInputSize(MI_U32 DevId, MI_U32 Channel, CusAEAWBCropSize_t *data);

    MI_S32 MI_ISP_CUS3A_GetImageResolution(MI_U32 DevId, MI_U32 Channel, CusImageResolution_t *data);
    MI_S32 MI_ISP_CUS3A_EnableISPOutImage(MI_U32 DevId, MI_U32 Channel, CusISPOutImage_t *data);
    MI_S32 MI_ISP_CUS3A_GetISPOutImageCount(MI_U32 DevId, MI_U32 Channel, MI_U32 *data);
    MI_S32 MI_ISP_CUS3A_CaptureHdrRawImage(MI_U32 DevId, MI_U32 Channel, CusHdrRawImage_t *data);
    MI_S32 MI_ISP_GetFrameMetaInfo(MI_U32 DevId, MI_U32 Channel, IspFrameMetaInfo_t *data);
    MI_S32 MI_ISP_ReadSensorData(MI_U32 DevId, MI_U32 Channel, CusSensorI2cParam_t *data);
    MI_S32 MI_ISP_WriteSensorData(MI_U32 DevId, MI_U32 Channel, CusSensorI2cParam_t *data);
    MI_S32 MI_ISP_CUS3A_SetAeStatsPostProcessing(MI_U32 DevId, MI_U32 Channel, CusAeStatsPostProcessing_e *data);
    MI_S32 MI_ISP_CUS3A_SetAwbStatsPostProcessing(MI_U32 DevId, MI_U32 Channel, CusAwbStatsPostProcessing_e *data);
    MI_S32 MI_ISP_CUS3A_GetDoAeCount(MI_U32 DevId, MI_U32 Channel, MI_U32 *data);
    MI_S32 MI_ISP_CUS3A_SetEarlyAwbDone(MI_U32 DevId, MI_U32 Channel, CusEarlyAwbDoneEnable_t *data);
    MI_S32 MI_ISP_CUS3A_SetAeOpMode(MI_U32 DevId, MI_U32 Channel, CusAeOpMode_e *data);
    MI_S32 MI_ISP_CUS3A_GetIqSize(MI_U32 DevId, MI_U32 Channel, MI_U32 *data);
    MI_S32 MI_ISP_CUS3A_LoadIqData(MI_U32 DevId, MI_U32 Channel, MI_PTR *data);
    /************************************* Customer 3A API END********************************/
    /************************************* Debug  API ***********************************/
    MI_S32 MI_ISP_SetDebugLevel_AE(MI_U32 DevId, MI_U32 Channel, MI_S32 *data);
    MI_S32 MI_ISP_GetDebugLevel_AE(MI_U32 DevId, MI_U32 Channel, MI_S32 *data);
    MI_S32 MI_ISP_SetDebugLevel_AWB(MI_U32 DevId, MI_U32 Channel, MI_S32 *data);
    MI_S32 MI_ISP_GetDebugLevel_AWB(MI_U32 DevId, MI_U32 Channel, MI_S32 *data);
    MI_S32 MI_ISP_SetDebugLevel_AF(MI_U32 DevId, MI_U32 Channel, MI_S32 *data);
    MI_S32 MI_ISP_GetDebugLevel_AF(MI_U32 DevId, MI_U32 Channel, MI_S32 *data);
    /************************************* Debug  API END********************************/

    MI_S32 MI_ISP_SetAll(MI_U32 DevId, MI_U32 Channel, MI_U16 ApiId, MI_U32 ApiLen, MI_U8 *pApiBuf);
    MI_S32 MI_ISP_GetAll(MI_U32 DevId, MI_U32 Channel, MI_U16 ApiId, MI_U32 *ApiLen, MI_U8 *pApiBuf);

    typedef MI_S32 (*MI_ISP_API_AGENT_FP)(MI_ISP_IQApiHeader_t *pstData, void *pData);

    /*Api agent for SigmaStar user space 3A library*/
    MI_S32 MI_ISP_RegisterIspApiAgent(MI_U32 Dev, MI_U32 Channel, MI_ISP_API_AGENT_FP fpApiSet,
                                      MI_ISP_API_AGENT_FP fpApiGet);
    MI_S32 MI_ISP_EnableUserspace3A(MI_U32 DevId, MI_U32 Channel);
    MI_S32 MI_ISP_DisableUserspace3A(MI_U32 DevId, MI_U32 Channel);
    MI_S32 MI_ISP_ApiCmdLoadBinFile(MI_U32 DevId, MI_U32 Channel, char *filepath, MI_U32 user_key);
    MI_S32 MI_ISP_ApiCmdLoadCaliData(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CaliItem_e eCaliItem, char *filepath);

    /*Common API, get isp root path*/
    MI_S32 MI_ISP_GetIspRoot(MI_U32 DevId, MI_U32 Channel, MI_ISP_ROOT_PATH_T *data);

#ifdef __cplusplus
} // end of extern C
#endif

#endif //_MI_ISPIQ_H_
