#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "mi_sys.h"
#include "sstardisp.h"
#if !defined(CHIP_IFORD) && !defined(CHIP_IFADO)
#include "mi_panel_datatype.h"
#include "mi_panel.h"
#endif
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "st_common.h"

/////////////////check////////////////////////
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include "mi_fb.h"

/////////////////////////////////////////////



#define UI_1024_600     0
#define UI_864_480      1
#define USE_MIPI        0

#if (DISP_INIT)
#if UI_1024_600
#include "SAT070CP50_1024x600.h"
#else
    #if USE_MIPI
    #include "ST7701S_480x480_MIPI.h"
    #else
    //#include "SAT070AT50_800x480.h"
    #include "LX50FWB4001_RM68172_480x854_v2.h" //UI_864_480
    #endif
#endif
#endif

// loacl play res
#if UI_1024_600
//#define LOCAL_VIDEO_W  822
//#define LOCAL_VIDEO_H  464
//#define LOCAL_VIDEO_X  100
//#define LOCAL_VIDEO_Y  60
#define LOCAL_VIDEO_W  1024
#define LOCAL_VIDEO_H  600
#define LOCAL_VIDEO_X  0
#define LOCAL_VIDEO_Y  0
#else
//#define LOCAL_VIDEO_W  640
//#define LOCAL_VIDEO_H  360
//#define LOCAL_VIDEO_X  100
//#define LOCAL_VIDEO_Y  60
#define LOCAL_VIDEO_W 800
#define LOCAL_VIDEO_H  480
#define LOCAL_VIDEO_X  0
#define LOCAL_VIDEO_Y  0
#endif

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif

static MI_S32 g_fbFd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

/**
 *dump fix info of Framebuffer
 */
void printFixedInfo ()
{
    printf ("Fixed screen info:\n"
            "\tid: %s\n"
            "\tsmem_start: 0x%lx\n"
            "\tsmem_len: %d\n"
            "\ttype: %d\n"
            "\ttype_aux: %d\n"
            "\tvisual: %d\n"
            "\txpanstep: %d\n"
            "\typanstep: %d\n"
            "\tywrapstep: %d\n"
            "\tline_length: %d\n"
            "\tmmio_start: 0x%lx\n"
            "\tmmio_len: %d\n"
            "\taccel: %d\n"
            "\n",
            finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
            finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
            finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
            finfo.mmio_len, finfo.accel);
}

/**
 *dump var info of Framebuffer
 */
void printVariableInfo ()
{
    printf ("Variable screen info:\n"
            "\txres: %d\n"
            "\tyres: %d\n"
            "\txres_virtual: %d\n"
            "\tyres_virtual: %d\n"
            "\tyoffset: %d\n"
            "\txoffset: %d\n"
            "\tbits_per_pixel: %d\n"
            "\tgrayscale: %d\n"
            "\tred: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tgreen: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tblue: offset: %2d, length: %2d, msb_right: %2d\n"
            "\ttransp: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tnonstd: %d\n"
            "\tactivate: %d\n"
            "\theight: %d\n"
            "\twidth: %d\n"
            "\taccel_flags: 0x%x\n"
            "\tpixclock: %d\n"
            "\tleft_margin: %d\n"
            "\tright_margin: %d\n"
            "\tupper_margin: %d\n"
            "\tlower_margin: %d\n"
            "\thsync_len: %d\n"
            "\tvsync_len: %d\n"
            "\tsync: %d\n"
            "\tvmode: %d\n"
            "\n",
            vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
            vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel,
            vinfo.grayscale, vinfo.red.offset, vinfo.red.length,
            vinfo.red.msb_right, vinfo.green.offset, vinfo.green.length,
            vinfo.green.msb_right, vinfo.blue.offset, vinfo.blue.length,
            vinfo.blue.msb_right, vinfo.transp.offset, vinfo.transp.length,
            vinfo.transp.msb_right, vinfo.nonstd, vinfo.activate,
            vinfo.height, vinfo.width, vinfo.accel_flags, vinfo.pixclock,
            vinfo.left_margin, vinfo.right_margin, vinfo.upper_margin,
            vinfo.lower_margin, vinfo.hsync_len, vinfo.vsync_len,
            vinfo.sync, vinfo.vmode);
}

