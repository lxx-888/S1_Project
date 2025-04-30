#pragma once
#include "uart/ProtocolSender.h"
#include <sstream>
#include <sys/time.h>
#include "carimpl/KeyEventContext.h"
#include "carimpl/carimpl.h"

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
#define FIT_IN_RANGE(X,LOW,HIGH)    (((X)>(HIGH)) ? (HIGH) : (((X)<(LOW))?(LOW):(X)) )
#define ALIGN_UP( X, Y )            (((X)+(Y)-1)/(Y)*(Y))
#define ALIGN_DOWN( X, Y )          (((X))/(Y)*(Y))

#define CONFIRM_POS_YES             (0)
#define CONFIRM_POS_NO              (1)

#define HIDE_ITEMS                  (2) // Hide protect & delete, Only show in browser

static int Datetime[6] = {2019,01,01,12,0,0};//year,month,day,hour,minute

static PSMENUSTRUCT pTopMenu = NULL;
static PSMENUSTRUCT pSubMenu = NULL;
static PSMENUSTRUCT pSubMenuSub = NULL;
static PSMENUITEM   pCurItem = NULL;

static PSMENUSTRUCT MenuPageList[] =
{
#if (MENU_MOVIE_PAGE_EN)
    &sMainMenuVideo,
#endif
#if (MENU_STILL_PAGE_EN)
    &sMainMenuStill,
#endif
#if (MENU_PLAYBACK_PAGE_EN)
    &sMainMenuPlayback,
#endif
#if (MENU_MEDIA_PAGE_EN)
    &sMainMenuMedia,
#endif
#if (MENU_GENERAL_PAGE_EN)
    &sMainMenuGeneral,
#endif
};

static bool onButtonClick_Button1(ZKButton *pButton);
static void onListItemClick_MenuListview(ZKListView *pListView, int index, int id);
static void onListItemClick_subMenuListView(ZKListView *pListView, int index, int id);
static bool onButtonClick_DecParam(ZKButton *pButton);
static bool onButtonClick_IncParam(ZKButton *pButton);
static bool onButtonClick_clocksetting_add(ZKButton *pButton);
static bool onButtonClick_clocksetting_del(ZKButton *pButton);
static bool onButtonClick_clocksetting_ok(ZKButton *pButton);
static bool onButtonClick_SubMenuConfirm_yes(ZKButton *pButton);
static bool onButtonClick_SubMenuConfirm_no(ZKButton *pButton);
static void onListItemClick_subMenuListSubView(ZKListView *pListView, int index, int id);
static void MenuShowSDCardInfoPage(void);

static void MenuSliderBarNext(int index)
{
    switch (index)
    {
    case 0:
        mSiderbar5Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar1Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 1:
        mSiderbar1Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar2Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 2:
        mSiderbar2Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar3Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 3:
        mSiderbar3Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar4Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 4:
        mSiderbar4Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar5Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    default:
        mSiderbar5Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar1Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    }
}

static void MenuSliderBarPrev(int index)
{
    switch (index)
    {
    case 0:
        mSiderbar2Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar1Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 1:
        mSiderbar3Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar2Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 2:
        mSiderbar4Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar3Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 3:
        mSiderbar5Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar4Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    case 4:
        mSiderbar1Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar5Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    default:
        mSiderbar2Ptr->setBackgroundPic("cardv/slider_drag_normal.png");
        mSiderbar1Ptr->setBackgroundPic("cardv/slider_drag_foucs.png");
        break;
    }
}

static PSMENUSTRUCT GetNextCatagoryMenu(PSMENUSTRUCT pMenu)
{
    unsigned int i, Next;

    for (i = 0; i < MENU_PAGE_NUM; i++)
    {
        if (pMenu->iMenuId == MenuPageList[i]->iMenuId)
        {
            Next = ((i + 1) == MENU_PAGE_NUM) ? (0) : (i + 1);
            MenuSliderBarNext(Next);
            return (MenuPageList[Next]);
        }
    }

    return &sMainMenuVideo;
}

static PSMENUSTRUCT GetPrevCatagoryMenu(PSMENUSTRUCT pMenu)
{
    unsigned int i, Prev;

    for (i = 0; i < MENU_PAGE_NUM; i++)
    {
        if (pMenu->iMenuId == MenuPageList[i]->iMenuId)
        {
            Prev = (i == 0) ? (MENU_PAGE_NUM - 1) : (i - 1);
            MenuSliderBarPrev(Prev);
            return (MenuPageList[Prev]);
        }
    }

    return &sMainMenuVideo;
}
/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
    // {0,  50}, //定时器id=0, 时间间隔50毫秒
};

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
    if (strcmp(EASYUICONTEXT->currentAppName(), "captureActivity") == 0) {
        pTopMenu = &sMainMenuStill;
        printf("set pTopMenu [%s] Line [%d]\n", MAP_STRINGID(pTopMenu->iStringId), __LINE__);
        MenuSliderBarNext(1);
        carimpl_set_alpha_blending(FB0, true, 0xCC);
    } else if (strcmp(EASYUICONTEXT->currentAppName(), "browserActivity") == 0 ||
               strcmp(EASYUICONTEXT->currentAppName(), "playerActivity") == 0) {
        pTopMenu = &sMainMenuPlayback;
        pTopMenu->iCurrentPos = 0; //UI init,need reset Pos
        printf("set pTopMenu [%s] Line [%d]\n", MAP_STRINGID(pTopMenu->iStringId), __LINE__);
        MenuSliderBarNext(2);
        carimpl_set_alpha_blending(FB0, false, 0xCC);
    } else {
        pTopMenu = &sMainMenuVideo;
        printf("set pTopMenu [%s] Line [%d]\n", MAP_STRINGID(pTopMenu->iStringId), __LINE__);
        MenuSliderBarNext(0);
        carimpl_set_alpha_blending(FB0, true, 0xCC);
    }
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
}

