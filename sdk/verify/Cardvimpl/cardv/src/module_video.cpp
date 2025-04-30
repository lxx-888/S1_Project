/*
 * module_video.cpp- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
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
#include "module_video.h"
#include "module_dcf.h"
#include "module_scl.h"
#include "module_isp.h"
#include "module_vdisp.h"
#include "module_ws.h"
#include "mid_vif.h"
#include "DCF.h"
#include "IPC_cardvInfo.h"
#include "libexif/exif-data.h"
#if (CARDV_ALINK_ENABLE)
#include "alink.h"
#endif

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

extern BOOL       g_camExisted[];
extern CarDV_Info carInfo;
#if (CARDV_ALINK_ENABLE)
extern AlinkDemo AlinkSever;
#endif
MI_VideoEncoder *       g_videoEncoderArray[MAX_VIDEO_NUMBER] = {NULL};
CarDV_VencAttr_t        gstVencAttr[MAX_VIDEO_NUMBER]         = {0};
CarDV_VencPrivPoolCfg_t gstVencPrivPoolCfg                    = {0};

//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

/* Create a brand-new tag with a data field of the given length, in the
 * given IFD. This is needed when exif_entry_initialize() isn't able to create
 * this type of tag itself, or the default data length it creates isn't the
 * correct length.
 */