MI_S32 ST_Fb_Init(void)
{
    const char *devfile = "/dev/fb0";
    MI_BOOL bShown;
    char *frameBuffer = NULL;
    MI_PHY frameBuffer_PHY = 0;
    MI_S32 s32BuffSize = 0;

    /* Open the file for reading and writing */
    g_fbFd = open (devfile, O_RDWR);
    if (g_fbFd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(0);
    }

    //get fb_fix_screeninfo
    if (ioctl(g_fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(0);
    }
    //printFixedInfo();

    //get fb_var_screeninfo
    if (ioctl(g_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(0);
    }
    //printVariableInfo();

    /* Figure out the size of the screen in bytes */
    s32BuffSize = finfo.smem_len;

    /* Map the device to memory */
    frameBuffer = (char *) mmap(0, s32BuffSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        exit(0);
    }

#ifdef DUAL_OS
    memset(frameBuffer, 0x00, s32BuffSize);
#else
    // clear framebuffer
    //drawRect(0, 0, vinfo.xres, vinfo.yres, 0xff0000ff);
    MI_SYS_Va2Pa(frameBuffer, &frameBuffer_PHY);
    MI_SYS_MemsetPa(0, frameBuffer_PHY, 0X00, s32BuffSize);
    
    //set colorkey
    MI_FB_ColorKey_t colorKeyInfo;
    MI_U32 u32ColorKeyVal = 0X00000000;
    colorKeyInfo.bKeyEnable = TRUE;
    colorKeyInfo.u8Red = PIXEL8888RED(u32ColorKeyVal);
    colorKeyInfo.u8Green = PIXEL8888GREEN(u32ColorKeyVal);
    colorKeyInfo.u8Blue = PIXEL8888BLUE(u32ColorKeyVal);

    if (ioctl(g_fbFd, FBIOSET_COLORKEY, &colorKeyInfo) < 0) {
        perror("Error: failed to FBIOGET_COLORKEY");
        //exit(0);
    }

    #if 0
    if (ioctl(g_fbFd, FBIOPAN_DISPLAY, &vinfo) < 0)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(0);
    }
    #endif

    #if 1
    if (ioctl(g_fbFd, FBIOGET_SHOW, &bShown) < 0) {
        perror("Error: failed to FBIOGET_SHOW");
        exit(0);
    }
    //test FBIOSET_SHOW
    bShown  = TRUE;
    if (ioctl(g_fbFd, FBIOSET_SHOW, &bShown) < 0) {
        perror("Error: failed to FBIOSET_SHOW");
        exit(0);
    }
    #endif

    #if 0//set alpha
    MI_FB_GlobalAlpha_t alphaInfo;
    alphaInfo.bAlphaEnable = TRUE;
    alphaInfo.bAlphaChannel= TRUE;
    alphaInfo.u8GlobalAlpha = 0xf0;
    if (ioctl(g_fbFd, FBIOSET_GLOBAL_ALPHA, &alphaInfo) < 0) {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
        //exit(12);
    }
    #endif
#endif

    close(g_fbFd);
    return MI_SUCCESS;
}

// MI_S32 ST_Fb_RgnInit(void)
// {
//     if (ioctl(g_fbFd, FBIOSET_RGN_INIT, NULL) == -1)
//     {
//         perror("Error reading fixed information");
//         exit(0);
//     }

//     return MI_SUCCESS;
// }


#if (DISP_INIT)
int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    MI_SYS_Init();

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        pstDispPubAttr->stSyncInfo.u16Vact = stPanelParam.u16Height;
        pstDispPubAttr->stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
                                                                      stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Hact = stPanelParam.u16Width;
        pstDispPubAttr->stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
                                                                      stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Bvact = 0;
        pstDispPubAttr->stSyncInfo.u16Bvbb = 0;
        pstDispPubAttr->stSyncInfo.u16Bvfb = 0;
        pstDispPubAttr->stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
        pstDispPubAttr->stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
        pstDispPubAttr->stSyncInfo.u32FrameRate = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_USER;
        pstDispPubAttr->eIntfType = E_MI_DISP_INTF_LCD;
        #if USE_MIPI
        eLinkType = E_MI_PNL_LINK_MIPI_DSI;
        #else
        eLinkType = E_MI_PNL_LINK_TTL;
        #endif
    }

    stInputPortAttr.u16SrcWidth = LOCAL_VIDEO_W;
    stInputPortAttr.u16SrcHeight = LOCAL_VIDEO_H;
    stInputPortAttr.stDispWin.u16X = LOCAL_VIDEO_X;
    stInputPortAttr.stDispWin.u16Y = LOCAL_VIDEO_Y;
    stInputPortAttr.stDispWin.u16Width = LOCAL_VIDEO_W;
    stInputPortAttr.stDispWin.u16Height = LOCAL_VIDEO_H;

    MI_DISP_SetPubAttr(0, pstDispPubAttr);
    MI_DISP_Enable(0);
    MI_DISP_BindVideoLayer(0, 0);
    MI_DISP_EnableVideoLayer(0);

    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        #if defined(UI_864_480)&&(UI_864_480)
        init_panel_spi_cmd();
        #endif
        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);

        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            #if USE_MIPI
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
            #endif
        }
    }
    return 0;
}
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr)
{

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);

    switch(pstDispPubAttr->eIntfType) {
        case E_MI_DISP_INTF_HDMI:
            break;

        case E_MI_DISP_INTF_VGA:
            break;

        case E_MI_DISP_INTF_LCD:
        default:
            MI_PANEL_DeInit();

    }

    MI_SYS_Exit();
    printf("TODO:sstar_disp_Deinit...\n");

    return 0;
}
#endif

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

