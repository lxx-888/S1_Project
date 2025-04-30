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

#if defined(SUPPORT_FB1)
#include "window.h"
#include "base.h"
#include "text.h"
#endif

#define ICON_NUM_PER_PAGE	(6)
#define RES_PATH "/customer/UI/res/ui/"

// char g_udiskPath[256] = {0};		// udisk root path
static bool bSdStatus = FALSE;
static bool onButtonClick_BTN_Up(ZKButton *pButton);
static bool onButtonClick_BTN_Down(ZKButton *pButton);
static bool bLayoutPosInit = FALSE;
LayoutPosition gstLayoutOriPos[ICON_NUM_PER_PAGE];// = mVideoview_videoPtr->getPosition();

#if defined(SUPPORT_FB1)
struct rect {
    int x;
    int y;
    int w;
    int h;
};

static Window *pWin = NULL;
static Base *pThumb[ICON_NUM_PER_PAGE] = {NULL};
static struct rect stThumbRect[ICON_NUM_PER_PAGE] = {
    {200, 200, 512, 288},
    {750, 200, 512, 288},
    {1300, 200, 512, 288},
    {200, 600, 512, 288},
    {750, 600, 512, 288},
    {1300, 600, 512, 288}
};
static Text *pSysTime = NULL;
static struct rect stSysTimeRect = {50, 960, 750, 64};
static Text *pFileNameStr = NULL;
static struct rect stFileNameStrRect = {200, 10, 1500, 64};
static Text *pFileDurStr = NULL;
static struct rect stFileDurStrRect = {1550, 960, 300, 64};
static Text *pCurPageStr = NULL;
static struct rect stCurPageStrRect = {10, 466, 64, 64};
static Text *pTotalPageStr = NULL;
static struct rect stTotalPageStrRect = {10, 550, 64, 64};
#endif

static void browserShowRtc(void)
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
    mTXT_SysTimePtr->setText(timeStr);
#if defined(SUPPORT_FB1)
    pSysTime->setText(timeStr);
#endif
}

static void browserShowThumbByIdx(ZKButton* pThumbPtr, char* pFileName, int idx)
{
    char szThumbName[16] = {0,};
    unsigned char u8DB = Carimpl_GetCurDB();

    sprintf(szThumbName, "/tmp/thumb%d.jpg", idx);
    if (u8DB == DB_PHOTO) {
        Carimpl_GetJpegFileThumbnail(pFileName, szThumbName);
    } else {
        Carimpl_GetFileThumbnail(pFileName, szThumbName);
    }
    pThumbPtr->setVisible(true);
    if (access(szThumbName, F_OK) == 0)
        pThumbPtr->setBackgroundPic(szThumbName);
    else
        pThumbPtr->setBackgroundPic("cardv/video_btn_browser.png"); // broken file

#if defined(SUPPORT_FB1)
    if (pThumb[idx]) {
        pThumb[idx]->setVisible(true);
        if (access(szThumbName, F_OK) == 0)
            pThumb[idx]->setBackgroundPic(szThumbName);
        else
            pThumb[idx]->setBackgroundPic(RES_PATH "cardv/video_btn_browser.png"); // broken file
    }
#endif
}