static ExifEntry *create_tag(ExifData *exif, ExifIfd ifd, ExifTag tag, size_t len)
{
    void *     buf;
    ExifEntry *entry;

    /* Create a memory allocator to manage this ExifEntry */
    ExifMem *mem = exif_mem_new_default();
    if (mem == NULL)
    {
        /* catch an out of memory condition */
        return NULL;
    }

    /* Create a new ExifEntry using our allocator */
    entry = exif_entry_new_mem(mem);
    if (entry == NULL)
        goto L_ERR_EXIT_MEM;

    /* Allocate memory to use for holding the tag data */
    buf = exif_mem_alloc(mem, len);
    if (buf == NULL)
        goto L_ERR_EXIT_ENTRY;

    /* Fill in the entry */
    entry->data       = (unsigned char *)buf;
    entry->size       = len;
    entry->tag        = tag;
    entry->components = len;
    entry->format     = EXIF_FORMAT_UNDEFINED;

    /* Attach the ExifEntry to an IFD */
    exif_content_add_entry(exif->ifd[ifd], entry);

/* The ExifMem and ExifEntry are now owned elsewhere */
L_ERR_EXIT_ENTRY:
    exif_entry_unref(entry);
L_ERR_EXIT_MEM:
    exif_mem_unref(mem);

    return entry;
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int VideoMapToIndex(MI_U8 u8CamId, VideoStreamType_e eStreamType)
{
    for (MI_U32 i = 0; i < MAX_VIDEO_NUMBER; i++)
    {
        if (gstVencAttr[i].bUsed == FALSE)
            continue;
        if (gstVencAttr[i].eVideoType == eStreamType && gstVencAttr[i].u8CamId == u8CamId)
            return i;
    }
    return -1;
}

MI_S32 VideoInitResolution(MI_SYS_ChnPort_t *pstChnPort, MI_U32 *pu32Width, MI_U32 *pu32Height)
{
    MI_ModuleId_e eBindModule  = pstChnPort->eModId;
    MI_U32        u32BindDevId = pstChnPort->u32DevId;
    MI_U32        u32BindChn   = pstChnPort->u32ChnId;
    MI_U32        u32BindPort  = pstChnPort->u32PortId;

    if (E_MI_MODULE_ID_SCL == eBindModule)
    {
        MI_SCL_OutPortParam_t stSclOutputParam;
        memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
        CARDVCHECKRESULT(MI_SCL_GetOutputPortParam(u32BindDevId, u32BindChn, u32BindPort, &stSclOutputParam));
        *pu32Width  = stSclOutputParam.stSCLOutputSize.u16Width;
        *pu32Height = stSclOutputParam.stSCLOutputSize.u16Height;
    }
#if (CARDV_VDISP_ENABLE)
    else if (E_MI_MODULE_ID_VDISP == eBindModule)
    {
        MI_VDISP_OutputPortAttr_t stVdispOutputAttr;
        memset(&stVdispOutputAttr, 0x0, sizeof(MI_VDISP_OutputPortAttr_t));
        CARDVCHECKRESULT(
            MI_VDISP_GetOutputPortAttr((MI_VDISP_DEV)u32BindDevId, (MI_VDISP_PORT)u32BindPort, &stVdispOutputAttr));
        *pu32Width  = stVdispOutputAttr.u32Width;
        *pu32Height = stVdispOutputAttr.u32Height;
    }
#endif

    return 0;
}

MI_S32 VideoSetInputResolution(MI_SYS_ChnPort_t *pstChnPort, MI_U32 u32Width, MI_U32 u32Height)
{
    MI_ModuleId_e eBindModule  = pstChnPort->eModId;
    MI_U32        u32BindDevId = pstChnPort->u32DevId;
    MI_U32        u32BindChn   = pstChnPort->u32ChnId;
    MI_U32        u32BindPort  = pstChnPort->u32PortId;

    if (E_MI_MODULE_ID_SCL == eBindModule)
    {
        MI_SYS_WindowSize_t stPortSize;
        stPortSize.u16Width  = u32Width;
        stPortSize.u16Height = u32Height;
        CARDVCHECKRESULT(CarDV_SclChangeOutputPortSize(u32BindDevId, u32BindChn, u32BindPort, &stPortSize));
    }

    return 0;
}

MI_S32 VideoEnableInput(MI_SYS_ChnPort_t *pstChnPort, MI_BOOL bEnable)
{
    MI_ModuleId_e eBindModule  = pstChnPort->eModId;
    MI_U32        u32BindDevId = pstChnPort->u32DevId;
    MI_U32        u32BindChn   = pstChnPort->u32ChnId;
    MI_U32        u32BindPort  = pstChnPort->u32PortId;

    if (E_MI_MODULE_ID_SCL == eBindModule)
    {
        CARDVCHECKRESULT(CarDV_SclEnableOutputPort(u32BindDevId, u32BindChn, u32BindPort, bEnable));
    }

    return 0;
}

void VideoCaptureThumbnail(MI_U8 u8CamId)
{
    int nVideoIdx = 0;
    nVideoIdx     = VideoMapToIndex(u8CamId, VIDEO_STREAM_THUMB);
    if (nVideoIdx > 0)
    {
        MI_S32 param[1];
        param[0] = nVideoIdx;
        cardv_send_cmd_HP(CMD_VIDEO_CAPTURE_THUMB, (MI_S8 *)param, sizeof(param));
    }
}

void VideoCaptureRaw(MI_U8 u8CamId, MI_U8 *pu8RawBuf, MI_U32 u32RawSize)
{
    char   filename[64];
    time_t timep;
    int    fd;

    if (DCF_IsDBFormatFree(DB_PHOTO))
    {
        printf("capture RAW should NOT enable format free\n");
        return;
    }

    time(&timep);
    if (DCF_CreateFileName(DB_PHOTO, u8CamId, filename, timep, "raw") < 0)
        return;

    fd = open(filename, O_WRONLY | O_CREAT);
    if (fd < 0)
    {
        printf("error: open %s failed\n", filename);
        return;
    }

    // TBD : HDR ?
    write(fd, pu8RawBuf, u32RawSize);
    close(fd);

    printf("cap [%s]\n", filename);
    DCF_AddFile(DB_PHOTO, u8CamId, filename);
    DCF_UpdateFileSizeByName(DB_PHOTO, u8CamId, filename);
}

void VideoCaptureSnapshot(int camId, MI_U8 *pu8FrameBuf, MI_U32 u32FrameSize, MI_U8 *pu8ThumbBuf, MI_U32 u32ThumbSize)
{
#define EXIF_HEADER_SIZE (6)
#define MANUFACTURER     "SigmaStar"
#define MODEL            "CarDV"
#define SOFTWARE         "Linux"
#define ARTIST           "Sstar"

    char           filename[64];
    time_t         timep;
    int            fd;
    unsigned char *jpeg_buf            = NULL;
    int            jpeg_buf_align_size = 0;
    int            jpeg_buf_offset     = 0;

    time(&timep);
    if (DCF_CreateFileName(DB_PHOTO, camId, filename, timep, "jpg") < 0)
        return;

    fd = open(filename, O_WRONLY | O_CREAT);
    if (fd < 0)
    {
        printf("error: open %s failed\n", filename);
        return;
    }

    if (1)
    {
        /* raw EXIF header & length data */
        unsigned char exif_header[EXIF_HEADER_SIZE] = {0xff, 0xd8, 0xff, 0xe1};
        /* start of JPEG image data section */
        static const unsigned int image_data_offset = 20;
        unsigned char *           exif_data;
        unsigned int              exif_data_len;
        ExifEntry *               entry;
        ExifMem *                 mem  = exif_mem_new_default();
        ExifData *                exif = exif_data_new_mem(mem);
        if (!exif)
            goto L_ERROR;

        /* Set the image options */
        exif_data_set_option(exif, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
        exif_data_set_data_type(exif, EXIF_DATA_TYPE_COMPRESSED);
        exif_data_set_byte_order(exif, EXIF_BYTE_ORDER_INTEL);

        /* Camera ManuFacturer */
        entry =
            create_tag(exif, EXIF_IFD_0, EXIF_TAG_MAKE, sizeof(MANUFACTURER) * exif_format_get_size(EXIF_FORMAT_ASCII));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_ASCII;
        entry->components = sizeof(MANUFACTURER);
        memcpy(entry->data, MANUFACTURER, sizeof(MANUFACTURER));

        /* Camera Model */
        entry = create_tag(exif, EXIF_IFD_0, EXIF_TAG_MODEL, sizeof(MODEL) * exif_format_get_size(EXIF_FORMAT_ASCII));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_ASCII;
        entry->components = sizeof(MODEL);
        memcpy(entry->data, MODEL, sizeof(MODEL));

        /* Camera Software */
        entry =
            create_tag(exif, EXIF_IFD_0, EXIF_TAG_SOFTWARE, sizeof(SOFTWARE) * exif_format_get_size(EXIF_FORMAT_ASCII));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_ASCII;
        entry->components = sizeof(SOFTWARE);
        memcpy(entry->data, SOFTWARE, sizeof(SOFTWARE));

        /* Camera Artist */
        entry = create_tag(exif, EXIF_IFD_0, EXIF_TAG_ARTIST, sizeof(ARTIST) * exif_format_get_size(EXIF_FORMAT_ASCII));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_ASCII;
        entry->components = sizeof(ARTIST);
        memcpy(entry->data, ARTIST, sizeof(ARTIST));

        /* Camera Focal Length */
        entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH, 1 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_RATIONAL;
        entry->components = 1;
        ExifRational focallen;
        focallen.numerator   = 68;
        focallen.denominator = 10;
        exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL, focallen);

        /* Camera Aperture */
        entry =
            create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE, 1 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_RATIONAL;
        entry->components = 1;
        ExifRational aperture;
        aperture.numerator   = 36;
        aperture.denominator = 10;
        exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL, aperture);

        /* Camera Max Aperture */
        entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_MAX_APERTURE_VALUE,
                           1 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
        if (!entry)
            goto L_ERROR;
        entry->format     = EXIF_FORMAT_RATIONAL;
        entry->components = 1;
        ExifRational maxAperture;
        maxAperture.numerator   = 37;
        maxAperture.denominator = 10;
        exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL, maxAperture);

        /* Thumbnail */
        if (pu8ThumbBuf && u32ThumbSize)
        {
            exif->data = (unsigned char *)exif_mem_alloc(mem, u32ThumbSize);
            exif->size = u32ThumbSize;
            memcpy(exif->data, pu8ThumbBuf, u32ThumbSize);
        }

        /* Get a pointer to the EXIF data block we just created */
        exif_data_save_data(exif, &exif_data, &exif_data_len);
        if (exif_data == NULL)
            goto L_ERROR;

        if (DCF_IsDBFormatFree(DB_PHOTO))
        {
            jpeg_buf_align_size = ALIGN_UP(EXIF_HEADER_SIZE + exif_data_len + u32FrameSize - image_data_offset, 512);
            jpeg_buf            = (unsigned char *)MALLOC(jpeg_buf_align_size);
        }

        /* Write EXIF header & EXIF block length in big-endian order */
        exif_header[4] = (exif_data_len + 2) >> 8;
        exif_header[5] = (exif_data_len + 2) & 0xff;
        if (jpeg_buf)
        {
            memcpy(jpeg_buf, exif_header, EXIF_HEADER_SIZE);
            jpeg_buf_offset += EXIF_HEADER_SIZE;

            /* Write EXIF data block */
            memcpy(jpeg_buf + jpeg_buf_offset, exif_data, exif_data_len);
            jpeg_buf_offset += exif_data_len;

            /* Write JPEG image data, skipping the non-EXIF header */
            memcpy(jpeg_buf + jpeg_buf_offset, pu8FrameBuf + image_data_offset, u32FrameSize - image_data_offset);

            write(fd, jpeg_buf, jpeg_buf_align_size);
            free(jpeg_buf);
        }
        else
        {
            write(fd, exif_header, EXIF_HEADER_SIZE);

            /* Write EXIF data block */
            write(fd, exif_data, exif_data_len);

            /* Write JPEG image data, skipping the non-EXIF header */
            write(fd, pu8FrameBuf + image_data_offset, u32FrameSize - image_data_offset);
        }

        fdatasync(fd);

        /* The allocator we're using for ExifData is the standard one, so use
         * it directly to free this pointer.
         */
        free(exif_data);
    L_ERROR:
        exif_data_unref(exif);
        exif_mem_unref(mem);
    }

    close(fd);

    printf("cap [%s]\n", filename);
    DCF_AddFile(DB_PHOTO, camId, filename);
    DCF_UpdateFileSizeByName(DB_PHOTO, camId, filename);

    carInfo.stCapInfo.u32FileCnt = DCF_GetDBFileCntByDB(DB_PHOTO);
    IPC_CarInfo_Write_CapInfo(&carInfo.stCapInfo);
}

