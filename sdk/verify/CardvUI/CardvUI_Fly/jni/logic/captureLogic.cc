#pragma once
#include "uart/ProtocolSender.h"
#include "utils/TimeHelper.h"
#include "carimpl/carimpl.h"
#include "carimpl/KeyEventContext.h"

/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[ID值]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index)
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress)
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX()
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXXPtr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

//static unsigned int gu32Pic_cunt = 0;
static bool bSdStatus = FALSE;
static bool onButtonClick_BTN_Cap(ZKButton *pButton);
struct timeval lastT;

static void CaptureShowRtc(void)
{
    char timeStr[64] = {0};
    struct tm *t = TimeHelper::getDateTime();
    switch (MenuSettingConfig()->uiDateTimeFormat)
    {
    case DATETIME_SETUP_YMD:
        sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + t->tm_year, t->tm_mon + 1, t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec);
        break;
    case DATETIME_SETUP_MDY:
        sprintf(timeStr, "%02d-%02d-%04d %02d:%02d:%02d", t->tm_mon + 1, t->tm_mday, 1900 + t->tm_year, t->tm_hour,t->tm_min,t->tm_sec);
        break;
    case DATETIME_SETUP_DMY:
        sprintf(timeStr, "%02d-%02d-%04d %02d:%02d:%02d", t->tm_mday, t->tm_mon + 1, 1900 + t->tm_year,  t->tm_hour,t->tm_min,t->tm_sec);
        break;
    default:
        sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + t->tm_year, t->tm_mon + 1, t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec);
        break;
    }
    if (MenuSettingConfig()->uiDateTimeFormat == DATETIME_SETUP_NONE)
       mTxtDateTimePtr->setVisible(false);
    else
       mTxtDateTimePtr->setVisible(true);
    mTxtDateTimePtr->setText(timeStr);
}

static void CaptureShowPhotoNum(void)
{
    char szPicNum[8] = {0};
    sprintf(szPicNum, "%05d", IMPL_CAP_PICNUM);
    mpic_cuntPtr->setText(szPicNum);
}

static void CaptureShowPhotoRes(void)
{
    switch (MenuSettingConfig()->uiIMGSize)
    {
    #if (MENU_STILL_SIZE_30M_EN)
    case IMAGE_SIZE_30M:
        mTxtResInfoPtr->setText("30M");
        break;
    #endif
    #if (MENU_STILL_SIZE_14M_EN)
    case IMAGE_SIZE_14M:
        mTxtResInfoPtr->setText("14M");
        break;
    #endif
    #if (MENU_STILL_SIZE_12M_EN)
    case IMAGE_SIZE_12M:
        mTxtResInfoPtr->setText("12M");
        break;
    #endif
    #if (MENU_STILL_SIZE_8M_EN)
    case IMAGE_SIZE_8M:
        mTxtResInfoPtr->setText("8M");
        break;
    #endif
    #if (MENU_STILL_SIZE_6M_WIDE_EN)
    case IMAGE_SIZE_6M_WIDE:
        mTxtResInfoPtr->setText("6M_WIDE");
        break;
    #endif
    #if (MENU_STILL_SIZE_6M_EN)
    case IMAGE_SIZE_6M:
        mTxtResInfoPtr->setText("6M");
        break;
    #endif
    #if (MENU_STILL_SIZE_5M_EN)
    case IMAGE_SIZE_5M:
        mTxtResInfoPtr->setText("5M");
        break;
    #endif
    #if (MENU_STILL_SIZE_3M_EN)
    case IMAGE_SIZE_3M:
        mTxtResInfoPtr->setText("3M");
        break;
    #endif
    #if (MENU_STILL_SIZE_2M_WIDE_EN)
    case IMAGE_SIZE_2M:
        mTxtResInfoPtr->setText("2M");
        break;
    #endif
    #if (MENU_STILL_SIZE_1d2M_EN)
    case IMAGE_SIZE_1_2M:
        mTxtResInfoPtr->setText("1.2M");
        break;
    #endif
    #if (MENU_STILL_SIZE_VGA_EN)
    case IMAGE_SIZE_VGA:
        mTxtResInfoPtr->setText("VGA");
        break;
    #endif
    default:
        mTxtResInfoPtr->setText("2M");
        break;
    }
}