static int OffsetItemNumber(int iCur, int iMin, int iMax, int iOffset, bool bWrap)
{
    int iRange, iResult,iMul;
    bool bSign;

    iRange  = iMax - iMin + 1;
    iCur    = FIT_IN_RANGE(iCur, iMin, iMax) - iMin;

    // Offset is positive or negative
    bSign = (iOffset < 0) ? (true) : (false);

    if (bWrap) {
        // Align up
        iMul = (bSign)?(-iOffset):(iOffset);
        iMul = ALIGN_UP(iMul, iRange);

        // Calculate the position
        iCur = (iCur+iMul+iOffset) % iRange;

        iResult = iCur + iMin;
    } else {
        // Non Wrap
        iResult = FIT_IN_RANGE(iCur+iMin+iOffset, iMin, iMax);
    }

    return iResult;
}

static void HandleClockSetting(int iOffset)
{
    ZKButton* ptrButton[] = {mclocksetting_addPtr, mclocksetting_delPtr, mclocksetting_okPtr};
    int iNum = 3;
    for (int i = 0; i < iNum; i++) {
        if (ptrButton[i]->isSelected()) {
            if (iOffset > 0) {
                if (i == iNum - 1)
                    ptrButton[0]->setSelected(true);
                else
                    ptrButton[i+1]->setSelected(true);
            } else if (iOffset < 0) {
                if (i == 0)
                    ptrButton[iNum-1]->setSelected(true);
                else
                    ptrButton[i-1]->setSelected(true);
            }
            ptrButton[i]->setSelected(false);
            break;
        }
    }
}

static void HandleConfirmMenu(int iOffset, bool bEnter)
{
    PSMENUSTRUCT pCurMenu = NULL;
    PSMENUSTRUCT pConfirmMenu = NULL;
    PSMENUITEM pConfirmItem = NULL;
    if (msubMenuListSubViewPtr->isVisible()) {
        pCurMenu = pSubMenuSub;
    } else {
        pCurMenu = pSubMenu;
    }

    pConfirmItem = pCurMenu->pItemsList[pCurMenu->iCurrentPos];
    pConfirmMenu = pConfirmItem->pSubMenu;
    if (pConfirmMenu) {
        if (bEnter == FALSE) {
            pConfirmMenu->iCurrentPos = OffsetItemNumber(pConfirmMenu->iCurrentPos, 0, pConfirmMenu->iNumOfItems-1, iOffset, true);
            if (pConfirmMenu->iCurrentPos == CONFIRM_POS_NO) {
                mSubMenuConfirm_yesPtr->setSelected(false);
                mSubMenuConfirm_noPtr->setSelected(true);
            } else {
                mSubMenuConfirm_yesPtr->setSelected(true);
                mSubMenuConfirm_noPtr->setSelected(false);
            }
        } else {
            if (pConfirmMenu->iCurrentPos == CONFIRM_POS_YES) {
                onButtonClick_SubMenuConfirm_yes(mSubMenuConfirm_yesPtr);
            } else {
                onButtonClick_SubMenuConfirm_no(mSubMenuConfirm_noPtr);
            }
            pConfirmMenu->iCurrentPos = 0; //After Confirm, reset pos to 0
        }
    } else {
        if (bEnter == FALSE) {
            pCurMenu->iCurrentPos = OffsetItemNumber(pCurMenu->iCurrentPos, 0, pCurMenu->iNumOfItems-1, iOffset, true);
            if (pCurMenu->iCurrentPos == CONFIRM_POS_NO) {
                mSubMenuConfirm_yesPtr->setSelected(false);
                mSubMenuConfirm_noPtr->setSelected(true);
            } else {
                mSubMenuConfirm_yesPtr->setSelected(true);
                mSubMenuConfirm_noPtr->setSelected(false);
            }
        } else {
            if (pCurMenu->iCurrentPos == CONFIRM_POS_YES) {
                onButtonClick_SubMenuConfirm_yes(mSubMenuConfirm_yesPtr);
            } else {
                onButtonClick_SubMenuConfirm_no(mSubMenuConfirm_noPtr);
            }
        }
    }
}

static void HandleTopMenuSwitch(int iOffset)
{
    if (iOffset < 0)
        pTopMenu = GetPrevCatagoryMenu(pTopMenu);
    else if (iOffset > 0)
        pTopMenu = GetNextCatagoryMenu(pTopMenu);
    printf("set pTopMenu [%s] Line [%d]\n", MAP_STRINGID(pTopMenu->iStringId), __LINE__);
    mMenuStatusModePtr->setTextTr(MAP_STRINGID(pTopMenu->iStringId));
    mMenuListviewPtr->setSelection(pTopMenu->iCurrentPos);
    mMenuListviewPtr->refreshListView();
}

static void HandleTopMenuItem(int iOffset)
{
    int iNumOfItems = pTopMenu->iNumOfItems - 1;
    if (pTopMenu->iMenuId == MENUID_MAIN_MENU_PLAYBACK &&
        gu8LastUIMode != UI_BROWSER_MODE &&
        gu8LastUIMode != UI_PLAYBACK_MODE) {
        // Hide protect & delete
        iNumOfItems -= HIDE_ITEMS;
    }

    pTopMenu->iCurrentPos = OffsetItemNumber(pTopMenu->iCurrentPos, 0, iNumOfItems, iOffset, true);
    mMenuListviewPtr->setSelection(pTopMenu->iCurrentPos);
    mMenuListviewPtr->refreshListView();
}

static void HandleSubMenu(int iOffset, bool bAdjustItem)
{
    pSubMenu->iCurrentPos = OffsetItemNumber(pSubMenu->iCurrentPos, 0, pSubMenu->iNumOfItems-1, iOffset, true);
    if (bAdjustItem) {
        if (pSubMenu->iCurrentPos == 0) {
            mDecParamPtr->setSelected(true);
            mIncParamPtr->setSelected(false);
        } else {
            mDecParamPtr->setSelected(false);
            mIncParamPtr->setSelected(true);
            if(pSubMenu->iCurrentPos > 1) {
            	printf("HandleSubMenu pSubMenu->iCurrentPos: %d", pSubMenu->iCurrentPos);
            	if(pSubMenu->iCurrentPos == 2) {
            		mDecParamPtr->setSelected(true);		//for down key-event
            		mIncParamPtr->setSelected(false);
            		pSubMenu->iCurrentPos = 0;
            	} else {
            		pSubMenu->iCurrentPos = 1;
            	}
            }
        }
    } else {
        msubMenuListViewPtr->setSelection(pSubMenu->iCurrentPos);
        msubMenuListViewPtr->refreshListView();
    }
}