void VideoUserGetFrame(MI_U32 u32VencChn, MI_VENC_Stream_t *pstStream, MI_U32 u32FrameSize)
{
    // Customers TODO:
    // Customers need to implement this function according to their own need.
    // Note that the function runs as short as possible, or it maybe drops frame!
    // Here SigmaStar just dump 300 frames to file for demo.

#if (CARDV_WS_OUTPUT_ENABLE)
    CarDV_WsServerSendStream(u32VencChn, pstStream, u32FrameSize);
#endif

#if 0
    static int frameCnt[2]  = {0};
    static int fd[2]        = {0};
    char       fileName[32] = {
        0,
    };

    if (frameCnt[camId] == 0)
    {
        sprintf(fileName, "/mnt/mmc/dump_cam%d.h264", camId);
        fd[camId] = open(fileName, O_WRONLY | O_CREAT);
    }

    if (-1 != fd[camId] && frameCnt[camId] < 300)
    {
        for (int i = 0; i < STREAM_PACK_CNT && pstPack[i].u32Len; i++)
        {
            write(fd[camId], pstPack[i].pu8Addr, pstPack[i].u32Len);
        }
    }

    if (-1 != fd[camId] && frameCnt[camId] == 300)
    {
        close(fd[camId]);
    }

    frameCnt[camId]++;
#elif (CARDV_ALINK_ENABLE)
    if (streamType == VIDEO_STREAM_RTSP)
    {
        auto connections = AlinkSever.mConnections;
        if (pstPack[0].u32Len == u32FrameSize)
        {
            for (auto cid : connections)
            {
                AlinkSever.mServer->feed(cid, alink::kVideoOut, pstPack[0].pu8Addr, pstPack[0].u32Len);
            }
        }
        else
        {
            MI_U8 *pu8Buf    = (MI_U8 *)MALLOC(u32FrameSize);
            MI_U32 u32Offset = 0;
            if (pu8Buf)
            {
                for (int i = 0; i < STREAM_PACK_CNT && pstPack[i].u32Len; i++)
                {
                    memcpy(pu8Buf + u32Offset, pstPack[i].pu8Addr, pstPack[i].u32Len);
                    u32Offset += pstPack[i].u32Len;
                }
                for (auto cid : connections)
                {
                    AlinkSever.mServer->feed(cid, alink::kVideoOut, pu8Buf, u32FrameSize);
                }
                FREEIF(pu8Buf);
            }
        }
    }
#endif
}

