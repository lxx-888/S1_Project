#pragma once
#include "uart/ProtocolSender.h"
#include "utils/TimeHelper.h"
#include "carimpl/carimpl.h"
#include "carimpl/KeyEventContext.h"
#include <signal.h>

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

#define FB_WIDTH    896
#define FB_HEIGHT   512

#define LINE_WIDTH  2
#define COLOR_CLEAR 0x000000
#define COLOR_WHITE 0xFFFFFF
#define COLOR_RED   0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE  0x0000FF

#define SYNC_DATA_DUR	10	//Unit:SEC
#define VIDEO_TIMER_UNIT  1     //Uint:SEC
#define DURATION_OFFSET   10   //Unit:US

static bool onButtonClick_BTN_Rec(ZKButton *pButton);
static bool onButtonClick_BTN_Cap(ZKButton *pButton);
static bool onButtonClick_BTN_WIFI(ZKButton *pButton);
static bool onButtonClick_BTN_AudioMute(ZKButton *pButton);
static void VideoTimer_Start(int iSec);
static void VideoTimer_Stop(void);


#ifdef SUPPORT_CARLIFE
extern int carimpl_CarLifeLaunch(void);
extern int carimpl_CarLifeSendKey(int keycode, bool isDown);
extern bool carimpl_CarLifeSendTouchEvt(int x, int y, int mode);
extern bool carimpl_CarLifeIsCnnt(void);
extern bool carimpl_CarLifeIsShow(void);
extern bool carimpl_CarLifeSwitch(void);
#endif


/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	{0,  500}, //定时器id=0, 时间间隔500ms
	{1,  30}, // For ADAS Drawing
};

static bool bSdStatus = FALSE;
static int adas_width = 0;
static int adas_height = 0;
static bool bAdasCalibDraw = FALSE;
SZKPoint pAdasLdwsLPoints[2] = {0, 0};
SZKPoint pAdasLdwsRPoints[2] = {0, 0};
int adasFcwsLeft = 0, adasFcwsTop = 0, adasFcwsWidth = 0, adasFcwsHeight = 0;

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
	IPC_CarInfo_Open();
	IPC_CarInfo_Read(&carimpl);
	VideoTimer_Start(VIDEO_TIMER_UNIT);
	IPC_MsgToUI_Init();
	IPC_MsgToUI_RegisterMsgHandler(carimpl_msg_handler);
	IPC_MsgToUI_CreateThread();
	LOGD(" main onUI_init!\n");
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
    // LOGD(" main onUI_intent!\n");
}

void onVideoModeShowIcon(bool show)
{
	static bool bstatus;
	if(bstatus == show)
		return;
		
	if(show){
		//mPainterAdasPtr->setVisible(TRUE);
		mTxtDateTimePtr->setVisible(TRUE);
		mTextLockPtr->setVisible(TRUE);
		if (IMPL_EMERG_REC_STATUS)
			mTextLockPtr->setVisible(TRUE);
		mBTN_RecPtr->setVisible(TRUE);
		mTxtRecIconPtr->setVisible(TRUE);
		mTxtSdStatusPtr->setVisible(TRUE);
		mTxtRecStatusPtr->setVisible(TRUE);
		mTxtChgStuPtr->setVisible(TRUE);
		mTxtResInfoPtr->setVisible(TRUE);
		mTxtEvValPtr->setVisible(TRUE);
		mDuration_TXTview2Ptr->setVisible(TRUE);
		//mBTN_WIFIPtr->setVisible(TRUE);
	}
	else{
		//mPainterAdasPtr->setVisible(FALSE);
		mTxtDateTimePtr->setVisible(FALSE);
		mTextLockPtr->setVisible(FALSE);
		mTextLockPtr->setVisible(FALSE);
		mBTN_RecPtr->setVisible(FALSE);
		mTxtRecIconPtr->setVisible(FALSE);
		mTxtSdStatusPtr->setVisible(FALSE);
		mTxtRecStatusPtr->setVisible(FALSE);
		mTxtChgStuPtr->setVisible(FALSE);
		mTxtResInfoPtr->setVisible(FALSE);
		mTxtEvValPtr->setVisible(FALSE);
		mDuration_TXTview2Ptr->setVisible(FALSE);
		//mBTN_WIFIPtr->setVisible(FALSE);
	}
	bstatus = show;
}