static void HandleSubMenuSub(int iOffset)
{
    pSubMenuSub->iCurrentPos = OffsetItemNumber(pSubMenuSub->iCurrentPos, 0, pSubMenuSub->iNumOfItems-1, iOffset, true);
    msubMenuListSubViewPtr->setSelection(pSubMenuSub->iCurrentPos);
    msubMenuListSubViewPtr->refreshListView();
}

static void on_key_callback(int keyCode, int keyStatus) {
    printf("[MenuLogic] key [%s] %s\n", KEYCODE_TO_STRING(keyCode), KEYSTATUS_TO_STRING(keyStatus));
    switch (keyCode)
    {
    case KEY_UP:
        if (keyStatus == UI_KEY_RELEASE) {
            if (mSubMenuConfirmPtr->isVisible()) {
                HandleConfirmMenu(-1, FALSE);
            } else if (msubMenuListSubViewPtr->isVisible()) {
                HandleSubMenuSub(-1);
            } else if (mSubMenuAdjustParamPtr->isVisible()) {
                HandleSubMenu(-1, true);
            } else if (msubMenuClockSettingPtr->isVisible()) {
                HandleClockSetting(-1);
            } else if (msubMenuListViewPtr->isVisible()) {
                HandleSubMenu(-2, false);
            } else if (!mSDCardInfoPtr->isVisible()) {
                HandleTopMenuItem(-1);
            }
        }
        break;
    case KEY_DOWN:
        if (keyStatus == UI_KEY_RELEASE) {
            if (mSubMenuConfirmPtr->isVisible()) {
                HandleConfirmMenu(1, FALSE);
            } else if (msubMenuListSubViewPtr->isVisible()) {
                HandleSubMenuSub(1);
            } else if (mSubMenuAdjustParamPtr->isVisible()) {
                HandleSubMenu(1, true);
            } else if (msubMenuClockSettingPtr->isVisible()) {
                HandleClockSetting(1);
            } else if (msubMenuListViewPtr->isVisible()) {
                HandleSubMenu(2, false);
            } else if (!mSDCardInfoPtr->isVisible()) {
                HandleTopMenuItem(1);
            }
        }
        break;
    case KEY_LEFT:
        if (keyStatus == UI_KEY_RELEASE) {
            if (mSubMenuConfirmPtr->isVisible()) {
                HandleConfirmMenu(-1, FALSE);
            } else if (msubMenuListSubViewPtr->isVisible()) {
                HandleSubMenuSub(-1);
            } else if (mSubMenuAdjustParamPtr->isVisible()) {
                HandleSubMenu(-1, true);
            } else if (msubMenuClockSettingPtr->isVisible()) {
                HandleClockSetting(-1);
            } else if (msubMenuListViewPtr->isVisible()) {
                HandleSubMenu(-1, false);
            } else if (!mSDCardInfoPtr->isVisible()) {
                HandleTopMenuSwitch(-1);
            }
        }
        break;
    case KEY_RIGHT:
        if (keyStatus == UI_KEY_RELEASE) {
            if (mSubMenuConfirmPtr->isVisible()) {
                HandleConfirmMenu(1, FALSE);
            } else if (msubMenuListSubViewPtr->isVisible()) {
                HandleSubMenuSub(1);
            } else if (mSubMenuAdjustParamPtr->isVisible()) {
                HandleSubMenu(1, true);
            } else if (msubMenuClockSettingPtr->isVisible()) {
                HandleClockSetting(1);
            } else if (msubMenuListViewPtr->isVisible()) {
                HandleSubMenu(1, false);
            } else if (!mSDCardInfoPtr->isVisible()) {
                HandleTopMenuSwitch(1);
            }
        }
        break;
    case KEY_MENU:
    case KEY_MENU_2:
        if (keyStatus == UI_KEY_RELEASE)
            onButtonClick_Button1(mButton1Ptr);
        break;
    case KEY_ENTER:
        if (keyStatus == UI_KEY_RELEASE) {
            if (mSubMenuConfirmPtr->isVisible()) {
                HandleConfirmMenu(0, TRUE);
            } else if (msubMenuListSubViewPtr->isVisible()) {
                onListItemClick_subMenuListSubView(msubMenuListSubViewPtr, pSubMenuSub->iCurrentPos, 0);
            } else if (mSubMenuAdjustParamPtr->isVisible()) {
                if (pSubMenu->iCurrentPos == 0)
                    onButtonClick_DecParam(mDecParamPtr);
                else
                    onButtonClick_IncParam(mIncParamPtr);
            } else if (msubMenuClockSettingPtr->isVisible()) {
                if (mclocksetting_addPtr->isSelected())
                    onButtonClick_clocksetting_add(mclocksetting_addPtr);
                else if (mclocksetting_delPtr->isSelected())
                    onButtonClick_clocksetting_del(mclocksetting_delPtr);
                else if (mclocksetting_okPtr->isSelected())
                    onButtonClick_clocksetting_ok(mclocksetting_okPtr);
            } else if (msubMenuListViewPtr->isVisible()) {
                onListItemClick_subMenuListView(msubMenuListViewPtr, pSubMenu->iCurrentPos, 0);
            } else if (!mSDCardInfoPtr->isVisible()) {
                onListItemClick_MenuListview(mMenuListviewPtr, pTopMenu->iCurrentPos, 0);
            }
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
    carimpl.stUIModeInfo.u8Mode = UI_MENU_MODE;
    IPC_CarInfo_Write_UIModeInfo(&carimpl.stUIModeInfo);

    //if (EASYUICONTEXT->isStatusBarShow())
    //	EASYUICONTEXT->hideStatusBar();
    //register CB FUNC
    set_key_event_callback(on_key_callback);
}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {
    carimpl_set_alpha_blending(FB0, false, 0xCC);
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
    carimpl_set_alpha_blending(FB0, false, 0xCC);
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
static bool onVideoMenuActivityTouchEvent(const MotionEvent &ev) {
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
            if ((s_ev.mX > ev.mX) && ((s_ev.mX - ev.mX) > 80)) {//slide to the left
                pTopMenu = GetNextCatagoryMenu(pTopMenu);
                printf("set pTopMenu [%s] Line [%d]\n", MAP_STRINGID(pTopMenu->iStringId), __LINE__);
                mMenuStatusModePtr->setTextTr(MAP_STRINGID(pTopMenu->iStringId));
                mMenuListviewPtr->refreshListView();
                ret = true;
            }

            s_ev.reset();
            break;
        default:
            break;
    }
    return ret;
}

static int getListItemCount_MenuListview(const ZKListView *pListView) {
    //LOGD("getListItemCount_MenuListview !\n");

    if (pTopMenu->iMenuId == MENUID_MAIN_MENU_PLAYBACK &&
        gu8LastUIMode != UI_BROWSER_MODE &&
        gu8LastUIMode != UI_PLAYBACK_MODE) {
        // Hide protect & delete
        return pTopMenu->iNumOfItems - HIDE_ITEMS;
    }

    return pTopMenu->iNumOfItems;
}

static void obtainListItemData_MenuListview(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ MenuListview  !!!\n");
    ZKListView::ZKListSubItem* sub = NULL;
    if (mSubMenuAdjustParamPtr->isVisible() || msubMenuClockSettingPtr->isVisible())
        mMenuStatusModePtr->setTextTr(MAP_STRINGID(pSubMenu->iStringId));
    else
        mMenuStatusModePtr->setTextTr(MAP_STRINGID(pTopMenu->iStringId));

    mMenuSlideBarPtr->setVisible(true);
    pListView->setSelection(pTopMenu->iCurrentPos);
    sub = pListItem->findSubItemByID(ID_VIDEOMENU_MenuItemKey);
    if (sub) {
        if (pTopMenu->iCurrentPos == index)
            sub->setSelected(true);
        else
            sub->setSelected(false);
        sub->setTextTr(MAP_STRINGID(pTopMenu->pItemsList[index]->pSubMenu->iStringId));
    }
    sub = pListItem->findSubItemByID(ID_VIDEOMENU_MenuItemVaule);
    if (sub) {
        int CurValue = 0;
        PSMENUITEM pItem = pTopMenu->pItemsList[index];
        if (pItem->pSubMenu->pfMenuGetDefaultVal != NULL)
            CurValue = pItem->pSubMenu->pfMenuGetDefaultVal(pItem->pSubMenu);
        else {
            sub->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
            return;
        }
        if (pItem->iItemId == ITEMID_VOLUME ||
            pItem->iItemId == ITEMID_CONTRAST ||
            pItem->iItemId == ITEMID_SATURATION ||
            pItem->iItemId == ITEMID_SHARPNESS ||
            pItem->iItemId == ITEMID_GAMMA ||
	    pItem->iItemId == ITEMID_MOVIE_RECORD_VOLUME ||
            pItem->iItemId == ITEMID_LCD_BRI) {
            sub->setText(CurValue);
        } else if (pItem->iItemId == ITEMID_FW_VERSION_INFO) {
            char *CmdBuf = carimpl_GetSettingCmdBuf();
            sub->setText(CmdBuf);//net_config.bin fwver
        } else {
            sub->setTextTr(MAP_STRINGID(pItem->pSubMenu->pItemsList[CurValue]->iStringId));
        }
    }
}

static void onListItemClick_MenuListview(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ MenuListview  !!!\n");
    pCurItem = pTopMenu->pItemsList[index];
    printf("set pCurItem [%s] Line [%d]\n", MAP_STRINGID(pCurItem->iStringId), __LINE__);
    pSubMenu = pCurItem->pSubMenu;
    printf("set pSubMenu [%s] Line [%d]\n", pSubMenu ? MAP_STRINGID(pSubMenu->iStringId) : "No SubMenu", __LINE__);

    if (pSubMenu != NULL) {
        pSubMenu->pParentMenu = pTopMenu;

        if (pCurItem->iItemId == ITEMID_VOLUME ||
            pCurItem->iItemId == ITEMID_CONTRAST ||
            pCurItem->iItemId == ITEMID_SATURATION ||
            pCurItem->iItemId == ITEMID_SHARPNESS ||
            pCurItem->iItemId == ITEMID_GAMMA ||
	    pCurItem->iItemId == ITEMID_MOVIE_RECORD_VOLUME ||
	    pCurItem->iItemId == ITEMID_LCD_BRI) {
            if (pCurItem->pSubMenu->pfMenuGetDefaultVal != NULL) {
                int CurValue = 0;
                CurValue = pCurItem->pSubMenu->pfMenuGetDefaultVal(pCurItem->pSubMenu);
                pSubMenu->iCurrentPos = 0;
                mSubMenuAdjustParamPtr->setVisible(true);
                mDecParamPtr->setSelected(true);
                mIncParamPtr->setSelected(false);
                mSetParamPtr->setText(CurValue);
                mAdjustParamPtr->setProgress(CurValue);
                msubMenuListViewPtr->setSelection(pSubMenu->iCurrentPos);
                msubMenuListViewPtr->refreshListView();
            }
        } else if (pCurItem->iItemId == ITEMID_CLOCK_SETTINGS) {
            msubMenuClockSettingPtr->setVisible(true);
            myear_valuePtr->setText(Datetime[IDX_YEAR]);
            mmon_valuePtr->setText(Datetime[IDX_MONTH]);
            mday_valuePtr->setText(Datetime[IDX_DAY]);
            mhour_valuePtr->setText(Datetime[IDX_HOUR]);
            mmin_valuePtr->setText(Datetime[IDX_MIN]);
            myear_valuePtr->setSelected(true);
            mmon_valuePtr->setSelected(false);
            mday_valuePtr->setSelected(false);
            mhour_valuePtr->setSelected(false);
            mmin_valuePtr->setSelected(false);
            mclocksetting_addPtr->setSelected(true);
        } else if (pCurItem->iItemId == ITEMID_RESET_SETUP) {
            pCurItem->pSubMenu->iCurrentPos = 0;
            mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_RESET_SETUP_CONFIRM));
            mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_RESET_INFO));
            mSubMenuConfirmPtr->setVisible(true);
            mSubMenuConfirm_yesPtr->setSelected(true);
            mSubMenuConfirm_noPtr->setSelected(false);
        } else if (pCurItem->iItemId == ITEMID_FORMAT_SD_CARD) {
            pCurItem->pSubMenu->iCurrentPos = 0;
            mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_FORMAT_CARD_CONFIRM));
            mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_DATA_DELETED));
            mSubMenuConfirmPtr->setVisible(true);
            mSubMenuConfirm_yesPtr->setSelected(true);
            mSubMenuConfirm_noPtr->setSelected(false);
        } else if (pCurItem->iItemId == ITEMID_SD_CARD_INFO) {
            MenuShowSDCardInfoPage();
        } else if (pCurItem->iItemId == ITEMID_FW_VERSION_INFO) {
            return;
        } else if (pCurItem->iItemId == ITEMID_SYSTEM_SETTINGS) {
            EASYUICONTEXT->openActivity("ZKSettingActivity");
        } else {
            msubMenuListViewPtr->setVisible(true);
            if (pSubMenu->pfMenuGetDefaultVal != NULL)
                pSubMenu->iCurrentPos = pSubMenu->pfMenuGetDefaultVal(pSubMenu);
            else
                pSubMenu->iCurrentPos = 0;
        }
        mMenuSlideBarPtr->setVisible(false);
    } else if (pCurItem->pfItemSelectHandler != NULL) {
        pCurItem->pfItemSelectHandler(pCurItem);
    }
}