void VideoSetLongExpo(MI_U8 u8CamId, MI_U32 u32FrameCnt, MI_U32 u32ShutterUs)
{
    MI_ISP_DEV     u32IspDevId = 0;
    MI_ISP_CHANNEL u32IspChnId = 0;
    CarDV_IspGetDevChnByCamId(u8CamId, &u32IspDevId, &u32IspChnId);
    CarDV_IspSetExpo(u32IspDevId, u32IspChnId, u32ShutterUs, u32FrameCnt);
}

void VideoSetDuration(int muxerType, MI_U64 u64TotalTime, MI_U32 u32PreRecdTime)
{
    carInfo.stRecInfo.u32CurDuration[muxerType]     = u64TotalTime;
    carInfo.stRecInfo.u32PreRecdDuration[muxerType] = u32PreRecdTime;
    IPC_CarInfo_Write_RecInfo(&carInfo.stRecInfo);
}

int videoStart(int videoIndex)
{
    if (g_videoEncoderArray[videoIndex] && g_camExisted[g_videoEncoderArray[videoIndex]->getCamId()])
        g_videoEncoderArray[videoIndex]->startVideoEncoder(TRUE);
    return 0;
}

int videoStop(int videoIndex)
{
    if (g_videoEncoderArray[videoIndex])
        g_videoEncoderArray[videoIndex]->stopVideoEncoder();
    return 0;
}