static void CaptureShowEv(void)
{
    switch (MenuSettingConfig()->uiEV)
    {
    #if (MENU_MANUAL_EV_N20_EN)
    case EVVALUE_N20:
        mTxtEvValPtr->setText("EV: -2.00");
        break;
    #endif
    #if (MENU_MANUAL_EV_N17_EN)
    case EVVALUE_N17:
        mTxtEvValPtr->setText("EV: -1.66");
        break;
    #endif
    #if (MENU_MANUAL_EV_N15_EN)
    case EVVALUE_N15:
        mTxtEvValPtr->setText("EV: -1.55");
        break;
    #endif
    #if (MENU_MANUAL_EV_N13_EN)
    case EVVALUE_N13:
        mTxtEvValPtr->setText("EV: -1.33");
        break;
    #endif
    #if (MENU_MANUAL_EV_N10_EN)
    case EVVALUE_N10:
        mTxtEvValPtr->setText("EV: -1.00");
        break;
    #endif
    #if (MENU_MANUAL_EV_N07_EN)
    case EVVALUE_N07:
        mTxtEvValPtr->setText("EV: -0.66");
        break;
    #endif
    #if (MENU_MANUAL_EV_N05_EN)
    case EVVALUE_N05:
        mTxtEvValPtr->setText("EV: -0.55");
        break;
    #endif
    #if (MENU_MANUAL_EV_N03_EN)
    case EVVALUE_N03:
        mTxtEvValPtr->setText("EV: -0.33");
        break;
    #endif
    //#if (MENU_MANUAL_EV_00_EN)    // always exist
    case EVVALUE_00:
        mTxtEvValPtr->setText("EV:  0.00");
        break;
    //#endif
    #if (MENU_MANUAL_EV_P03_EN)
    case EVVALUE_P03:
        mTxtEvValPtr->setText("EV: +0.33");
        break;
    #endif
    #if (MENU_MANUAL_EV_P05_EN)
    case EVVALUE_P05:
        mTxtEvValPtr->setText("EV: +0.55");
        break;
    #endif
    #if (MENU_MANUAL_EV_P07_EN)
    case EVVALUE_P07:
        mTxtEvValPtr->setText("EV: +0.66");
        break;
    #endif
    #if (MENU_MANUAL_EV_P10_EN)
    case EVVALUE_P10:
        mTxtEvValPtr->setText("EV: +1.00");
        break;
    #endif
    #if (MENU_MANUAL_EV_P13_EN)
    case EVVALUE_P13:
        mTxtEvValPtr->setText("EV: +1.33");
        break;
    #endif
    #if (MENU_MANUAL_EV_P15_EN)
    case EVVALUE_P15:
        mTxtEvValPtr->setText("EV: +1.55");
        break;
    #endif
    #if (MENU_MANUAL_EV_P17_EN)
    case EVVALUE_P17:
        mTxtEvValPtr->setText("EV: +1.66");
        break;
    #endif
    #if (MENU_MANUAL_EV_P20_EN)
    case EVVALUE_P20 :
        mTxtEvValPtr->setText("EV: +2.00");
        break;
    #endif
    default:
        mTxtEvValPtr->setText("EV:  0.00");
        break;
    }
}

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
    {0,  1000}, //定时器id=0, 时间间隔6秒
    //{1,  1000},
};

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
    //if (!EASYUICONTEXT->isStatusBarShow())
    //	EASYUICONTEXT->showStatusBar();
    gettimeofday(&lastT, NULL);
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
}