static bool onButtonClick_Button1(ZKButton *pButton) {
    //LOGD(" ButtonClick Button1 !!!\n");
    //EASYUICONTEXT->closeActivity("VideoMenuActivity");
    if (mSubMenuConfirmPtr->isVisible()) {
        return false;
    }

    if (msubMenuListSubViewPtr->isVisible()) {
        msubMenuListSubViewPtr->setVisible(false);
    } else if (mSubMenuAdjustParamPtr->isVisible()) {
        mSubMenuAdjustParamPtr->setVisible(false);
    } else if (msubMenuClockSettingPtr->isVisible()) {
        msubMenuClockSettingPtr->setVisible(false);
    } else if (mSDCardInfoPtr->isVisible()) {
        mSDCardInfoPtr->setVisible(false);
    } else if (msubMenuListViewPtr->isVisible()) {
        msubMenuListViewPtr->setVisible(false);
    } else {
        Carimpl_SyncAllSetting();
        Menu_WriteSetting();
        gu8LastUIMode = UI_MENU_MODE;
        EASYUICONTEXT->goBack();
    }
    return false;
}

static int getListItemCount_subMenuListView(const ZKListView *pListView) {
    //LOGD("getListItemCount_subMenuListView !\n");
    return pSubMenu->iNumOfItems;
}

