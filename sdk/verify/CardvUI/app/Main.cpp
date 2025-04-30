#include "entry/EasyUIContext.h"
#include <signal.h>
#include <stdlib.h>
#include "sstardisp.h"
#include "mi_sys.h"
#include <string.h>
#include <execinfo.h>

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)

#define GEN_BT_MSG2UART

#ifndef CHIP_I6E
static MI_DISP_PubAttr_t stDispPubAttr;
#ifdef DUAL_OS
#include "mi_panel_datatype.h"
#include "mi_hdmi_datatype.h"
#include "mi_disp.h"
#include "mi_panel.h"

#define DISP_DEV_MAX 2
#define DISP_LAYER_MAX 2
typedef struct stDispUtDev_s
{
    MI_BOOL bDevEnable;
    MI_BOOL bPanelInit;
    MI_BOOL bDevBindLayer[DISP_LAYER_MAX];
    MI_DISP_PubAttr_t stPubAttr;
    MI_PANEL_IntfType_e ePnlIntfType;
    MI_HDMI_TimingType_e eHdmiTiming;
}stDispUtDev_t;
static stDispUtDev_t _gastDispUtDev[DISP_DEV_MAX];
#endif
#endif

static void server_on_exit() {
    #if (defined DISP_INIT)
    sstar_disp_Deinit(&stDispPubAttr);
    #endif
}

#if defined(GEN_BT_MSG2UART)
static void print_backtrace(void)
{
    void *bt[16];
    int bt_size;
    char **bt_syms;
    int i;

    bt_size = backtrace(bt, 16);
    bt_syms = backtrace_symbols(bt, bt_size);
    printf("backtrace() returned %d addresses\n", bt_size);
    printf("BACKTRACE ------------\n");

    for (i = 1; i < bt_size; i++)
    {
        printf("%s\n", bt_syms[i]);
    }

    printf("----------------------\n");
    free(bt_syms);
}

void segfault_sigaction(int signo, siginfo_t *si, void *arg)
{
    pid_t pid = getpid();
    char cmd[128] = {0};

    printf("Caught \"%s\"\n", strsignal(signo));
    print_backtrace();
    sprintf(cmd, "cat /proc/%d/maps |tee /mnt/mmc/pid%d_maps.txt", pid, pid);
    system(cmd);
    signal(signo, SIG_DFL);
    raise(signo);
}

void EnableBackTrace()
{
    struct sigaction sa;
    atexit(server_on_exit);
    
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
}
#endif

#ifdef DUAL_OS
static MI_BOOL disp_ut_setdev(MI_DISP_DEV DispDev)
{
    MI_DISP_PubAttr_t stPubAttr;

    memcpy(&stPubAttr, &_gastDispUtDev[DispDev].stPubAttr, sizeof(MI_DISP_PubAttr_t));
    stPubAttr.u32BgColor = YUYV_BLACK;
    MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    MI_DISP_Enable(DispDev);
    _gastDispUtDev[DispDev].bDevEnable = TRUE;

    #if 1
    {
        if(_gastDispUtDev[DispDev].stPubAttr.eIntfType == E_MI_DISP_INTF_LCD
                || _gastDispUtDev[DispDev].stPubAttr.eIntfType == E_MI_DISP_INTF_TTL
                || _gastDispUtDev[DispDev].stPubAttr.eIntfType == E_MI_DISP_INTF_MIPIDSI)
        {
            MI_PANEL_Init(_gastDispUtDev[DispDev].ePnlIntfType);
            _gastDispUtDev[DispDev].bPanelInit = TRUE;
        }
        else if(_gastDispUtDev[DispDev].stPubAttr.eIntfType == E_MI_DISP_INTF_HDMI)
        {
        #if (defined INTERFACE_HDMI) && (INTERFACE_HDMI == 1)
            MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
            MI_HDMI_InitParam_t stInitParam;
            MI_HDMI_TimingType_e eHdmiTiming;
            memset(&stInitParam, 0, sizeof(MI_HDMI_InitParam_t));
            stInitParam.pCallBackArgs = NULL;
            stInitParam.pfnHdmiEventCallback = Hdmi_callback;
            MI_HDMI_Init(&stInitParam);
            MI_HDMI_Open(eHdmi);
            eHdmiTiming = _gastDispUtDev[DispDev].eHdmiTiming;
            Hdmi_Start(eHdmi,eHdmiTiming);
        #endif
        }
    }
    #endif
    return MI_SUCCESS;
}
#endif

int main(int argc, const char *argv[])
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    MI_SYS_Init(0);
#ifdef DUAL_OS
    MI_DISP_DEV DispDev = 0;
    memset(_gastDispUtDev, 0, sizeof(_gastDispUtDev));
    _gastDispUtDev[0].stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    _gastDispUtDev[0].stPubAttr.eIntfType = E_MI_DISP_INTF_TTL;
    _gastDispUtDev[0].ePnlIntfType = E_MI_PNL_INTF_TTL;
    _gastDispUtDev[0].eHdmiTiming = E_MI_HDMI_TIMING_MAX;
    disp_ut_setdev(DispDev);
#endif
#else
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    system("echo 73 >/sys/class/gpio/export");
    system("echo out >/sys/class/gpio/gpio12/direction");
    system("echo 1 >/sys/class/gpio/gpio12/value");
    #if (defined DISP_INIT)
    sstar_disp_init(&stDispPubAttr);
    #endif
#endif
    ST_Fb_Init();
#if defined(GEN_BT_MSG2UART)
    EnableBackTrace();
#endif

#if 0
    char buf[50];
    fd_set stFdSet;
    FD_ZERO(&stFdSet);
    FD_SET(0,&stFdSet);
    for(;;)
    {
        printf("input 'q' exit\n");
        select(1, &stFdSet, NULL, NULL, NULL);
        if(FD_ISSET(0, &stFdSet))
        {
            int i = read(0, buf, sizeof(buf));
            if(i>0 && (buf[0] == 'q'))
            {
                break;
            }
        }
    }
#else
    if (EASYUICONTEXT->initEasyUI()) {
        EASYUICONTEXT->runEasyUI();
        EASYUICONTEXT->deinitEasyUI();
    }
#endif

    MI_SYS_Exit(0);
    return 0;
}