int videoCapture(int videoIndex, MI_U32 u32FileCnt)
{
    if (g_videoEncoderArray[videoIndex] && g_camExisted[g_videoEncoderArray[videoIndex]->getCamId()])
    {
        g_videoEncoderArray[videoIndex]->captureJpegFile(u32FileCnt);
    }
    return 0;
}

int videoCaptureLongExpo(int videoIndex, MI_U32 u32FileCnt, MI_U32 u32ShutterUs)
{
    if (g_videoEncoderArray[videoIndex] && g_camExisted[g_videoEncoderArray[videoIndex]->getCamId()])
    {
        g_videoEncoderArray[videoIndex]->captureLongExpoJpegFile(u32FileCnt, u32ShutterUs);
    }
    return 0;
}

int videoCaptureRaw(int videoIndex, MI_U32 u32ShutterUs)
{
    CarDV_GetRaw(g_videoEncoderArray[videoIndex]->getCamId(), u32ShutterUs);
    return 0;
}

int videoCaptureThumb(int videoIndex)
{
    if (g_videoEncoderArray[videoIndex] && g_camExisted[g_videoEncoderArray[videoIndex]->getCamId()])
    {
        g_videoEncoderArray[videoIndex]->captureVideoThumbnail();
    }
    return 0;
}

