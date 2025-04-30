#include "entry/EasyUIContext.h"
#include "uart/UartContext.h"
#include "manager/ConfigManager.h"
#include "carimpl/KeyEventContext.h"
#include "carimpl/carimpl.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void onEasyUIInit(EasyUIContext *pContext) {
    // 初始化时打开串口
    //printf("get uart name %s\n", CONFIGMANAGER->getUartName().c_str());

    carimpl_set_alpha_blending(FB0, false, 0x00);
    MenuSettingInit();
    Carimpl_SyncAllSetting();
    carimpl_MenuSetting_Init();
    //UARTCONTEXT->openUart(CONFIGMANAGER->getUartName().c_str(), CONFIGMANAGER->getUartBaudRate());
    start_key_event_ctx();
#if defined(SUPPORT_FB1)
    CarUI_Init();
#endif
}

void onEasyUIDeinit(EasyUIContext *pContext) {
    //UARTCONTEXT->closeUart();
    stop_key_event_ctx();
#if defined(SUPPORT_FB1)
    CarUI_DeInit();
#endif
}

const char* onStartupApp(EasyUIContext *pContext) {
//	return "emptyActivity";
    return "mainActivity";
}

#ifdef __cplusplus
}
#endif  /* __cplusplus */