static void obtainListItemData_subMenuListView(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ subMenuListView  !!!\n");
    ZKListView::ZKListSubItem* sub = NULL;
    mMenuStatusModePtr->setTextTr(MAP_STRINGID(pSubMenu->iStringId));
    pListView->setSelection(pSubMenu->iCurrentPos);
    sub = pListItem->findSubItemByID(ID_VIDEOMENU_SubItem);
    if (sub) {
        PSMENUITEM pItem = pSubMenu->pItemsList[index];
        sub->setTextTr(MAP_STRINGID(pItem->iStringId));
        if (index == pSubMenu->iCurrentPos)
            sub->setSelected(true);
        else
            sub->setSelected(false);
    }
}

static void onListItemClick_subMenuListView(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ subMenuListView  !!!\n");
    pCurItem = pSubMenu->pItemsList[index];
    printf("set pCurItem [%s] Line [%d]\n", MAP_STRINGID(pCurItem->iStringId), __LINE__);
    pSubMenuSub = pCurItem->pSubMenu;
    printf("set pSubMenuSub [%s] Line [%d]\n", pSubMenuSub ? MAP_STRINGID(pSubMenuSub->iStringId) : "No SubMenu", __LINE__);

    if (pSubMenuSub != NULL) {
        pSubMenuSub->pParentMenu = pSubMenu;
        if (pSubMenuSub->pfMenuGetDefaultVal != NULL)
            pSubMenuSub->iCurrentPos = pSubMenuSub->pfMenuGetDefaultVal(pSubMenuSub);
        else
            pSubMenuSub->iCurrentPos = 0;
        if (pCurItem->iItemId == ITEMID_DELETE_ONE) {
            pCurItem->pSubMenu->iCurrentPos = 0;
            mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_MSG_SURE_TO_DELETE_SELETED));
            mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
            mSubMenuConfirmPtr->setVisible(true);
            mSubMenuConfirm_yesPtr->setSelected(true);
            mSubMenuConfirm_noPtr->setSelected(false);
        } else if (pCurItem->iItemId == ITEMID_PROTECT_ONE) {
            pCurItem->pSubMenu->iCurrentPos = 0;
            mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_MSG_SURE_TO_PROTECT_SELETED));
            mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
            mSubMenuConfirmPtr->setVisible(true);
            mSubMenuConfirm_yesPtr->setSelected(true);
            mSubMenuConfirm_noPtr->setSelected(false);
        } else if (pCurItem->iItemId == ITEMID_UNPROTECT_ONE) {
            pCurItem->pSubMenu->iCurrentPos = 0;
            mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_MSG_SURE_TO_UNPROTECT_SELETED));
            mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
            mSubMenuConfirmPtr->setVisible(true);
            mSubMenuConfirm_yesPtr->setSelected(true);
            mSubMenuConfirm_noPtr->setSelected(false);
        } else
            msubMenuListSubViewPtr->setVisible(true);
    } else {
        if (pCurItem->pfItemSelectHandler != NULL) {
            bool bRet = false;
            bRet = pCurItem->pfItemSelectHandler(pCurItem);
            if (bRet == true) {
            	pListView->setSelection(0);		//reset position//msubMenuListViewPtr->setSelection(0);
                pListView->setVisible(false);
                //printf("******setVisible(false) %s:reset position, Line [%d]\n",__FUNCTION__, __LINE__);
                mMenuSlideBarPtr->setVisible(true);
            }
        }
    }
}
static void onProgressChanged_AdjustParam(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged AdjustParam %d !!!\n", progress);
    //mSetParamPtr->setText(progress);
    //pSeekBar->setProgress(progress);
}