static void browserShowThumbFocus(void)
{
    int s32CurFileIdx = Carimpl_GetCurFileIdx();
    int s32CurBtnIdx = s32CurFileIdx % ICON_NUM_PER_PAGE;
    LayoutPosition stLayoutPos;

    mThumb0Ptr->setPosition(gstLayoutOriPos[0]);
    mThumb1Ptr->setPosition(gstLayoutOriPos[1]);
    mThumb2Ptr->setPosition(gstLayoutOriPos[2]);
    mThumb3Ptr->setPosition(gstLayoutOriPos[3]);
    mThumb4Ptr->setPosition(gstLayoutOriPos[4]);
    mThumb5Ptr->setPosition(gstLayoutOriPos[5]);

    stLayoutPos = gstLayoutOriPos[s32CurBtnIdx];
    stLayoutPos.offsetPosition(10, 10);
    stLayoutPos.mWidth -= 20;
    stLayoutPos.mHeight -= 20;

    switch (s32CurBtnIdx)
    {
    case 0:
        mThumb0Ptr->setPosition(stLayoutPos);
        break;
    case 1:
        mThumb1Ptr->setPosition(stLayoutPos);
        break;
    case 2:
        mThumb2Ptr->setPosition(stLayoutPos);
        break;
    case 3:
        mThumb3Ptr->setPosition(stLayoutPos);
        break;
    case 4:
        mThumb4Ptr->setPosition(stLayoutPos);
        break;
    case 5:
        mThumb5Ptr->setPosition(stLayoutPos);
        break;
    default:
        break;
    }

#if defined(SUPPORT_FB1)
    for (int i = 0; i < ICON_NUM_PER_PAGE; i++)
    {
        if (s32CurBtnIdx == i)
        {
            // Focus
            pThumb[i]->setPosition(stThumbRect[i].x + 10, stThumbRect[i].y + 10,
                stThumbRect[i].w - 20, stThumbRect[i].h - 20);
        }
        else
        {
            pThumb[i]->setPosition(stThumbRect[i].x, stThumbRect[i].y,
                stThumbRect[i].w, stThumbRect[i].h);
        }
    }
#endif
}

static void browserHideThumb(void)
{
    mThumb0Ptr->setVisible(false);
    mThumb1Ptr->setVisible(false);
    mThumb2Ptr->setVisible(false);
    mThumb3Ptr->setVisible(false);
    mThumb4Ptr->setVisible(false);
    mThumb5Ptr->setVisible(false);
    mThumb0_dotPtr->setBackgroundPic("");
    mThumb1_dotPtr->setBackgroundPic("");
    mThumb2_dotPtr->setBackgroundPic("");
    mThumb3_dotPtr->setBackgroundPic("");
    mThumb4_dotPtr->setBackgroundPic("");
    mThumb5_dotPtr->setBackgroundPic("");

#if defined(SUPPORT_FB1)
    for (int i = 0; i < ICON_NUM_PER_PAGE; i++)
    {
        pThumb[i]->setVisible(false);
    }
#endif
}

static void browserShowThumb(bool bSdStatus)
{
    unsigned int u32TotalFile = Carimpl_GetTotalFiles();
    unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
    unsigned int u32FileIdx[ICON_NUM_PER_PAGE];
    char *pszFileName = NULL;
    bool bProtect = false;

    browserHideThumb();

    if (!bSdStatus) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }

    if (u32TotalFile == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        return;
    }

    u32FileIdx[0] = u32CurFileIdx / ICON_NUM_PER_PAGE * ICON_NUM_PER_PAGE;
    for (int i = 0; i < ICON_NUM_PER_PAGE; i++) {
        u32FileIdx[i] = u32FileIdx[0] + i;
        if (u32FileIdx[i] < u32TotalFile) {
            pszFileName = Carimpl_GetCurFileName(u32FileIdx[i]);
            if (pszFileName) {
                bProtect = Carimpl_IsProtectFile(u32FileIdx[i]);
                printf("idx [%d] : %s protect [%d]\n", i, pszFileName, bProtect);
                switch (i) {
                case 0:
                    browserShowThumbByIdx(mThumb0Ptr, pszFileName, i);
                    if (bProtect)
                        mThumb0_dotPtr->setBackgroundPic("cardv/browser_thumbdot.png");
                    else
                        mThumb0_dotPtr->setBackgroundPic("");
                break;
                case 1:
                    browserShowThumbByIdx(mThumb1Ptr, pszFileName, i);
                    if (bProtect)
                        mThumb1_dotPtr->setBackgroundPic("cardv/browser_thumbdot.png");
                    else
                        mThumb1_dotPtr->setBackgroundPic("");
                break;
                case 2:
                    browserShowThumbByIdx(mThumb2Ptr, pszFileName, i);
                    if (bProtect)
                        mThumb2_dotPtr->setBackgroundPic("cardv/browser_thumbdot.png");
                    else
                        mThumb2_dotPtr->setBackgroundPic("");
                break;
                case 3:
                    browserShowThumbByIdx(mThumb3Ptr, pszFileName, i);
                    if (bProtect)
                        mThumb3_dotPtr->setBackgroundPic("cardv/browser_thumbdot.png");
                    else
                        mThumb3_dotPtr->setBackgroundPic("");
                break;
                case 4:
                    browserShowThumbByIdx(mThumb4Ptr, pszFileName, i);
                    if (bProtect)
                        mThumb4_dotPtr->setBackgroundPic("cardv/browser_thumbdot.png");
                    else
                        mThumb4_dotPtr->setBackgroundPic("");
                break;
                case 5:
                    browserShowThumbByIdx(mThumb5Ptr, pszFileName, i);
                    if (bProtect)
                        mThumb5_dotPtr->setBackgroundPic("cardv/browser_thumbdot.png");
                    else
                        mThumb5_dotPtr->setBackgroundPic("");
                break;
                default:
                    printf("CHECK thumnail count\n");
                break;
                }
            }
        } else {
            break;
        }
    }

    browserShowThumbFocus();
}