static void on_key_callback(int keyCode, int keyStatus) {
    printf("[Capture] key [%s] %s\n", KEYCODE_TO_STRING(keyCode), KEYSTATUS_TO_STRING(keyStatus));
    switch (keyCode) {
    case KEY_ENTER:
        if (keyStatus == UI_KEY_RELEASE) {
            struct timeval curT;
            gettimeofday(&curT, NULL);
            long tv_sec,tv_usec;

            tv_sec = curT.tv_sec - lastT.tv_sec;
            if (lastT.tv_usec > curT.tv_usec) {
                tv_sec --;
                tv_usec = 1000000 + curT.tv_usec - lastT.tv_usec;
            } else {
                tv_usec = curT.tv_usec - lastT.tv_usec;
            }

            if (tv_sec == 0 && tv_usec < 300000) {
                //printf("%s:%ld.%ldsec\n",__FUNCTION__,tv_sec,tv_usec);
            } else {
                onButtonClick_BTN_Cap(mBTN_CapPtr);
                lastT.tv_sec = curT.tv_sec;
                lastT.tv_usec = curT.tv_usec;
            }
        }
    break;
    case KEY_RIGHT:
        if (keyStatus == UI_KEY_RELEASE) {
            gu8LastUIMode = UI_CAPTURE_MODE;
        #ifndef PLAYING_AND_RECORDING
            carimpl_VideoJpegInit(FALSE);
            carimpl_VideoSensorInit(FALSE);
        #endif
            EASYUICONTEXT->openActivity("browserActivity");
        }
    break;
    case KEY_LEFT:
    case KEY_UP:
        break;
    case KEY_DOWN:
        if (keyStatus == UI_KEY_RELEASE)
            carimpl_GeneralFunc_SetPreviewCamIdx();
        break;
    case KEY_MENU:
    case KEY_MENU_2:
        if (keyStatus == UI_KEY_RELEASE) {
            gu8LastUIMode = UI_CAPTURE_MODE;
            EASYUICONTEXT->openActivity("VideoMenuActivity");
        }
        break;
    case KEY_POWER:
        if (keyStatus == UI_KEY_LPRESS)
            carimpl_Poweroff_handler();
        break;
    default:
    break;
    }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {
    //register CB FUNC
    set_key_event_callback(on_key_callback);

    carimpl.stUIModeInfo.u8Mode = UI_CAPTURE_MODE;
    IPC_CarInfo_Write_UIModeInfo(&carimpl.stUIModeInfo);

    carimpl_CaptureFunc_SetResolution();
    IPC_CarInfo_Read_CapInfo(&carimpl.stCapInfo);
    CaptureShowPhotoRes();
    CaptureShowPhotoNum();
    CaptureShowRtc();
    CaptureShowEv();

    bSdStatus = FALSE;
    SHOW_SD_ICON(mTxtSdStatusPtr, bSdStatus);

    #ifdef USE_KEY_BOARD
    mBTN_CapPtr->setVisible(FALSE);
    #endif
    carimpl_set_colorkey(FB0, true);
#if defined(SUPPORT_FB1)
    carimpl_set_colorkey(FB1, true);
#endif
}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {
    if (EASYUICONTEXT->isStatusBarShow())
        EASYUICONTEXT->hideStatusBar();
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {

}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {

}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id) {
    switch (id) {
        case 0:
            IPC_CarInfo_Read_CapInfo(&carimpl.stCapInfo);
            CaptureShowPhotoRes();
            CaptureShowPhotoNum();
            CaptureShowRtc();
            CaptureShowEv();
            SHOW_BAT_ICON(mTxtChgStuPtr);
            SHOW_SD_ICON(mTxtSdStatusPtr, bSdStatus);
        break;
        default:
            break;
    }
    return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool oncaptureActivityTouchEvent(const MotionEvent &ev) {
    static MotionEvent s_ev;
    bool ret = false;

    switch (ev.mActionStatus) {
        case MotionEvent::E_ACTION_DOWN://触摸按下
            //LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
            s_ev = ev;
            break;
        case MotionEvent::E_ACTION_MOVE://触摸滑动
            //printf("cap E_ACTION_MOVE (%d,%d)\r\n",ev.mX,ev.mY);
            break;
        case MotionEvent::E_ACTION_UP:  //触摸抬起
            if ((s_ev.mX > ev.mX) && ((s_ev.mX - ev.mX) > 200)) {
                //slide to the left
                gu8LastUIMode = UI_CAPTURE_MODE;
            #ifndef PLAYING_AND_RECORDING
                carimpl_VideoJpegInit(FALSE);
                carimpl_VideoSensorInit(FALSE);
            #endif
                EASYUICONTEXT->openActivity("browserActivity");
                ret = true;
            }
            s_ev.reset();
            break;
        default:
            break;
    }
    return ret;
}
static bool onButtonClick_BTN_Cap(ZKButton *pButton) {
    //LOGD(" ButtonClick BTN_Cap !!!\n");
    if (IMPL_SD_ISMOUNT && (bSdStatus == TRUE)) {

    }
    else if ((IMPL_SD_ISMOUNT && (bSdStatus == FALSE)) ||
        (!IMPL_SD_ISMOUNT && (bSdStatus == TRUE)))
        carimpl_show_wmsg(true, WMSG_INSERT_SD_AGAIN);
    else {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
    }
    carimpl_VideoFunc_Capture();
    return true;
}