static bool onButtonClick_DecParam(ZKButton *pButton) {
    //LOGD(" ButtonClick DecParam !!!\n");
    int iPreprogress = mAdjustParamPtr->getProgress();
    mAdjustParamPtr->setProgress(((iPreprogress - 10)>0?(iPreprogress - 10):0));
    mSetParamPtr->setText(((iPreprogress - 10)>0?(iPreprogress - 10):0));
    if (pCurItem->pSubMenu->pItemsList[0]->pfItemSelectHandler != NULL) {
        pCurItem->pSubMenu->pItemsList[0]->pfItemSelectHandler(pCurItem->pSubMenu->pItemsList[0]);
    }
    return false;
}

static bool onButtonClick_IncParam(ZKButton *pButton) {
    //LOGD(" ButtonClick IncParam !!!\n");
    int iPreprogress = mAdjustParamPtr->getProgress();
    mAdjustParamPtr->setProgress(((iPreprogress + 10)>100?100:iPreprogress + 10));
    mSetParamPtr->setText(((iPreprogress + 10)>100?100:iPreprogress + 10));
    if (pCurItem->pSubMenu->pItemsList[1]->pfItemSelectHandler != NULL) {
        pCurItem->pSubMenu->pItemsList[1]->pfItemSelectHandler(pCurItem->pSubMenu->pItemsList[1]);
    }
    return false;
}