int videoInitH26x()
{
    VideoSetUserGetFrameCallback(VideoUserGetFrame);
    VideoSetDurationFrameCallback(VideoSetDuration);

    for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
    {
        if (gstVencAttr[i].bUsed == FALSE)
            continue;

        if (gstVencAttr[i].u8EncoderType != VE_H264 && gstVencAttr[i].u8EncoderType != VE_H265)
            continue;

        if (g_videoEncoderArray[i] == NULL)
        {
            g_videoEncoderArray[i] = MI_VideoEncoder::createNew(i, &gstVencAttr[i]);
            g_videoEncoderArray[i]->initVideoEncoder();
        }
    }

    return 0;
}

int videoDeInitH26x()
{
    for (int i = MAX_VIDEO_NUMBER - 1; i >= 0; i--)
    {
        if (g_videoEncoderArray[i] == NULL)
            continue;

        if (gstVencAttr[i].u8EncoderType != VE_H264 && gstVencAttr[i].u8EncoderType != VE_H265)
            continue;

        g_videoEncoderArray[i]->uninitVideoEncoder();
        delete g_videoEncoderArray[i];
        g_videoEncoderArray[i] = NULL;
    }

    return 0;
}

int videoInitJpeg()
{
    stThumbPipeInfo stThumbPipeInfo;

    VideoSetCaptureCallback(VideoCaptureSnapshot);
    VideoSetCaptureThumbCallback(VideoCaptureThumbnail);
    VideoSetExpoCallback(VideoSetLongExpo);
    CarDV_SetRawCallback(VideoCaptureRaw);

    stThumbPipeInfo.u32SclDevId     = 3;
    stThumbPipeInfo.u32UseHwSclMask = 0x20;
    stThumbPipeInfo.u16Width        = 256;
    stThumbPipeInfo.u16Height       = 144;
    VideoSetThumbPipeInfo(&stThumbPipeInfo);

    for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
    {
        if (gstVencAttr[i].bUsed == FALSE)
            continue;

        if (gstVencAttr[i].u8EncoderType != VE_JPG)
            continue;

        if (g_videoEncoderArray[i] == NULL)
        {
            g_videoEncoderArray[i] = MI_VideoEncoder::createNew(i, &gstVencAttr[i]);
            g_videoEncoderArray[i]->initVideoEncoder();
        }
    }

    return 0;
}

int videoDeInitJpeg()
{
    for (int i = MAX_VIDEO_NUMBER - 1; i >= 0; i--)
    {
        if (g_videoEncoderArray[i] == NULL)
            continue;

        if (gstVencAttr[i].u8EncoderType != VE_JPG)
            continue;

        g_videoEncoderArray[i]->uninitVideoEncoder();
        delete g_videoEncoderArray[i];
        g_videoEncoderArray[i] = NULL;
    }

    return 0;
}

