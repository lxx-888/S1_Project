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
#ifndef __MI_RGN_DATATYPE_
#define __MI_RGN_DATATYPE_

#include "mi_common_datatype.h"

#define MI_RGN_MAX_HANDLE            1024
#define MI_RGN_HANDLE_NULL           -1
#define MI_RGN_MAX_PALETTE_TABLE_NUM 256
#define MI_RGN_I2_PALETTE_INDEX_MAX  63
#define MI_RGN_I4_PALETTE_INDEX_MAX  15
#define MI_RGN_POLY_VERTEX_MIN       3
#define MI_RGN_POLY_VERTEX_MAX       6

typedef enum
{
    E_MI_RGN_TYPE_OSD = 0,
    E_MI_RGN_TYPE_COVER,
    E_MI_RGN_TYPE_FRAME,
    E_MI_RGN_TYPE_MAX
} MI_RGN_Type_e;

typedef enum
{
    E_MI_RGN_PIXEL_FORMAT_ARGB1555 = 0,
    E_MI_RGN_PIXEL_FORMAT_ARGB4444,
    E_MI_RGN_PIXEL_FORMAT_I2,
    E_MI_RGN_PIXEL_FORMAT_I4,
    E_MI_RGN_PIXEL_FORMAT_I8,
    E_MI_RGN_PIXEL_FORMAT_RGB565,
    E_MI_RGN_PIXEL_FORMAT_ARGB8888,
    E_MI_RGN_PIXEL_FORMAT_MAX
} MI_RGN_PixelFormat_e;

typedef enum
{
    E_MI_RGN_BLOCK_SIZE_4 = 0,
    E_MI_RGN_BLOCK_SIZE_8,
    E_MI_RGN_BLOCK_SIZE_16,
    E_MI_RGN_BLOCK_SIZE_32,
    E_MI_RGN_BLOCK_SIZE_64,
    E_MI_RGN_BLOCK_SIZE_128,
    E_MI_RGN_BLOCK_SIZE_256,
    E_MI_RGN_BLOCK_SIZE_MAX
} MI_RGN_BlockSize_e;

typedef enum
{
    E_MI_RGN_COVER_MODE_COLOR = 0,
    E_MI_RGN_COVER_MODE_MOSAIC,
    E_MI_RGN_COVER_MODE_MAX
} MI_RGN_CoverMode_e;

typedef enum
{
    E_MI_RGN_AREA_TYPE_RECT = 0,
    E_MI_RGN_AREA_TYPE_POLY,
    E_MI_RGN_AREA_TYPE_MAX
} MI_RGN_AreaType_e;

typedef enum
{
    E_MI_RGN_COLOR_INVERT_WORK_MODE_AUTO = 0,
    E_MI_RGN_COLOR_INVERT_WORK_MODE_MANUAL,
    E_MI_RGN_COLOR_INVERT_WORK_MODE_MAX
} MI_RGN_ColorInvertWorkMode_e;

typedef struct MI_RGN_ChnPort_s
{
    MI_ModuleId_e eModId;
    MI_S32        s32DevId;
    MI_S32        s32ChnId;
    MI_S32        s32PortId;
    MI_BOOL       bInputPort;
} MI_RGN_ChnPort_t;