static int Check_Validate_ClockSetting(int* pDatetime, int ubCheckType)
{
    int Year,Month,Day,Hour,Min,Sec;

    Year  = *(pDatetime+IDX_YEAR);
    Month = *(pDatetime+IDX_MONTH);
    Day   = *(pDatetime+IDX_DAY);
    Hour  = *(pDatetime+IDX_HOUR);
    Min   = *(pDatetime+IDX_MIN);
    Sec   = *(pDatetime+IDX_SEC);

    //Check Year
    if (ubCheckType & CHECK_YEAR) {
        if (Year > RTC_MAX_YEAR) {
            *(pDatetime+IDX_YEAR) = RTC_MIN_YEAR;
        } else if (Year < RTC_MIN_YEAR) {
            *(pDatetime+IDX_YEAR) = RTC_MAX_YEAR;
        }
        if (((Year %4 == 0) && (Year %100 != 0)) || (Year %400 == 0)) {//Leap Year
            if (Month == 2 && Day == 28) {
                *(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_LEAP_YEAR;
            }
        } else {
            if (Month == 2 && Day == 29) {
                *(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_NONLEAP_YEAR;
            }
        }
    }
    //Check Month
    if (ubCheckType & CHECK_MONTH) {
        if (Month > RTC_MAX_MONTH) {
            *(pDatetime+IDX_MONTH) = RTC_MIN_MONTH;
        } else if (Month < RTC_MIN_MONTH) {
            *(pDatetime+IDX_MONTH) = RTC_MAX_MONTH;
        }
        if (Month == 2) {
            if (((Year %4 == 0) && (Year %100 != 0)) || (Year %400 == 0)) {//Leap Year
                if (Day == 30 || Day == 31) {
                    *(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_LEAP_YEAR;
                }
            } else {
                if (Day == 29 || Day == 30 || Day == 31) {
                    *(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_NONLEAP_YEAR;
                }
            }
        } else if (Month == 4 || Month == 6 || Month == 9 || Month == 11) {
            if (Day == 31) {
                *(pDatetime+IDX_DAY) = RTC_MAX_DAY_30;
            }
        }
    }
    //Check Day
    if (ubCheckType & CHECK_DAY) {
        if (Month==1  || Month==3 || Month==5 || Month==7 || Month==8 || Month==10 || Month==12) {
            if (Day >RTC_MAX_DAY_31) {
                *(pDatetime+IDX_DAY) = RTC_MIN_DAY;
            } else if (Day < RTC_MIN_DAY) {
                *(pDatetime+IDX_DAY) = RTC_MAX_DAY_31;
            }
        }

        if (Month==4 || Month==6 || Month==9 || Month==11) {
            if (Day >RTC_MAX_DAY_30) {
                *(pDatetime+IDX_DAY) = RTC_MIN_DAY;
            } else if (Day < RTC_MIN_DAY) {
                    *(pDatetime+IDX_DAY) = RTC_MAX_DAY_30;
            }
        }

        if (Month==2) {
            if (((Year %4 == 0) && (Year %100 != 0)) || (Year %400 == 0)) {//Leap Year
                if (Day >RTC_MAX_DAY_FEB_LEAP_YEAR) {
                    *(pDatetime+IDX_DAY) = RTC_MIN_DAY;
                } else if (Day < RTC_MIN_DAY) {
                    *(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_LEAP_YEAR;
                }
            } else {
                if (Day >RTC_MAX_DAY_FEB_NONLEAP_YEAR) {
                    *(pDatetime+IDX_DAY) = RTC_MIN_DAY;
                } else if (Day < RTC_MIN_DAY) {
                    *(pDatetime+IDX_DAY) = RTC_MAX_DAY_FEB_NONLEAP_YEAR;
                }
            }
        }
    }
     //Check Hour
    if (ubCheckType & CHECK_HOUR) {
        if (Hour > RTC_MAX_HOUR) {
            *(pDatetime+IDX_HOUR) = RTC_MIN_HOUR;
        } else if (Hour < RTC_MIN_HOUR) {
            *(pDatetime+IDX_HOUR) = RTC_MAX_HOUR;
        }
    }
    //Check Minute
    if (ubCheckType & CHECK_MIN) {
        if (Min > RTC_MAX_MIN) {
            *(pDatetime+IDX_MIN) = RTC_MIN_MIN;
        } else if (Min < RTC_MIN_MIN) {
            *(pDatetime+IDX_MIN) = RTC_MAX_MIN;
        }
    }
    //Check Second
    if (ubCheckType & CHECK_SEC) {
        if (Sec > RTC_MAX_SEC) {
            *(pDatetime+IDX_SEC) = RTC_MIN_SEC;
        } else if (Sec < RTC_MIN_SEC) {
            *(pDatetime+IDX_SEC) = RTC_MAX_SEC;
        }
    }
    return CHECK_PASS;
}

int SetRtcTime(int* pDatetime)
{
    struct tm RtcTm;
    struct timeval tv;
    time_t timep;

    RtcTm.tm_sec = *(pDatetime+IDX_SEC);
    RtcTm.tm_min = *(pDatetime+IDX_MIN);
    RtcTm.tm_hour = *(pDatetime+IDX_HOUR);
    RtcTm.tm_mday = *(pDatetime+IDX_DAY);
    RtcTm.tm_mon = *(pDatetime+IDX_MONTH) - 1;    // month of year [0,11]
    RtcTm.tm_year = *(pDatetime+IDX_YEAR) - 1900;
    RtcTm.tm_isdst = 0;

    timep = mktime(&RtcTm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    if (settimeofday(&tv, (struct timezone *) 0) < 0) {
        printf("Set rtc datatime error!\n");
        return -1;
    }
    return 0;
}

static bool onButtonClick_clocksetting_add(ZKButton *pButton) {
    //LOGD(" ButtonClick clocksetting_add !!!\n");
    string str = "";
    if (myear_valuePtr->isSelected()) {
        str = myear_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_YEAR];
        Datetime[IDX_YEAR] += 1;
        Check_Validate_ClockSetting(Datetime, CHECK_YEAR);
        myear_valuePtr->setText(Datetime[IDX_YEAR]);
    } else if (mmon_valuePtr->isSelected()) {
        str = mmon_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_MONTH];
        Datetime[IDX_MONTH] += 1;
        Check_Validate_ClockSetting(Datetime, CHECK_MONTH);
        mmon_valuePtr->setText(Datetime[IDX_MONTH]);
    } else if (mday_valuePtr->isSelected()) {
        str = mday_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_DAY];
        Datetime[IDX_DAY] += 1;
        Check_Validate_ClockSetting(Datetime, CHECK_DAY);
        mday_valuePtr->setText(Datetime[IDX_DAY]);
    } else if (mhour_valuePtr->isSelected()) {
        str = mhour_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_HOUR];
        Datetime[IDX_HOUR] += 1;
        Check_Validate_ClockSetting(Datetime, CHECK_HOUR);
        mhour_valuePtr->setText(Datetime[IDX_HOUR]);
    } else if (mmin_valuePtr->isSelected()) {
        str = mmin_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_MIN];
        Datetime[IDX_MIN] += 1;
        Check_Validate_ClockSetting(Datetime, CHECK_MIN);
        mmin_valuePtr->setText(Datetime[IDX_MIN]);
    }
    return false;
}

static bool onButtonClick_clocksetting_del(ZKButton *pButton) {
    //LOGD(" ButtonClick clocksetting_del !!!\n");
    string str = "";
    if (myear_valuePtr->isSelected()) {
        str = myear_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_YEAR];
        Datetime[IDX_YEAR] -= 1;
        Check_Validate_ClockSetting(Datetime, CHECK_YEAR);
        myear_valuePtr->setText(Datetime[IDX_YEAR]);
    } else if (mmon_valuePtr->isSelected()) {
        str = mmon_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_MONTH];
        Datetime[IDX_MONTH] -= 1;
        Check_Validate_ClockSetting(Datetime, CHECK_MONTH);
        mmon_valuePtr->setText(Datetime[IDX_MONTH]);
    } else if (mday_valuePtr->isSelected()) {
        str = mday_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_DAY];
        Datetime[IDX_DAY] -= 1;
        Check_Validate_ClockSetting(Datetime, CHECK_DAY);
        mday_valuePtr->setText(Datetime[IDX_DAY]);
    } else if (mhour_valuePtr->isSelected()) {
        str = mhour_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_HOUR];
        Datetime[IDX_HOUR] -= 1;
        Check_Validate_ClockSetting(Datetime, CHECK_HOUR);
        mhour_valuePtr->setText(Datetime[IDX_HOUR]);
    } else if (mmin_valuePtr->isSelected()) {
        str = mmin_valuePtr->getText();
        istringstream(str)>>Datetime[IDX_MIN];
        Datetime[IDX_MIN] -= 1;
        Check_Validate_ClockSetting(Datetime, CHECK_MIN);
        mmin_valuePtr->setText(Datetime[IDX_MIN]);
    }
    return false;
}

static bool onButtonClick_clocksetting_ok(ZKButton *pButton) {
    //LOGD(" ButtonClick clocksetting_ok !!!\n");
    if (myear_valuePtr->isSelected()) {
        myear_valuePtr->setSelected(false);
        mmon_valuePtr->setSelected(true);
    } else if (mmon_valuePtr->isSelected()) {
        mmon_valuePtr->setSelected(false);
        mday_valuePtr->setSelected(true);
    } else if (mday_valuePtr->isSelected()) {
        mday_valuePtr->setSelected(false);
        mhour_valuePtr->setSelected(true);
    } else if (mhour_valuePtr->isSelected()) {
        mhour_valuePtr->setSelected(false);
        mmin_valuePtr->setSelected(true);
    } else if (mmin_valuePtr->isSelected()) {
        mmin_valuePtr->setSelected(false);
        myear_valuePtr->setSelected(true);
        SetRtcTime(Datetime);
        msubMenuClockSettingPtr->setVisible(false);
    }
    return false;
}