static void browserShowFileInfo(bool bSdStatus)
{
    unsigned char u8DB = Carimpl_GetCurDB();
    unsigned int u32TotalFile = Carimpl_GetTotalFiles();
    unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
    char *pszFileName = NULL;
    char szFileInfo[32] = {0,};
    int s32Duration = 0;
    unsigned int u32Width = 0, u32Height = 0;

    if (bSdStatus && u32TotalFile > 0) {
        pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
        if (pszFileName) {
            mTXT_FileNamePtr->setText(pszFileName + strlen(SD_ROOT));
#if defined(SUPPORT_FB1)
            pFileNameStr->setText(pszFileName + strlen(SD_ROOT));
#endif
            if (u8DB != DB_PHOTO) {
                s32Duration = Carimpl_GetFileDuration(pszFileName);
                sprintf(szFileInfo, "%02d:%02d:%02d", s32Duration / 3600, (s32Duration % 3600) / 60, s32Duration % 60);
                mTXT_FileDurPtr->setText(szFileInfo);
#if defined(SUPPORT_FB1)
                pFileDurStr->setText(szFileInfo);
#endif
            } else {
                Carimpl_GetJpegFileResolution(pszFileName, &u32Width, &u32Height);
                sprintf(szFileInfo, "%dx%d", u32Width, u32Height);
                mTXT_FileDurPtr->setText(szFileInfo);
#if defined(SUPPORT_FB1)
                pFileDurStr->setText(szFileInfo);
#endif
            }
        }
    } else {
        mTXT_FileNamePtr->setText("");
        mTXT_FileDurPtr->setText("");
#if defined(SUPPORT_FB1)
        pFileNameStr->setText("");
        pFileDurStr->setText("");
#endif
    }
}

static void browserShowPageInfo(bool bSdStatus)
{
    char szStr[16] = {0};
    unsigned int u32TotalFile = Carimpl_GetTotalFiles();
    unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
    unsigned int u32PageCnt = 0;
    unsigned int u32CurPageIdx = 0;

    if (bSdStatus && u32TotalFile > 0) {
        u32PageCnt = u32TotalFile / ICON_NUM_PER_PAGE + (u32TotalFile % ICON_NUM_PER_PAGE ? 1 : 0);
        u32CurPageIdx = u32CurFileIdx / ICON_NUM_PER_PAGE;

        sprintf(szStr, "%d", u32CurPageIdx + 1);
        mTXT_CurPagePtr->setText(szStr);
#if defined(SUPPORT_FB1)
        pCurPageStr->setText(szStr);
#endif
        sprintf(szStr, "%d", u32PageCnt);
        mTXT_TotalPagePtr->setText(szStr);
#if defined(SUPPORT_FB1)
        pTotalPageStr->setText(szStr);
#endif
    } else {
        mTXT_CurPagePtr->setText("0");
#if defined(SUPPORT_FB1)
        pCurPageStr->setText("0");
#endif
        mTXT_TotalPagePtr->setText("0");
#if defined(SUPPORT_FB1)
        pTotalPageStr->setText("0");
#endif
    }
}

