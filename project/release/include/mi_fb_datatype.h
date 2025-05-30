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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_fb_datatype.h
/// @brief  Sigmastar graphic Interface header file
/// @author Sigmastar Semiconductor Inc.
/// @attention
/// <b><em></em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _UAPI_SSTAR_FB_GRAPHIC_H
#define _UAPI_SSTAR_FB_GRAPHIC_H

#include "mi_common_datatype.h"

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    // E_DRV_FB_GOP_COLOR_RGB565
    E_MI_FB_COLOR_FMT_RGB565 = 1,
    // E_DRV_FB_GOP_COLOR_ARGB4444
    E_MI_FB_COLOR_FMT_ARGB4444 = 2,
    // E_DRV_FB_GOP_COLOR_ARGB8888
    E_MI_FB_COLOR_FMT_ARGB8888 = 5,
    // E_DRV_FB_GOP_COLOR_ARGB1555
    E_MI_FB_COLOR_FMT_ARGB1555 = 6,
    // E_DRV_FB_GOP_COLOR_YUV422
    E_MI_FB_COLOR_FMT_YUV422 = 9,
    // E_DRV_FB_GOP_COLOR_I8
    E_MI_FB_COLOR_FMT_I8 = 4,
    // E_DRV_FB_GOP_COLOR_I4
    E_MI_FB_COLOR_FMT_I4 = 13,
    // E_DRV_FB_GOP_COLOR_I2
    E_MI_FB_COLOR_FMT_I2 = 14,
    // E_DRV_FB_GOP_COLOR_INVALID
    E_MI_FB_COLOR_FMT_INVALID = 12,
} MI_FB_ColorFmt_e;

typedef enum
{
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS          = 0x1,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE         = 0x2,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE       = 0x4,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_SCREEN_SIZE       = 0x8,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_PREMUL            = 0x10,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB         = 0x20,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_OUTPUT_COLORSPACE = 0x40,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_DST_DISP          = 0x80,
} MI_FB_DisplayLayerAttrMaskbit_e;

typedef enum
{
    E_MI_FB_ALPHA_CHANNEL       = 0x01, /* Enable alpha chanel, 0: disable(global_alpha), 1: enable(pixel_alpha) */
    E_MI_FB_ALPHA_INVERT        = 0x02, /* Enable alpha invert, 0: disable, 1: enable */
    E_MI_FB_ALPHA_PREMULTIPLIED = 0x04, /* Enable pixel pre-multiplied alpha, 0: disable, 1: enable */
    E_MI_FB_ALPHA_PLANE_ALPHA   = 0x08, /* Enable plane alpha, 0: disable, 1: enable */
} MI_FB_AlphaMode_e;
typedef struct MI_FB_GlobalAlpha_s
{
    MI_U16 u8AlphaMode;
    MI_U16 u16PlaneAlpha;
    MI_U8  u8Alpha0;
    MI_U8  u8Alpha1;
    MI_U8  u8GlobalAlpha;
} MI_FB_GlobalAlpha_t;

typedef struct MI_FB_ColorKey_s
{
    MI_BOOL bKeyEnable;
    MI_U8   u8Red;
    MI_U8   u8Green;
    MI_U8   u8Blue;
} MI_FB_ColorKey_t;

typedef struct MI_FB_Rectangle_s
{
    MI_U16 u16Xpos;
    MI_U16 u16Ypos;
    MI_U16 u16Width;
    MI_U16 u16Height;
} MI_FB_Rectangle_t;

typedef enum
{
    // DRV_FB_GOPOUT_RGB
    E_MI_FB_OUTPUT_RGB = 0,
    // DRV_FB_GOPOUT_YUV
    E_MI_FB_OUTPUT_YUV = 1,
} MI_FB_OutputColorSpace_e;

typedef enum
{
    // E_DRV_FB_GOP_DST_IP0
    E_MI_FB_DST_IP0 = 0,
    // E_DRV_FB_GOP_DST_IP0_SUB
    E_MI_FB_DST_IP0_SUB = 1,
    // E_DRV_FB_GOP_DST_MIXER2VE
    E_MI_FB_DST_MIXER2VE = 2,
    // E_DRV_FB_GOP_DST_OP0
    E_MI_FB_DST_OP0 = 3,
    // E_DRV_FB_GOP_DST_VOP
    E_MI_FB_DST_VOP = 4,
    // E_DRV_FB_GOP_DST_IP1
    E_MI_FB_DST_IP1 = 5,
    // E_DRV_FB_GOP_DST_IP1_SUB
    E_MI_FB_DST_IP1_SUB = 6,
    // E_DRV_FB_GOP_DST_MIXER2OP
    E_MI_FB_DST_MIXER2OP = 7,
    // E_DRV_FB_GOP_DST_VOP_SUB
    E_MI_FB_DST_VOP_SUB = 8,
    // E_DRV_FB_GOP_DST_FRC
    E_MI_FB_DST_FRC = 9,
    // E_DRV_FB_GOP_DST_VE
    E_MI_FB_DST_VE = 10,
    // E_DRV_FB_GOP_DST_BYPASS
    E_MI_FB_DST_BYPASS = 11,
    // E_DRV_FB_GOP_DST_OP1
    E_MI_FB_DST_OP1 = 12,
    // E_DRV_FB_GOP_DST_MIXER2OP1
    E_MI_FB_DST_MIXER2OP1 = 13,
    // E_DRV_FB_GOP_DST_DIP
    E_MI_FB_DST_DIP = 14,
    // E_DRV_FB_GOP_DST_GOPScaling
    E_MI_FB_DST_GOPScaling = 15, /* NOLINT */
    // E_DRV_FB_GOP_DST_OP_DUAL_RATE
    E_MI_FB_DST_OP_DUAL_RATE = 16,
    // E_DRV_FB_GOP_DST_INVALID
    E_MI_FB_DST_INVALID = 17,
} MI_FB_DstDisplayplane_e;