typedef struct MI_RGN_Size_s
{
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_RGN_Size_t;

typedef struct MI_RGN_Bitmap_s
{
    MI_RGN_PixelFormat_e ePixelFormat;
    MI_RGN_Size_t        stSize;
    union
    {
        MI_PTR   pData;
        MI_PTR64 _Reserved;
    };
} MI_RGN_Bitmap_t;

typedef struct MI_RGN_OsdInitParam_s
{
    MI_RGN_PixelFormat_e ePixelFmt;
    MI_RGN_Size_t        stSize;
    MI_U16               u16MaxCanvasNum;
} MI_RGN_OsdInitParam_t;

typedef struct MI_RGN_Point_s
{
    MI_U32 u32X;
    MI_U32 u32Y;
} MI_RGN_Point_t;

typedef enum
{
    E_MI_RGN_PIXEL_ALPHA = 0,
    E_MI_RGN_CONSTANT_ALPHA
} MI_RGN_AlphaMode_e;

typedef struct MI_RGN_OsdArgb1555Alpha_s
{
    MI_U8 u8BgAlpha;
    MI_U8 u8FgAlpha;
} MI_RGN_OsdArgb1555Alpha_t;
typedef union
{
    MI_RGN_OsdArgb1555Alpha_t stArgb1555Alpha;
    MI_U8                     u8ConstantAlpha;
} MI_RGN_AlphaModePara_u; /* NOLINT */

typedef struct MI_RGN_OsdAlphaAttr_s
{
    MI_RGN_AlphaMode_e     eAlphaMode;
    MI_RGN_AlphaModePara_u stAlphaPara;
} MI_RGN_OsdAlphaAttr_t;

typedef struct MI_RGN_Rect_s
{
    MI_S32 s32X;
    MI_S32 s32Y;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_RGN_Rect_t;

typedef struct MI_RGN_Poly_s
{
    MI_U8          u8VertexNum;
    MI_RGN_Point_t astCoord[MI_RGN_POLY_VERTEX_MAX];
} MI_RGN_Poly_t;

typedef struct MI_RGN_CoverColorAttr_s
{
    MI_U32 u32Color;
} MI_RGN_CoverColorAttr_t;

typedef struct MI_RGN_CoverMosaicAttr_s
{
    MI_RGN_BlockSize_e eBlkSize;
} MI_RGN_CoverMosaicAttr_t;

typedef struct MI_RGN_CoverChnPortParam_s
{
    MI_RGN_AreaType_e eAreaType;
    union
    {
        MI_RGN_Rect_t stRect;
        MI_RGN_Poly_t stPoly;
    };
    MI_RGN_CoverMode_e eMode;
    union
    {
        MI_RGN_CoverColorAttr_t  stColorAttr;
        MI_RGN_CoverMosaicAttr_t stMosaicAttr;
    };
} MI_RGN_CoverChnPortParam_t;

typedef struct MI_RGN_FrameChnPortParam_s
{
    MI_RGN_Rect_t stRect;
    MI_U32        u32Color;
    MI_U8         u8Thickness;
} MI_RGN_FrameChnPortParam_t;

typedef struct MI_RGN_OsdChnPortParam_s
{
    MI_RGN_Point_t        stPoint;
    MI_RGN_OsdAlphaAttr_t stOsdAlphaAttr;
    MI_U8                 u8PaletteIdx;
} MI_RGN_OsdChnPortParam_t;

typedef struct MI_RGN_LumaInfo_s
{
    MI_PHY        phyAddr;
    MI_VIRT       virtAddr;
    MI_RGN_Size_t stSize;
    MI_U32        u32Stride;
} MI_RGN_LumaInfo_t;

typedef struct MI_RGN_ColorInvertAttr_s
{
    MI_BOOL                      bEnable;
    MI_BOOL                      bApplyMap;
    MI_RGN_ColorInvertWorkMode_e eWorkMode;
    MI_U8                        u8ThresholdLow;
    MI_U8                        u8ThresholdHigh;
    MI_RGN_BlockSize_e           eBlkSizeHori;
    MI_RGN_BlockSize_e           eBlkSizeVert;
} MI_RGN_ColorInvertAttr_t;

typedef struct MI_RGN_Attr_s
{
    MI_RGN_Type_e         eType;
    MI_RGN_OsdInitParam_t stOsdInitParam;
} MI_RGN_Attr_t;

typedef struct MI_RGN_ChnPortParam_s
{
    MI_BOOL bShow;
    MI_U32  u32Layer;
    union
    {
        MI_RGN_CoverChnPortParam_t stCoverChnPort;
        MI_RGN_OsdChnPortParam_t   stOsdChnPort;
        MI_RGN_FrameChnPortParam_t stFrameChnPort;
    };
} MI_RGN_ChnPortParam_t;

typedef struct MI_RGN_CanvasInfo_s
{
    MI_PHY phyAddr;
    union
    {
        MI_VIRT  virtAddr;
        MI_PTR64 _Reserved;
    };
    MI_RGN_Size_t        stSize;
    MI_U32               u32Stride;
    MI_RGN_PixelFormat_e ePixelFmt;
} MI_RGN_CanvasInfo_t;

typedef struct MI_RGN_PaletteElement_s
{
    MI_U8 u8Alpha;
    MI_U8 u8Red;
    MI_U8 u8Green;
    MI_U8 u8Blue;
} MI_RGN_PaletteElement_t;

typedef struct MI_RGN_PaletteTable_s
{
    MI_RGN_PaletteElement_t astElement[MI_RGN_MAX_PALETTE_TABLE_NUM];
} MI_RGN_PaletteTable_t;

#define MI_RGN_OK MI_SUCCESS

/* PingPong buffer change when set attr, it needs to remap memory in mpi interface */
#define MI_NOTICE_RGN_BUFFER_CHANGE MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_INFO, MI_SUCCESS)

/* invlalid handle */
#define MI_ERR_RGN_INVALID_HANDLE MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
/* invlalid device ID */
#define MI_ERR_RGN_INVALID_DEVID MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define MI_ERR_RGN_INVALID_CHNID MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define MI_ERR_RGN_ILLEGAL_PARAM MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
/* channel exists */
#define MI_ERR_RGN_EXIST MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_EXIST)
/*UN exist*/
#define MI_ERR_RGN_UNEXIST MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_UNEXIST)
/* using a NULL point */
#define MI_ERR_RGN_NULL_PTR MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define MI_ERR_RGN_NOT_CONFIG MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define MI_ERR_RGN_NOT_SUPPORT MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define MI_ERR_RGN_NOT_PERM MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define MI_ERR_RGN_NOMEM MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
/* failure caused by malloc buffer */
#define MI_ERR_RGN_NOBUF MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOBUF)
/* no data in buffer */
#define MI_ERR_RGN_BUF_EMPTY MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY)
/* no buffer for new data */
#define MI_ERR_RGN_BUF_FULL MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define MI_ERR_RGN_BADADDR MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BADADDR)
/* resource is busy, eg. destroy a venc chn without unregistering it */
#define MI_ERR_RGN_BUSY MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)

/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define MI_ERR_RGN_NOTREADY MI_DEF_ERR(E_MI_MODULE_ID_RGN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY)

#endif /* End of #ifndef __MI_RGN_DATATYPE_ */