static bool onButtonClick_SubMenuConfirm_yes(ZKButton *pButton) {
    LOGD(" ButtonClick SubMenuConfirm_yes !!! %s, %d\n", MAP_STRINGID(pCurItem->pSubMenu->iStringId), pCurItem->pSubMenu->iCurrentPos);

    if (pCurItem->pSubMenu->pItemsList[0]->pfItemSelectHandler != NULL) {
        pCurItem->pSubMenu->pItemsList[0]->pfItemSelectHandler(pCurItem->pSubMenu->pItemsList[0]);
    }
    mSubMenuConfirmPtr->setVisible(false);
    return false;
}

static bool onButtonClick_SubMenuConfirm_no(ZKButton *pButton) {
    LOGD(" ButtonClick SubMenuConfirm_no !!! %s, %d\n", MAP_STRINGID(pCurItem->pSubMenu->iStringId), pCurItem->pSubMenu->iCurrentPos);

    if (pCurItem->pSubMenu->pItemsList[1]->pfItemSelectHandler != NULL){
        pCurItem->pSubMenu->pItemsList[1]->pfItemSelectHandler(pCurItem->pSubMenu->pItemsList[1]);
    }
    mSubMenuConfirmPtr->setVisible(false);
    return false;
}

static int getListItemCount_subMenuListSubView(const ZKListView *pListView) {
    //LOGD("getListItemCount_subMenuListSubView !\n");
    return pSubMenuSub->iNumOfItems;
}

static void obtainListItemData_subMenuListSubView(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ subMenuListSubView  !!!\n");
    ZKListView::ZKListSubItem* sub = NULL;

    mMenuStatusModePtr->setTextTr(MAP_STRINGID(pSubMenuSub->iStringId));
    pListView->setSelection(pSubMenuSub->iCurrentPos);
    sub = pListItem->findSubItemByID(ID_VIDEOMENU_SubMenuItem);
    if (sub) {
        PSMENUITEM pItem = pSubMenuSub->pItemsList[index];
        sub->setTextTr(MAP_STRINGID(pItem->iStringId));
        if (index == pSubMenuSub->iCurrentPos)
            sub->setSelected(true);
        else
            sub->setSelected(false);
    }
}

static void onListItemClick_subMenuListSubView(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ subMenuListSubView  !!!\n");
    pCurItem = pSubMenuSub->pItemsList[index];
    printf("set pCurItem [%s] Line [%d]\n", MAP_STRINGID(pCurItem->iStringId), __LINE__);

    if (pSubMenuSub->pfMenuGetDefaultVal != NULL)
        pSubMenuSub->iCurrentPos = pSubMenuSub->pfMenuGetDefaultVal(pSubMenuSub);
    else
        pSubMenuSub->iCurrentPos = 0;
    if (pCurItem->iItemId == ITEMID_DELETE_ALL_VIDEO || pCurItem->iItemId == ITEMID_DELETE_ALL_IMAGE) {
        pCurItem->pSubMenu->iCurrentPos = 0;
        mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_DELETE_ALL_CONFIRM));
        mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
        mSubMenuConfirmPtr->setVisible(true);
        mSubMenuConfirm_yesPtr->setSelected(true);
        mSubMenuConfirm_noPtr->setSelected(false);
    } else if (pCurItem->iItemId == ITEMID_PROTECT_ALL_VIDEO || pCurItem->iItemId == ITEMID_PROTECT_ALL_IMAGE) {
        pCurItem->pSubMenu->iCurrentPos = 0;
        mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_PROTECT_ALL_CONFIRM));
        mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
        mSubMenuConfirmPtr->setVisible(true);
        mSubMenuConfirm_yesPtr->setSelected(true);
        mSubMenuConfirm_noPtr->setSelected(false);
    } else if (pCurItem->iItemId == ITEMID_UNPROTECT_ALL_VIDEO || pCurItem->iItemId == ITEMID_UNPROTECT_ALL_IMAGE) {
        pCurItem->pSubMenu->iCurrentPos = 0;
        mSubMenuConfirm_text1Ptr->setTextTr(MAP_STRINGID(IDS_DS_UNPROTECT_ALL));
        mSubMenuConfirm_text2Ptr->setTextTr(MAP_STRINGID(IDS_DS_EMPTY));
        mSubMenuConfirmPtr->setVisible(true);
        mSubMenuConfirm_yesPtr->setSelected(true);
        mSubMenuConfirm_noPtr->setSelected(false);
    }
}

static void MenuShowSDCardInfoPage(void)
{
    float fTotalSize, fAvalSize, fUsedSize, fFreeSpace;
    char strSize[20] = {0};

    IPC_CarInfo_Read_SdInfo(&carimpl.stSdInfo);
    fTotalSize = (float)carimpl.stSdInfo.u64TotalSize/1024/1024/1024;
    fAvalSize = (float)carimpl.stSdInfo.u64ReservedSpace/1024/1024/1024;
    fFreeSpace = (float)carimpl.stSdInfo.u64FreeSize/1024/1024/1024;
    fUsedSize = fAvalSize - fFreeSpace;
    sprintf(strSize, "%.1f GB", fTotalSize);
    mSDInfo_TotalSize_valuePtr->setText(strSize);
    sprintf(strSize, "%.1f GB", fAvalSize);
    mSDInfo_AvaliableSize_valuePtr->setText(strSize);
    sprintf(strSize, "%.1f GB", fUsedSize);
    mSDInfo_Used_valuePtr->setText(strSize);
    mSDCardInfo_CircleBarPtr->setProgress(fFreeSpace * 100 / fAvalSize);
    mSDCardInfoPtr->setVisible(true);
}