typedef struct MI_FB_DisplayLayerAttr_s
{
    MI_U32           u32Xpos;          /**the x pos of orign point in screen.Meaning for stretchwindow posx*/
    MI_U32           u32YPos;          /**the y pos of orign point in screen.Meaning for stretchwindow posy*/
    MI_U32           u32dstWidth;      /**display buffer dest with in screen.Meaning for stretch window dst width*/
    MI_U32           u32dstHeight;     /**display buffer dest hight in screen.Meaning for stretch window dst height*/
    MI_U32           u32DisplayWidth;  /**the width of display buf in fb.Meaning for OSD resolution width */
    MI_U32           u32DisplayHeight; /**the height of display buf in fb.Meaning for OSD resolution height*/
    MI_U32           u32ScreenWidth;   /**the width of screen.Meaning for timing width.Meaning for timing height*/
    MI_U32           u32ScreenHeight;  /** the height of screen */
    MI_BOOL          bPreMul;          /**the data drawed in buffer whether is premultiply alpha or not*/
    MI_FB_ColorFmt_e eFbColorFmt;      /**the color format of framebuffer*/
    MI_FB_OutputColorSpace_e eFbOutputColorSpace; /**output color space*/
    MI_FB_DstDisplayplane_e  eFbDestDisplayPlane; /**destination displayplane*/
    MI_U32                   u32SetAttrMask;      /** display attribute modify mask*/
} MI_FB_DisplayLayerAttr_t;

typedef struct MI_FB_CursorImage_s
{
    MI_U32           u32Width;  /**width, unit pixel*/
    MI_U32           u32Height; /**Height, unit pixel*/
    MI_U32           u32Pitch;  /**Pitch, unit pixel*/
    MI_FB_ColorFmt_e eColorFmt; /**Color format*/
#ifndef __KERNEL__
    /**Image raw data*/
    union
    {
        const char *data;
        MI_PTR64    _p64Reserved;
    };
#else
    /**Image raw data*/
    union
    {
        const char __user *data;
        MI_PTR64           _p64Reserved;
    };
#endif
} MI_FB_CursorImage_t;

typedef enum
{
    E_MI_FB_CURSOR_ATTR_MASK_ICON     = 0x1,
    E_MI_FB_CURSOR_ATTR_MASK_POS      = 0x2,
    E_MI_FB_CURSOR_ATTR_MASK_ALPHA    = 0x4,
    E_MI_FB_CURSOR_ATTR_MASK_SHOW     = 0x8,
    E_MI_FB_CURSOR_ATTR_MASK_HIDE     = 0x10,
    E_MI_FB_CURSOR_ATTR_MASK_COLORKEY = 0x20,
    E_MI_FB_CURSOR_ATTR_MASK          = 0x3F
} MI_FB_CursorAttrMaskbit_e;

typedef struct MI_FB_CursorAttr_s
{
    MI_U32              u32XPos;
    MI_U32              u32YPos;
    MI_U32              u32HotSpotX;
    MI_U32              u32HotSpotY;
    MI_FB_GlobalAlpha_t stAlpha;
    MI_FB_ColorKey_t    stColorKey;
    MI_BOOL             bShown;
    MI_FB_CursorImage_t stCursorImageInfo;
    MI_U16              u16CursorAttrMask;
} MI_FB_CursorAttr_t;

typedef struct
{
    MI_U32 u32Offset;
    MI_U32 u32Length;
    MI_U32 u32MsbRight;
} MI_FB_BitField_t;

typedef struct
{
    MI_U32 u32Xres; /* visible resolution */
    MI_U32 u32Yres;
    MI_U32 u32Xres_virtual; /* virtual resolution */
    MI_U32 u32Yres_virtual;
    MI_U32 u32Xoffset; /* offset from virtual to visible */
    MI_U32 u32Yoffset; /* resolution */

    MI_U32 u32BitsPerPixel; /* guess what */
    MI_U32 u32Grayscale;    /* != 0 Graylevels instead of colors */

    MI_FB_BitField_t stBitFieldRed;   /* bitfield in fb mem if true color, */
    MI_FB_BitField_t stBitFieldGreen; /* else only length is significant */
    MI_FB_BitField_t stBitFieldBlue;
    MI_FB_BitField_t stBitFieldTransp; /* transparency */

    MI_U32 u32Nonstd;      /* != 0 Non standard pixel format */
    MI_U32 u32Rotate;      /* angle we rotate counter clockwise */
    MI_U32 u32Reserved[5]; /* Reserved for future compatibility */
} MI_FB_VarScreenInfo_t;