int video_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_VIDEO_INIT_H26X:
        videoInitH26x();
        break;

    case CMD_VIDEO_DEINIT_H26X:
        videoDeInitH26x();
        break;

    case CMD_VIDEO_INIT_JPEG:
        videoInitJpeg();
        break;

    case CMD_VIDEO_DEINIT_JPEG:
        videoDeInitJpeg();
        break;

    case CMD_VIDEO_START: {
        MI_S8 videoParam = param[0];
        int   videoIndex = videoParam;
        if (videoIndex < MAX_VIDEO_NUMBER)
            videoStart(videoIndex);
    }
    break;

    case CMD_VIDEO_STOP: {
        MI_S8 videoParam = param[0];
        int   videoIndex = videoParam;
        if (videoIndex < MAX_VIDEO_NUMBER)
            videoStop(videoIndex);
    }
    break;

    case CMD_VIDEO_CAPTURE: {
        MI_U32 videoParam[2];
        memcpy(videoParam, param, paramLen);
        MI_U32 u32JpegFileCnt = videoParam[0];
        MI_U32 u32ShutterUs   = videoParam[1];
        DCF_CheckDeleteFilesForSpace(DB_PHOTO);
        DCF_CheckDeleteFilesForNum(DB_PHOTO, 500);
        DCF_CheckDeleteFilesForFormatFreeReserved(DB_PHOTO, u32JpegFileCnt);

        for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_CAPTURE)
            {
                if (u32ShutterUs >= 1000000)
                {
                    videoCaptureLongExpo(i, u32JpegFileCnt, u32ShutterUs);
                }
                else
                {
                    videoCapture(i, u32JpegFileCnt);
                }
            }
        }
    }
    break;

    case CMD_VIDEO_CAPTURE_RAW: {
        MI_U32 videoParam[1];
        memcpy(videoParam, param, paramLen);
        MI_U32 u32ShutterUs = videoParam[0];
        DCF_CheckDeleteFilesForSpace(DB_PHOTO);
        DCF_CheckDeleteFilesForNum(DB_PHOTO, 500);

        for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_CAPTURE)
                videoCaptureRaw(i, u32ShutterUs);
        }
    }
    break;

    case CMD_VIDEO_CAPTURE_THUMB: {
        MI_S8 videoParam = param[0];
        int   videoIndex = videoParam;
        if (videoIndex < MAX_VIDEO_NUMBER)
            videoCaptureThumb(videoIndex);
    }
    break;

    case CMD_VIDEO_SET_RESOLUTION: {
        Size_t size;
        MI_U32 veChn;
        MI_U32 videoParam[3];
        memcpy(videoParam, param, paramLen);
        veChn       = videoParam[0];
        size.width  = videoParam[1];
        size.height = videoParam[2];

        if (NULL == g_videoEncoderArray[veChn])
        {
            CARDV_ERR("venc channel [%d] error\n", veChn);
            break;
        }

        g_videoEncoderArray[veChn]->setResolution(size.width, size.height);
    }
    break;

    case CMD_VIDEO_SET_FRAMERATE: {
        MI_S32 videoParam[2];
        U32    frameRate = 0;

        memcpy(videoParam, param, sizeof(videoParam));
        frameRate = videoParam[1];

        if (videoParam[0] >= MAX_VIDEO_NUMBER || NULL == g_videoEncoderArray[videoParam[0]])
        {
            CARDV_ERR("video channel [%d] error\n", videoParam[0]);
            break;
        }
        g_videoEncoderArray[videoParam[0]]->setFrameRate(frameRate, 1);
    }
    break;

    case CMD_VIDEO_SET_BITRATE: {
        MI_U32 veChn;
        MI_U32 bitrate;
        MI_U32 videoParam[2];
        memcpy(videoParam, param, sizeof(videoParam));
        veChn   = videoParam[0];
        bitrate = videoParam[1];
        printf("veChn[%d] bitrate[%d]. \n", veChn, bitrate);
        if (NULL == g_videoEncoderArray[veChn])
        {
            CARDV_ERR("venc channel [%d] error\n", veChn);
            break;
        }

        g_videoEncoderArray[veChn]->setBitrate(bitrate);
    }
    break;

    case CMD_VIDEO_SET_CODEC: {
        MI_U32 videoParam[2];
        memcpy(videoParam, param, paramLen);
        MI_S8 videoIndex = videoParam[0];
        MI_S8 videoCodec = videoParam[1];
        if (videoIndex < MAX_VIDEO_NUMBER && g_videoEncoderArray[videoIndex])
        {
            printf("change venc [%d] codec [%d]\n", videoIndex, videoCodec);
            g_videoEncoderArray[videoIndex]->uninitVideoEncoder();
            g_videoEncoderArray[videoIndex]->setCodec(videoCodec);
            g_videoEncoderArray[videoIndex]->initVideoEncoder();
        }
    }
    break;

    default:
        break;
    }

    return 0;
}