static void on_key_callback(int keyCode, int keyStatus) {
	printf("[Video] key [%s] %s\n", KEYCODE_TO_STRING(keyCode), KEYSTATUS_TO_STRING(keyStatus));

	#ifdef SUPPORT_CARLIFE
	if(carimpl_CarLifeIsCnnt()){
		int ret = carimpl_CarLifeSendKey(keyCode,keyStatus);

		if(ret == 0){
			return;
		}
		else if(ret == 1){
			onVideoModeShowIcon(FALSE);
			return;
		}
		else if(ret == 2){
			onVideoModeShowIcon(TRUE);
			return;
		}
	}
	#endif
	
	switch (keyCode) {
	case KEY_ENTER:
		if (keyStatus == UI_KEY_RELEASE)
			onButtonClick_BTN_Rec(mBTN_RecPtr);
		if (keyStatus == UI_KEY_LPRESS)
			onButtonClick_BTN_Cap(NULL);
	break;
	case KEY_RIGHT:
		if (keyStatus == UI_KEY_RELEASE) {
		#ifdef PLAYING_AND_RECORDING
				gu8LastUIMode = UI_VIDEO_MODE;
				//carimpl_VideoH26xInit(FALSE);
				EASYUICONTEXT->openActivity("captureActivity");
		#else
			if (IMPL_REC_STATUS == FALSE &&
				IMPL_EMERG_REC_STATUS == FALSE &&
				IMPL_MD_REC_STATUS == FALSE) {
				gu8LastUIMode = UI_VIDEO_MODE;
				carimpl_VideoH26xInit(FALSE);
				EASYUICONTEXT->openActivity("captureActivity");
			} else if (!IMPL_EMERG_REC_STATUS) {
				carimpl_VideoFunc_StartEmergRecording();
				mTextLockPtr->setVisible(TRUE);
			}
		#endif
		}
	break;
	case KEY_LEFT:
		if (keyStatus == UI_KEY_RELEASE)
			onButtonClick_BTN_AudioMute(NULL);
		break;
	case KEY_DOWN:
		if (keyStatus == UI_KEY_RELEASE)
			carimpl_GeneralFunc_SetPreviewCamIdx();
		break;
	case KEY_UP:
		if (keyStatus == UI_KEY_RELEASE){
			#ifdef SUPPORT_CARLIFE
			carimpl_GeneralFunc_SetCarLifeOnOff();
			carimpl_CarLifeLaunch();
			#else
			onButtonClick_BTN_WIFI(mBTN_WIFIPtr);
			#endif
		}
		break;
	case KEY_MENU:
    case KEY_MENU_2:
		if (keyStatus == UI_KEY_RELEASE &&
			IMPL_REC_STATUS == FALSE &&
			IMPL_EMERG_REC_STATUS == FALSE &&
			IMPL_MD_REC_STATUS == FALSE) {
			gu8LastUIMode = UI_VIDEO_MODE;
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

static void VideoShowLock(void)
{
	if(carimpl.stRecInfo.bMuxingEmerg){
		mTextLockPtr->setVisible(TRUE);
	}
	else{
		mTextLockPtr->setVisible(FALSE);
	}
}

static void VideoShowMovieRes(void)
{
	switch (MenuSettingConfig()->uiMOVSize)
	{
	#if (MENU_MOVIE_SIZE_4K_25P_EN)
	case MOVIE_SIZE_4K_25P:
		mTxtResInfoPtr->setText("4K");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_1440_30P_EN)
	case MOVIE_SIZE_1440_30P:
		mTxtResInfoPtr->setText("1440P");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_SHD_30P_EN)
	case MOVIE_SIZE_SHD_30P:
		mTxtResInfoPtr->setText("SHD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_SHD_25P_EN)
	case MOVIE_SIZE_SHD_25P:
		mTxtResInfoPtr->setText("SHD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_1080_60P_EN)
	case MOVIE_SIZE_1080_60P:
		mTxtResInfoPtr->setText("FHD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_1080P_EN)
	case MOVIE_SIZE_1080P:
		mTxtResInfoPtr->setText("FHD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
	case MOVIE_SIZE_1080_30P_HDR:
		mTxtResInfoPtr->setText("FHD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_900P_30P_EN)
	case MOVIE_SIZE_900P_30P:
		mTxtResInfoPtr->setText("900P");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_960P_30P_EN)
	case MOVIE_SIZE_960P_30P:
		mTxtResInfoPtr->setText("960P");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_720P_EN)
	case MOVIE_SIZE_720P:
		mTxtResInfoPtr->setText("HD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_720_60P_EN)
	case MOVIE_SIZE_720_60P:
		mTxtResInfoPtr->setText("HD");
		break;
	#endif
	#if (MENU_MOVIE_SIZE_VGA30P_EN)
	case MOVIE_SIZE_VGA_30P:
		mTxtResInfoPtr->setText("VGA");
		break;
	#endif
	default:
		mTxtResInfoPtr->setText("FHD");
		break;
	}
}

static void VideoShowEv(void)
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

static void VideoShowAudioStatus(void)
{
	#ifdef SUPPORT_CARLIFE
	if(!carimpl_CarLifeIsShow())
	#endif
	{
		switch (MenuSettingConfig()->uiMOVSoundRecord)
		{
		#if (MENU_MOVIE_SOUND_RECORD_ON_EN)
		case MOVIE_SOUND_RECORD_ON:
			mTxtAudioStatusPtr->setBackgroundPic("cardv/video_btn_mic_on.png");
			break;
		#endif
		#if (MENU_MOVIE_SOUND_RECORD_OFF_EN)
		case MOVIE_SOUND_RECORD_OFF:
			mTxtAudioStatusPtr->setBackgroundPic("cardv/video_btn_mic_off.png");
			break;
		#endif
		default:
			mTxtAudioStatusPtr->setBackgroundPic("cardv/video_btn_mic_on.png");
			break;
		}
	}
}

static void VideoShowRecStatus(void)
{
	char timestr[20] = "00:00:00";
	int duration;
	static int cnt = 0;
	if (IMPL_REC_STATUS || IMPL_EMERG_REC_STATUS || IMPL_MD_REC_STATUS) {
		if (IMPL_REC_STATUS || IMPL_MD_REC_STATUS)
			duration = (IMPL_REC_DURATION + DURATION_OFFSET) / 1000000;
		else
			duration = (IMPL_EMERG_REC_DURATION + DURATION_OFFSET) / 1000000;

		sprintf(timestr, "%02d:%02d:%02d", duration / 3600, (duration % 3600) / 60, duration % 60);
		mDuration_TXTview2Ptr->setTextColor(COLOR_RED);
		mDuration_TXTview2Ptr->setText(timestr);
		mTxtRecStatusPtr->setText("REC");
		if (cnt++ % 2 == 0) {
			mTxtRecIconPtr->setBackgroundPic("cardv/video_icon_rec_dot.png");
		} else {
			mTxtRecIconPtr->setBackgroundPic("");
		}
	} else {
		mDuration_TXTview2Ptr->setTextColor(COLOR_WHITE);
		mDuration_TXTview2Ptr->setText("00:00:00");
		mTxtRecStatusPtr->setText("STBY");
		mTxtRecIconPtr->setBackgroundPic("");
	}
}

static void VideoShowRtc(void)
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

	if (MenuSettingConfig()->uiDateTimeFormat == DATETIME_SETUP_NONE){
		mTxtDateTimePtr->setVisible(false);
	}
	else{
		#ifdef SUPPORT_CARLIFE
		if(!carimpl_CarLifeIsShow())
		#endif
	    	mTxtDateTimePtr->setVisible(true);
    }

	mTxtDateTimePtr->setText(timeStr);
}

static void VideoShowWifiStatus(NET_STATUS_TYPE eNetStatus)
{
	if (eNetStatus == NET_STATUS_READY) {
		//AP mode: get ready flag after cnnt
		#ifdef SUPPORT_CARLIFE
		if(carimpl_CarLifeIsCnnt()){
			mBTN_WIFIPtr->setBackgroundPic("main_/sp.png");
		}else
		#endif
		mBTN_WIFIPtr->setBackgroundPic("wifi/wifi_signal_5.png");
	} else if (eNetStatus == NET_STATUS_WEAK) {
		mBTN_WIFIPtr->setBackgroundPic("wifi/wifi_signal_2.png");
	} else if (eNetStatus == NET_STATUS_UP) {
		//TODO:		STA mode: get STATUS_UP flag after cnnt
		#ifdef SUPPORT_CARLIFE
		if(carimpl_CarLifeIsCnnt()){
			mBTN_WIFIPtr->setBackgroundPic("main_/sp.png");
		}else
		#endif
		mBTN_WIFIPtr->setBackgroundPic("wifi/wifi_signal_4.png");
	} else {
		mBTN_WIFIPtr->setBackgroundPic("wifi/wifi_signal_0.png");
	}
}

static void VideoStateUpdate(void)
{
	VideoShowRecStatus();
	VideoShowRtc();
	VideoShowAudioStatus();
	VideoShowWifiStatus(check_wifi_status());
	VideoShowMovieRes();
	VideoShowLock();
	VideoShowEv();
	SHOW_SD_ICON(mTxtSdStatusPtr, bSdStatus);
	SHOW_BAT_ICON(mTxtChgStuPtr);
}

static void DrawAdasCalibration(bool bDraw)
{
	IPC_CarInfo_Read_AdasInfo(&carimpl.stAdasInfo);
	if (carimpl.stAdasInfo.bCalibVaild) {
		// Draw calibration.
		SZKPoint pPoints[4];
		adas_width = carimpl.stAdasInfo.stCalibInfo.laneCalibrationImageWidth;
		adas_height = carimpl.stAdasInfo.stCalibInfo.laneCalibrationImageHeight;
		pPoints[0].x = carimpl.stAdasInfo.stCalibInfo.laneCalibrationUpPointX * FB_WIDTH / adas_width;
		pPoints[0].y = carimpl.stAdasInfo.stCalibInfo.laneCalibrationUpPointY * FB_HEIGHT / adas_height;
		pPoints[1].x = carimpl.stAdasInfo.stCalibInfo.laneCalibrationLeftPointX * FB_WIDTH / adas_width;
		pPoints[1].y = carimpl.stAdasInfo.stCalibInfo.laneCalibrationLeftPointY * FB_HEIGHT / adas_height;
		pPoints[2].x = carimpl.stAdasInfo.stCalibInfo.laneCalibrationRightPointX * FB_WIDTH / adas_width;
		pPoints[2].y = carimpl.stAdasInfo.stCalibInfo.laneCalibrationRightPointY * FB_HEIGHT / adas_height;
		pPoints[3].x = pPoints[0].x;
		pPoints[3].y = pPoints[0].y;
		if (bAdasCalibDraw == FALSE && bDraw) {
			mPainterAdasPtr->setLineWidth(LINE_WIDTH);
			mPainterAdasPtr->setSourceColor(COLOR_WHITE);
			mPainterAdasPtr->drawLines(pPoints, 4);
			bAdasCalibDraw = TRUE;
		} else if (bAdasCalibDraw && bDraw == FALSE) {
			int left, top, right, bottom;
			left   = pPoints[1].x - LINE_WIDTH;
			top    = pPoints[0].y - LINE_WIDTH;
			right  = pPoints[2].x + LINE_WIDTH;
			bottom = MIN(pPoints[1].y + LINE_WIDTH, pPoints[2].y + LINE_WIDTH);
			mPainterAdasPtr->erase(left, top, right - left, bottom - top);
			bAdasCalibDraw = FALSE;
		}
	}
}

static void DrawAdasInfo(void)
{
	//RECT Rect;
	int left, top, right, bottom;

	if (MenuSettingConfig()->uiLDWS == LDWS_EN_OFF &&
	    MenuSettingConfig()->uiFCWS == FCWS_EN_OFF) {
		mPainterAdasPtr->setVisible(FALSE);
		return;
	}

	if (!mPainterAdasPtr->isVisible())
		mPainterAdasPtr->setVisible(TRUE);

	IPC_CarInfo_Read_AdasInfo(&carimpl.stAdasInfo);
	if (!carimpl.stAdasInfo.bLdwsVaild &&
	    !carimpl.stAdasInfo.bFcwsVaild)
		DrawAdasCalibration(TRUE);
	else
		DrawAdasCalibration(FALSE); // TBD : When clear calibration info ?

	if (carimpl.stAdasInfo.bLdwsVaild) {
		// printf("[UI][LDWS] State [%d] Left [%03d, %03d] to [%03d, %03d] Right [%03d, %03d] to [%03d, %03d]\n",
		// 		carimpl.stAdasInfo.stLdwsInfo.state,
		// 		carimpl.stAdasInfo.stLdwsInfo.left_lane[0].x,
		// 		carimpl.stAdasInfo.stLdwsInfo.left_lane[0].y,
		// 		carimpl.stAdasInfo.stLdwsInfo.left_lane[1].x,
		// 		carimpl.stAdasInfo.stLdwsInfo.left_lane[1].y,
		// 		carimpl.stAdasInfo.stLdwsInfo.right_lane[0].x,
		// 		carimpl.stAdasInfo.stLdwsInfo.right_lane[0].y,
		// 		carimpl.stAdasInfo.stLdwsInfo.right_lane[1].x,
		// 		carimpl.stAdasInfo.stLdwsInfo.right_lane[1].y);
		if (carimpl.stAdasInfo.stLdwsInfo.left_lane[0].x > 0 &&
		    carimpl.stAdasInfo.stLdwsInfo.left_lane[0].y > 0 &&
			carimpl.stAdasInfo.stLdwsInfo.left_lane[1].x > 0 &&
			carimpl.stAdasInfo.stLdwsInfo.left_lane[1].y > 0 &&
			carimpl.stAdasInfo.stLdwsInfo.right_lane[0].x > 0 &&
			carimpl.stAdasInfo.stLdwsInfo.right_lane[0].y > 0 &&
			carimpl.stAdasInfo.stLdwsInfo.right_lane[1].x > 0 &&
			carimpl.stAdasInfo.stLdwsInfo.right_lane[1].y > 0) {
			// Draw Ldws
			SZKPoint pPoints[2];
			int color;

			pPoints[0].x = carimpl.stAdasInfo.stLdwsInfo.left_lane[0].x * FB_WIDTH / adas_width;
			pPoints[0].y = carimpl.stAdasInfo.stLdwsInfo.left_lane[0].y * FB_HEIGHT / adas_height;
			pPoints[1].x = carimpl.stAdasInfo.stLdwsInfo.left_lane[1].x * FB_WIDTH / adas_width;
			pPoints[1].y = carimpl.stAdasInfo.stLdwsInfo.left_lane[1].y * FB_HEIGHT / adas_height;
			if (pPoints[0].x != pAdasLdwsLPoints[0].x || pPoints[1].x != pAdasLdwsLPoints[1].x ||
				pPoints[0].y != pAdasLdwsLPoints[0].y || pPoints[1].y != pAdasLdwsLPoints[1].y) {

				left   = MIN(pAdasLdwsLPoints[0].x - LINE_WIDTH, pAdasLdwsLPoints[1].x - LINE_WIDTH);
				top    = MIN(pAdasLdwsLPoints[0].y - LINE_WIDTH, pAdasLdwsLPoints[1].y - LINE_WIDTH);
				right  = MAX(pAdasLdwsLPoints[0].x + LINE_WIDTH, pAdasLdwsLPoints[1].x + LINE_WIDTH);
				bottom = MAX(pAdasLdwsLPoints[0].y + LINE_WIDTH, pAdasLdwsLPoints[1].y + LINE_WIDTH);
				mPainterAdasPtr->erase(left, top, right - left, bottom - top);

				if (carimpl.stAdasInfo.stLdwsInfo.state == LDWS_STATE_DEPARTURE_LEFT)
					color = COLOR_RED;
				else
					color = COLOR_BLUE;
				mPainterAdasPtr->setLineWidth(LINE_WIDTH);
				mPainterAdasPtr->setSourceColor(color);
				mPainterAdasPtr->drawLines(pPoints, 2);
				pAdasLdwsLPoints[0] = pPoints[0];
				pAdasLdwsLPoints[1] = pPoints[1];
			}

			pPoints[0].x = carimpl.stAdasInfo.stLdwsInfo.right_lane[0].x * FB_WIDTH / adas_width;
			pPoints[0].y = carimpl.stAdasInfo.stLdwsInfo.right_lane[0].y * FB_HEIGHT / adas_height;
			pPoints[1].x = carimpl.stAdasInfo.stLdwsInfo.right_lane[1].x * FB_WIDTH / adas_width;
			pPoints[1].y = carimpl.stAdasInfo.stLdwsInfo.right_lane[1].y * FB_HEIGHT / adas_height;
			if (pPoints[0].x != pAdasLdwsRPoints[0].x || pPoints[1].x != pAdasLdwsRPoints[1].x ||
				pPoints[0].y != pAdasLdwsRPoints[0].y || pPoints[1].y != pAdasLdwsRPoints[1].y) {

				left   = MIN(pAdasLdwsRPoints[0].x - LINE_WIDTH, pAdasLdwsRPoints[1].x - LINE_WIDTH);
				top    = MIN(pAdasLdwsRPoints[0].y - LINE_WIDTH, pAdasLdwsRPoints[1].y - LINE_WIDTH);
				right  = MAX(pAdasLdwsRPoints[0].x + LINE_WIDTH, pAdasLdwsRPoints[1].x + LINE_WIDTH);
				bottom = MAX(pAdasLdwsRPoints[0].y + LINE_WIDTH, pAdasLdwsRPoints[1].y + LINE_WIDTH);
				mPainterAdasPtr->erase(left, top, right - left, bottom - top);

				if (carimpl.stAdasInfo.stLdwsInfo.state == LDWS_STATE_DEPARTURE_RIGHT)
					color = COLOR_RED;
				else
					color = COLOR_BLUE;
				mPainterAdasPtr->setLineWidth(LINE_WIDTH);
				mPainterAdasPtr->setSourceColor(color);
				mPainterAdasPtr->drawLines(pPoints, 2);
				pAdasLdwsRPoints[0] = pPoints[0];
				pAdasLdwsRPoints[1] = pPoints[1];
			}
		}
	} else {
		left   = MIN(pAdasLdwsLPoints[0].x - LINE_WIDTH, pAdasLdwsLPoints[1].x - LINE_WIDTH);
		top    = MIN(pAdasLdwsLPoints[0].y - LINE_WIDTH, pAdasLdwsLPoints[1].y - LINE_WIDTH);
		right  = MAX(pAdasLdwsLPoints[0].x + LINE_WIDTH, pAdasLdwsLPoints[1].x + LINE_WIDTH);
		bottom = MAX(pAdasLdwsLPoints[0].y + LINE_WIDTH, pAdasLdwsLPoints[1].y + LINE_WIDTH);
		mPainterAdasPtr->erase(left, top, right - left, bottom - top);
		pAdasLdwsLPoints[0].x = 0;
		pAdasLdwsLPoints[0].y = 0;
		pAdasLdwsLPoints[1].x = 0;
		pAdasLdwsLPoints[1].y = 0;
		left   = MIN(pAdasLdwsRPoints[0].x - LINE_WIDTH, pAdasLdwsRPoints[1].x - LINE_WIDTH);
		top    = MIN(pAdasLdwsRPoints[0].y - LINE_WIDTH, pAdasLdwsRPoints[1].y - LINE_WIDTH);
		right  = MAX(pAdasLdwsRPoints[0].x + LINE_WIDTH, pAdasLdwsRPoints[1].x + LINE_WIDTH);
		bottom = MAX(pAdasLdwsRPoints[0].y + LINE_WIDTH, pAdasLdwsRPoints[1].y + LINE_WIDTH);
		mPainterAdasPtr->erase(left, top, right - left, bottom - top);
		pAdasLdwsRPoints[0].x = 0;
		pAdasLdwsRPoints[0].y = 0;
		pAdasLdwsRPoints[1].x = 0;
		pAdasLdwsRPoints[1].y = 0;
	}

	if (carimpl.stAdasInfo.bFcwsVaild) {
		// printf("[UI][FCWS] State [%d] Pos [%03d, %03d] WH [%03d, %03d] Distance [%03d]\n",
		// 		carimpl.stAdasInfo.stFcwsInfo.state,
		// 		carimpl.stAdasInfo.stFcwsInfo.car_x,
		// 		carimpl.stAdasInfo.stFcwsInfo.car_y,
		// 		carimpl.stAdasInfo.stFcwsInfo.car_width,
		// 		carimpl.stAdasInfo.stFcwsInfo.car_height,
		// 		carimpl.stAdasInfo.stFcwsInfo.distance);
		int left, top, width, height;
		left   = carimpl.stAdasInfo.stFcwsInfo.car_x * FB_WIDTH / adas_width;
		top    = carimpl.stAdasInfo.stFcwsInfo.car_y * FB_HEIGHT / adas_height;
		width  = carimpl.stAdasInfo.stFcwsInfo.car_width * FB_WIDTH / adas_width;
		height = carimpl.stAdasInfo.stFcwsInfo.car_height * FB_HEIGHT / adas_height;
		if (adasFcwsLeft   != left  ||
			adasFcwsTop    != top   ||
			adasFcwsWidth  != width ||
			adasFcwsHeight != height) {
			mPainterAdasPtr->setLineWidth(LINE_WIDTH);
			mPainterAdasPtr->setSourceColor(COLOR_CLEAR);
			if (adasFcwsWidth != 0 && adasFcwsHeight != 0)
				mPainterAdasPtr->drawRect(adasFcwsLeft, adasFcwsTop, adasFcwsWidth, adasFcwsHeight, 0);
			mPainterAdasPtr->setLineWidth(LINE_WIDTH);
			mPainterAdasPtr->setSourceColor(COLOR_GREEN);
			if (width != 0 && height != 0)
				mPainterAdasPtr->drawRect(left, top, width, height, 0);
			adasFcwsLeft   = left;
			adasFcwsTop    = top;
			adasFcwsWidth  = width;
			adasFcwsHeight = height;
		}
	} else {
		mPainterAdasPtr->setLineWidth(LINE_WIDTH);
		mPainterAdasPtr->setSourceColor(COLOR_CLEAR);
		if (adasFcwsWidth != 0 && adasFcwsHeight != 0)
			mPainterAdasPtr->drawRect(adasFcwsLeft, adasFcwsTop, adasFcwsWidth, adasFcwsHeight, 0);
	}
}

static void VideoTimer_Handle(int s)
{
    IPC_CarInfo_Read_RecInfo(&carimpl.stRecInfo);
}

static void VideoTimer_Start(int iSec)
{
	struct itimerval tv_value;
	signal(SIGALRM, VideoTimer_Handle);

	tv_value.it_value.tv_sec = 1;
	tv_value.it_value.tv_usec = 0;
	tv_value.it_interval.tv_sec = iSec;
	tv_value.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &tv_value, NULL);
}

static void VideoTimer_Stop(void)
{
	struct itimerval tv_value;
	tv_value.it_value.tv_sec = 0;
	tv_value.it_value.tv_usec = 0;
	tv_value.it_interval = tv_value.it_value;
	setitimer(ITIMER_REAL, &tv_value, NULL);
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {
	// LOGD(" main onUI_show!\n");

	carimpl.stUIModeInfo.u8Mode = UI_VIDEO_MODE;
	IPC_CarInfo_Write_UIModeInfo(&carimpl.stUIModeInfo);

	//register CB FUNC
	set_key_event_callback(on_key_callback);

	IPC_CarInfo_Read_RecInfo(&carimpl.stRecInfo);
	VideoShowRtc();
	VideoShowRecStatus();
	VideoShowAudioStatus();
	VideoShowWifiStatus(check_wifi_status());
	VideoShowMovieRes();
	VideoShowEv();
	SHOW_SD_ICON(mTxtSdStatusPtr, bSdStatus);
	#ifdef USE_KEY_BOARD
	mBTN_RecPtr->setVisible(FALSE);
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
	// LOGD(" main onUI_hide!\n");
	mPainterAdasPtr->setVisible(FALSE);
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
	VideoTimer_Stop();
	IPC_MsgToUI_DestroyThread();
	IPC_MsgToUI_DeInit();
	IPC_CarInfo_Close();
	LOGD(" main onUI_quit!\n");
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
			VideoStateUpdate();
			break;
		case 1:
			DrawAdasInfo();
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
static bool onmainActivityTouchEvent(const MotionEvent &ev) {
	static MotionEvent s_ev;
	bool ret = false;

	#ifdef SUPPORT_CARLIFE
	if(carimpl_CarLifeIsShow()){
		if(carimpl_CarLifeSendTouchEvt(ev.mX, ev.mY,(int)ev.mActionStatus))
			return true;
		else
			printf("%s handle:(%d,%d,%d)\r\n",__FUNCTION__,ev.mX,ev.mY,ev.mActionStatus);
	}
	#endif

    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			s_ev = ev;
			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			//printf("main E_ACTION_MOVE (%d,%d)\r\n",ev.mX,ev.mY);
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			printf("main TouchEvent %d,%d\r\n",ev.mX,ev.mY);
			#ifdef PLAYING_AND_RECORDING
			if ((s_ev.mX > ev.mX) && ((s_ev.mX - ev.mX) > 200)) {//slide to the left
					gu8LastUIMode = UI_VIDEO_MODE;
					//carimpl_VideoH26xInit(FALSE);
					EASYUICONTEXT->openActivity("captureActivity");
					ret = true;
				} else if ((abs(s_ev.mY - ev.mY) > 200)) {//slide to the up or down
					gu8LastUIMode = UI_VIDEO_MODE;
					EASYUICONTEXT->openActivity("VideoMenuActivity");
					ret = true;
				}
			#else
			if (IMPL_REC_STATUS == FALSE &&
				IMPL_EMERG_REC_STATUS == FALSE &&
				IMPL_MD_REC_STATUS == FALSE) {
				if ((s_ev.mX > ev.mX) && ((s_ev.mX - ev.mX) > 200)) {//slide to the left
					gu8LastUIMode = UI_VIDEO_MODE;
					carimpl_VideoH26xInit(FALSE);
					EASYUICONTEXT->openActivity("captureActivity");
					ret = true;
				} else if ((abs(s_ev.mY - ev.mY) > 200)) {//slide to the up or down
					gu8LastUIMode = UI_VIDEO_MODE;
					EASYUICONTEXT->openActivity("VideoMenuActivity");
					ret = true;
				}
			}
			#endif
			s_ev.reset();
			break;
		default:
			break;
	}
	return ret;
}

static bool onButtonClick_BTN_WIFI(ZKButton *pButton) {
    //LOGD(" ButtonClick BTN_WIFI !!!\n");
	#ifdef SUPPORT_CARLIFE
	if(carimpl_CarLifeIsCnnt()){
		if(carimpl_CarLifeSwitch())
			return true;
	}
	#else
    carimpl_GeneralFunc_SetWifiOnOff();
    #endif
    
    return true;
}

static bool onButtonClick_BTN_Rec(ZKButton *pButton) {
    // LOGD(" ButtonClick BTN_Rec !!!\n");
	if (IMPL_REC_STATUS || IMPL_EMERG_REC_STATUS || IMPL_MD_REC_STATUS) {
		#ifdef USE_KEY_BOARD
		mBTN_RecPtr->setBackgroundPic("main_/sxt.png");
		#endif
		VideoShowRecStatus();
		carimpl_VideoFunc_StartRecording(0);
	} else {
		if (IMPL_SD_ISMOUNT && (bSdStatus == TRUE)) {
			VideoShowRecStatus();
			//carimpl_VideoFunc_StartRecording(1);
		} else if ((IMPL_SD_ISMOUNT && (bSdStatus == FALSE)) ||
		           (!IMPL_SD_ISMOUNT && (bSdStatus == TRUE)))
			carimpl_show_wmsg(true, WMSG_INSERT_SD_AGAIN);
		else {
			carimpl_show_wmsg(true, WMSG_NO_CARD);
		}
		carimpl_VideoFunc_StartRecording(1);//play sound in cardvimpl.
	}
    return true;
}

static bool onButtonClick_BTN_Cap(ZKButton *pButton) {
    // LOGD(" ButtonClick BTN_Cap !!!\n");
	if (IMPL_SD_ISMOUNT && (bSdStatus == TRUE)) {
		mTxtRecStatusPtr->setText("CAP"); // reset to STBY/REC in timer
		//carimpl_VideoFunc_Capture();
	}
	else if ((IMPL_SD_ISMOUNT && (bSdStatus == FALSE)) ||
				(!IMPL_SD_ISMOUNT && (bSdStatus == TRUE)))
		carimpl_show_wmsg(true, WMSG_INSERT_SD_AGAIN);
	else {
		carimpl_show_wmsg(true, WMSG_NO_CARD);
	}
	carimpl_VideoFunc_Capture();//play sound in cardvimpl.
    return true;
}

static bool onButtonClick_BTN_AudioMute(ZKButton *pButton) {
    // LOGD(" ButtonClick BTN_AudioMute !!!\n");
	#if (MENU_MOVIE_SOUND_RECORD_ON_EN && MENU_MOVIE_SOUND_RECORD_OFF_EN)
	if (MenuSettingConfig()->uiMOVSoundRecord == MOVIE_SOUND_RECORD_ON)
		MenuSettingConfig()->uiMOVSoundRecord = MOVIE_SOUND_RECORD_OFF;
	else
		MenuSettingConfig()->uiMOVSoundRecord = MOVIE_SOUND_RECORD_ON;
		
	carimpl_VideoFunc_SetSoundRecord();
	VideoShowAudioStatus();
	#endif
    return true;
}