static void browserChangePageInfo(bool bSdStatus, int s32Offset)
{
    int  s32TotalFile = Carimpl_GetTotalFiles();
    int  s32CurFileIdx = Carimpl_GetCurFileIdx();
    int  s32CurPageIdx = s32CurFileIdx / ICON_NUM_PER_PAGE;
    int  s32NextPageIdx = 0;
    char szCurPage[16] = {0,};

    if (!bSdStatus || s32TotalFile == 0) {
        return;
    }

    s32CurFileIdx += s32Offset;
    if (s32CurFileIdx >= s32TotalFile) {
        s32CurFileIdx = s32CurFileIdx % s32TotalFile;
    } else if (s32CurFileIdx < 0) {
        s32CurFileIdx = s32CurFileIdx + s32TotalFile;
    }

    Carimpl_SetCurFileIdx(s32CurFileIdx);
    s32NextPageIdx = s32CurFileIdx / ICON_NUM_PER_PAGE;
    if (s32NextPageIdx != s32CurPageIdx) {
        sprintf(szCurPage, "%d", s32NextPageIdx + 1);
        mTXT_CurPagePtr->setText(szCurPage);
#if defined(SUPPORT_FB1)
        pCurPageStr->setText(szCurPage);
#endif
        browserShowThumb(bSdStatus);
    }

    browserShowThumbFocus();
}

static void browserShowAll(bool bSdStatus)
{
    browserShowPageInfo(bSdStatus);
    browserShowFileInfo(bSdStatus);
    browserShowThumb(bSdStatus);
}