typedef struct
{
    MI_U8  u8Id[16];       /* identification string eg "TT Builtin" */
    MI_PHY phySmemStart;   /* Start of frame buffer mem (physical address) */
    MI_U32 u32SmemLen;     /* Length of frame buffer mem */
    MI_U32 u32Type;        /* see FB_TYPE_* */
    MI_U32 u32TypeAux;     /* Interleave for interleaved Planes */
    MI_U32 u32Visual;      /* see FB_VISUAL_* */
    MI_U16 u16XpanStep;    /* zero if no hardware panning */
    MI_U16 u16YpanStep;    /* zero if no hardware panning */
    MI_U16 u16YwrapStep;   /* zero if no hardware ywrap */
    MI_U32 u32LineLength;  /* length of a line in bytes */
    MI_PHY phyMmioStart;   /* Start of Memory Mapped I/O (physical address) */
    MI_U32 u32MmioLen;     /* Length of Memory Mapped I/O */
    MI_U32 u32Accel;       /* Indicate to driver which specific chip/card we have */
    MI_U16 u16Reserved[3]; /* Reserved for future compatibility */
} MI_FB_FixScreenInfo_t;

typedef struct
{
    MI_U32 u32Start;
    MI_U32 u32Len;
    union
    {
        MI_U16 * pu16Red;
        MI_PTR64 _pu64ReservedRed;
    };
    union
    {
        MI_U16 * pu16Green;
        MI_PTR64 _pu64ReservedGreen;
    };
    union
    {
        MI_U16 * pu16Blue;
        MI_PTR64 _pu64ReservedBlue;
    };
    union
    {
        MI_U16 * pu16Transp;
        MI_PTR64 _pu64ReservedTransp;
    };
} MI_FB_Cmap_t;

typedef struct
{
    MI_BOOL          bEnable;
    MI_BOOL          bBlockSplit;
    MI_BOOL          bColorTransform;
    MI_FB_ColorFmt_e eColorFmt;
    MI_U16           u16Width;
    MI_U16           u16Height;
} MI_FB_Compression_t;

#ifndef __RTK_OS__
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define FB_IOC_MAGIC 'F'

#define FBIOGET_SCREEN_LOCATION _IOR(FB_IOC_MAGIC, 0x60, MI_FB_Rectangle_t)
#define FBIOSET_SCREEN_LOCATION _IOW(FB_IOC_MAGIC, 0x61, MI_FB_Rectangle_t)

#define FBIOGET_SHOW _IOR(FB_IOC_MAGIC, 0x62, MI_BOOL)
#define FBIOSET_SHOW _IOW(FB_IOC_MAGIC, 0x63, MI_BOOL)

#define FBIOGET_GLOBAL_ALPHA _IOR(FB_IOC_MAGIC, 0x64, MI_FB_GlobalAlpha_t)
#define FBIOSET_GLOBAL_ALPHA _IOW(FB_IOC_MAGIC, 0x65, MI_FB_GlobalAlpha_t)

#define FBIOGET_COLORKEY _IOR(FB_IOC_MAGIC, 0x66, MI_FB_ColorKey_t)
#define FBIOSET_COLORKEY _IOW(FB_IOC_MAGIC, 0x67, MI_FB_ColorKey_t)

#define FBIOGET_DISPLAYLAYER_ATTRIBUTES _IOR(FB_IOC_MAGIC, 0x68, MI_FB_DisplayLayerAttr_t)
#define FBIOSET_DISPLAYLAYER_ATTRIBUTES _IOW(FB_IOC_MAGIC, 0x69, MI_FB_DisplayLayerAttr_t)

#define FBIOGET_CURSOR_ATTRIBUTE _IOR(FB_IOC_MAGIC, 0x70, MI_FB_CursorAttr_t)
#define FBIOSET_CURSOR_ATTRIBUTE _IOW(FB_IOC_MAGIC, 0x71, MI_FB_CursorAttr_t)

#define FBIOGET_COMPRESSIONINFO _IOR(FB_IOC_MAGIC, 0x72, MI_FB_Compression_t)
#define FBIOSET_COMPRESSIONINFO _IOW(FB_IOC_MAGIC, 0x73, MI_FB_Compression_t)

#endif

#define MI_ERR_FB_INVALID_DEVID MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_FB_ILLEGAL_PARAM MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_FB_EXIST         MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_EXIST)
#define MI_ERR_FB_UNEXIST       MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_UNEXIST)
#define MI_ERR_FB_NULL_PTR      MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
#define MI_ERR_FB_NOT_CONFIG    MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG)
#define MI_ERR_FB_NOT_SUPPORT   MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_FB_NOMEM         MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
#define MI_ERR_FB_BADADDR       MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BADADDR)
#define MI_ERR_FB_BUSY          MI_DEF_ERR(E_MI_MODULE_ID_FB, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)

#endif
