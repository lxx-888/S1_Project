#ifndef __SSTARDISP__H__
#define __SSTARDISP__H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <mi_disp_datatype.h>
#ifdef MI_HDMI
#include "mi_hdmi_datatype.h"
#include "mi_hdmi.h"

typedef struct stTimingArray_s
{
    char desc[50];
    MI_DISP_OutputTiming_e eOutputTiming;
    MI_HDMI_TimingType_e eHdmiTiming;
    MI_U16 u16Width;
    MI_U16 u16Height;
}stTimingArray_t;
#endif

#define DIVP_CHN_FOR_DISP_SCALE      1
#define DIVP_CHN_FOR_DISP_ROTATION   2

#define DIVP_ROT_INPUT_ALIGN    64
#define DIVP_ROT_OUTPUT_ALIGN   16//32

#ifndef CARDVCHECKRESULT
#define CARDVCHECKRESULT(_func_)\
    do{ \
        MI_S32 s32Ret = MI_SUCCESS; \
        s32Ret = _func_; \
        if (s32Ret != MI_SUCCESS)\
        { \
            printf("(%s %d)exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            /*return s32Ret; */ \
        } \
        else \
        { \
            /* printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__); */ \
        } \
    } while(0)
#endif

#define PIXEL8888ALPHA(pixelval)	(((pixelval) >> 24) & 0xff)
#define PIXEL8888RED(pixelval)  	(((pixelval) >> 16) & 0xff)
#define PIXEL8888GREEN(pixelval)	(((pixelval) >> 8) & 0xff)
#define PIXEL8888BLUE(pixelval) 	((pixelval) & 0xff)

#define ARGB888_BLACK   ARGB2PIXEL8888(128,0,0,0)
#define ARGB888_RED     ARGB2PIXEL8888(128,255,0,0)
#define ARGB888_GREEN   ARGB2PIXEL8888(128,0,255,0)
#define ARGB888_BLUE    ARGB2PIXEL8888(128,0,0,255)

int sstar_disp_init(MI_DISP_PubAttr_t* pstDispPubAttr);
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr);
MI_S32 CarDVUI_DispInit(void);
MI_S32 ST_Fb_Init(void);
MI_S32 ST_Fb_RgnInit(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SSTARDISP__H__