static void browserPlay(unsigned char u8ThumbIdx)
{
    unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
    char *pszFileName = NULL;

    u32CurFileIdx = u32CurFileIdx / ICON_NUM_PER_PAGE * ICON_NUM_PER_PAGE + u8ThumbIdx;
    Carimpl_SetCurFileIdx(u32CurFileIdx);
    pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
    if (pszFileName) {
        // enter to player
        gu8LastUIMode = UI_BROWSER_MODE;
        EASYUICONTEXT->openActivity("playerActivity", NULL);
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
    Carimpl_DcfUnmount();
    carimpl_PlaybackFunc_setVideoType();
    Carimpl_SetCurFileIdx(0);
#if defined(SUPPORT_FB1)
    pWin = new Window("Browser", 0, 0, 1920, 1080, 0x08080808);
    if (pWin) {
        for (int i = 0; i < ICON_NUM_PER_PAGE; i++) {
            pThumb[i] = new Base(pWin, stThumbRect[i].x, stThumbRect[i].y, stThumbRect[i].w, stThumbRect[i].h);
        }

        pSysTime = new Text(pWin, stSysTimeRect.x, stSysTimeRect.y, stSysTimeRect.w, stSysTimeRect.h);
        pSysTime->setTextSize(stSysTimeRect.h);
        pSysTime->setVisible(true);

        pFileNameStr = new Text(pWin, stFileNameStrRect.x, stFileNameStrRect.y, stFileNameStrRect.w, stFileNameStrRect.h);
        pFileNameStr->setTextSize(stFileNameStrRect.h);
        pFileNameStr->setVisible(true);

        pFileDurStr = new Text(pWin, stFileDurStrRect.x, stFileDurStrRect.y, stFileDurStrRect.w, stFileDurStrRect.h);
        pFileDurStr->setTextSize(stFileDurStrRect.h);
        pFileDurStr->setVisible(true);

        pCurPageStr = new Text(pWin, stCurPageStrRect.x, stCurPageStrRect.y, stCurPageStrRect.w, stCurPageStrRect.h);
        pCurPageStr->setTextSize(stCurPageStrRect.h);
        pCurPageStr->setVisible(true);

        pTotalPageStr = new Text(pWin, stTotalPageStrRect.x, stTotalPageStrRect.y, stTotalPageStrRect.w, stTotalPageStrRect.h);
        pTotalPageStr->setTextSize(stTotalPageStrRect.h);
        pTotalPageStr->setVisible(true);
    }
#endif
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
    // if (0 != SSTAR_DetectUsbDev((char*)g_udiskPath, sizeof(g_udiskPath)))
    // {
    // 	LOGD("onUI_intent DetectUsbDev fail\n");
    // }
}

static void on_key_callback(int keyCode, int keyStatus) {
    printf("[Browser] key [%s] %s\n", KEYCODE_TO_STRING(keyCode), KEYSTATUS_TO_STRING(keyStatus));
    switch (keyCode) {
    case KEY_ENTER:
        if (keyStatus == UI_KEY_RELEASE) {
            unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
            char *pszFileName = NULL;
            pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
            if (pszFileName) {
                // Enter to player
                gu8LastUIMode = UI_BROWSER_MODE;
                EASYUICONTEXT->openActivity("playerActivity", NULL);
            }
        }
        break;
    case KEY_RIGHT:
        if (keyStatus == UI_KEY_RELEASE) {
            if (Carimpl_GetCurDB() != DB_PHOTO) {
                // Switch browser from video to photo
                carimpl_PlaybackFunc_setImageType();
                Carimpl_SetCurFileIdx(0);
                browserShowAll(bSdStatus);
            } else {
                gu8LastUIMode = UI_BROWSER_MODE;
            #ifndef PLAYING_AND_RECORDING
                carimpl_VideoSensorInit(TRUE);
                carimpl_VideoAllInit(TRUE);
            #endif
                EASYUICONTEXT->openActivity("mainActivity");
                carimpl_VideoFunc_SetResolution();
                carimpl_VideoFunc_SetTimelapse();
            }
        }
        break;
    case KEY_DOWN:
        if (keyStatus == UI_KEY_RELEASE)
            onButtonClick_BTN_Down(mBTN_DownPtr);
        break;
    case KEY_UP:
        if (keyStatus == UI_KEY_RELEASE)
            onButtonClick_BTN_Up(mBTN_UpPtr);
        break;
    case KEY_LEFT:
        break;
    case KEY_MENU:
    case KEY_MENU_2:
        if (keyStatus == UI_KEY_RELEASE) {
            gu8LastUIMode = UI_BROWSER_MODE;
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
    carimpl.stUIModeInfo.u8Mode = UI_BROWSER_MODE;
    IPC_CarInfo_Write_UIModeInfo(&carimpl.stUIModeInfo);

    Carimpl_DcfMount();

    if (Carimpl_GetCurDB() != DB_PHOTO)
        carimpl_PlaybackFunc_setVideoType();
    else
        carimpl_PlaybackFunc_setImageType();

    if (bLayoutPosInit == FALSE) {
        int i = 0;
        gstLayoutOriPos[i++]= mThumb0Ptr->getPosition();
        gstLayoutOriPos[i++]= mThumb1Ptr->getPosition();
        gstLayoutOriPos[i++]= mThumb2Ptr->getPosition();
        gstLayoutOriPos[i++]= mThumb3Ptr->getPosition();
        gstLayoutOriPos[i++]= mThumb4Ptr->getPosition();
        gstLayoutOriPos[i++]= mThumb5Ptr->getPosition();
        bLayoutPosInit = TRUE;
    }

    //register CB FUNC
    set_key_event_callback(on_key_callback);

    bSdStatus = FALSE;
    SHOW_SD_ICON(mTxtSdStatusPtr, bSdStatus);
    browserShowAll(bSdStatus);
    browserShowRtc();

    carimpl_set_colorkey(FB0, false);
#if defined(SUPPORT_FB1)
    carimpl_set_colorkey(FB1, false);
#endif
}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {

}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
    carimpl_show_wmsg(false, WMSG_NONE);
#if defined(SUPPORT_FB1)
    if (pWin) {
        for (int i = 0; i < ICON_NUM_PER_PAGE; i++) {
            if (pThumb[i]) {
                delete pThumb[i];
            }
        }

        if (pSysTime) {
            delete pSysTime;
        }

        if (pFileNameStr) {
            delete pFileNameStr;
        }

        if (pFileDurStr) {
            delete pFileDurStr;
        }

        if (pCurPageStr) {
            delete pCurPageStr;
        }

        if (pTotalPageStr) {
            delete pTotalPageStr;
        }

        delete pWin;
    }
#endif
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
    bool bSdLastStatus = bSdStatus;
    switch (id) {
    case 0:
        browserShowRtc();
        SHOW_SD_ICON(mTxtSdStatusPtr, bSdStatus);
        if (bSdLastStatus != bSdStatus) {
            if (bSdStatus)
                Carimpl_DcfMount();
            browserShowAll(bSdStatus);
        }
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
static bool onbrowserActivityTouchEvent(const MotionEvent &ev) {
    static MotionEvent s_ev;
    bool ret = false;

    switch (ev.mActionStatus) {
    case MotionEvent::E_ACTION_DOWN://触摸按下
        //LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
        s_ev = ev;
        break;
    case MotionEvent::E_ACTION_MOVE://触摸滑动
        break;
    case MotionEvent::E_ACTION_UP:  //触摸抬起
        if ((s_ev.mX > ev.mX) && ((s_ev.mX - ev.mX) > 80)) {
            //slide to the left
            gu8LastUIMode = UI_BROWSER_MODE;
        #ifndef PLAYING_AND_RECORDING
            carimpl_VideoSensorInit(TRUE);
            carimpl_VideoAllInit(TRUE);
        #endif
            EASYUICONTEXT->openActivity("mainActivity");
            carimpl_VideoFunc_SetResolution();
            ret = true;
        }
        s_ev.reset();
        break;
    default:
        break;
    }
    return ret;
}

static bool onButtonClick_BTN_Up(ZKButton *pButton) {
    browserChangePageInfo(bSdStatus, -1);
    browserShowFileInfo(bSdStatus);
    return true;
}

static bool onButtonClick_BTN_Down(ZKButton *pButton) {
    browserChangePageInfo(bSdStatus, 1);
    browserShowFileInfo(bSdStatus);
    return true;
}

static bool onButtonClick_Thumb0(ZKButton *pButton) {
    //LOGD(" ButtonClick Thumb0 !!!\n");
    browserPlay(0);
    return true;
}

static bool onButtonClick_Thumb1(ZKButton *pButton) {
    //LOGD(" ButtonClick Thumb1 !!!\n");
    browserPlay(1);
    return true;
}

static bool onButtonClick_Thumb2(ZKButton *pButton) {
    //LOGD(" ButtonClick Thumb2 !!!\n");
    browserPlay(2);
    return true;
}

static bool onButtonClick_Thumb3(ZKButton *pButton) {
    //LOGD(" ButtonClick Thumb3 !!!\n");
    browserPlay(3);
    return true;
}

static bool onButtonClick_Thumb4(ZKButton *pButton) {
    //LOGD(" ButtonClick Thumb4 !!!\n");
    browserPlay(4);
    return true;
}

static bool onButtonClick_Thumb5(ZKButton *pButton) {
    //LOGD(" ButtonClick Thumb5 !!!\n");
    browserPlay(5);
    return true;
}
