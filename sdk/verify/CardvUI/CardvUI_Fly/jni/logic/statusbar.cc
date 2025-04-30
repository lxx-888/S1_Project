#pragma once
#include "uart/ProtocolSender.h"
#include "carimpl/carimpl.h"
/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[标识]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index) 
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[标识]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress) 
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[标识]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX() 
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[标识]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[标识]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXX->setText("****") 在控件TextXXX上显示文字****
*mButton1->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBar->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度

*/

#define WMSGTIMEOUT       3 //s
static int m_WMSGShowTime = 0;

void StatusBar_Update(void)
{
	int iStrID = IDS_DS_EMPTY;
	WMSG_INFO WMSGInfo = carimpl_get_wmsginfo();

	switch (WMSGInfo)
	{
	case WMSG_CARD_ERROR:
		iStrID = IDS_DS_MSG_CARD_ERROR;
		break;
	case WMSG_STORAGE_FULL:
		iStrID = IDS_DS_MSG_STORAGE_FULL;
		break;
	case WMSG_NO_CARD:
		iStrID = IDS_DS_MSG_NO_CARD;
		break;
	case WMSG_LOW_BATTERY:
		iStrID = IDS_DS_MSG_LOW_BATTERY;
		break;
	case WMSG_FILE_ERROR:
		iStrID = IDS_DS_MSG_FILE_ERROR;
		break;
	case WMSG_CARD_LOCK:
		iStrID = IDS_DS_MSG_CARD_LOCK;
		break;
	case WMSG_CARD_SLOW:
		iStrID = IDS_DS_MSG_CARD_SLOW;
		break;
	case WMSG_ADAPTER_ERROR:
		iStrID = IDS_DS_MSG_ADAPTER_ERROR;
		break;
	case WMSG_INVALID_OPERATION:
		iStrID = IDS_DS_MSG_INVALID_OPERATION;
		break;
	case WMSG_CANNOT_DELETE:
		iStrID = IDS_DS_MSG_CANNOT_DELETE;
		break;
	case WMSG_BATTERY_FULL:
		iStrID = IDS_DS_MSG_BATTERY_FULL;
		break;
	case WMSG_LENS_ERROR:
		iStrID = IDS_DS_MSG_LENS_ERROR;
		break;
	case WMSG_HDMI_TV:
		iStrID = IDS_DS_MSG_HDMI_TV;
		break;
	case WMSG_FHD_VR:
		iStrID = IDS_DS_MSG_FHD_VR;
		break;
	case WMSG_PLUG_OUT_SD_CARD:
		iStrID = IDS_DS_MSG_PLUG_OUT_SD;
		break;
	case WMSG_WAIT_INITIAL_DONE:
		iStrID = IDS_DS_MSG_WAIT_INITIAL_DONE;
		break;
	case WMSG_INSERT_SD_AGAIN:
		iStrID = IDS_DS_MSG_INSERT_SD_AGAIN;
		break;
	case WMSG_FORMAT_SD_PROCESSING:
		iStrID = IDS_DS_MSG_FORMAT_SD_PROCESSING;
		break;
	case WMSG_FORMAT_SD_CARD_OK:
		iStrID = IDS_DS_MSG_FORMAT_SD_CARD_OK;
		break;
	case WMSG_FORMAT_SD_CARD_FAIL:
		iStrID = IDS_DS_MSG_FORMAT_SD_CARD_FAIL;
		break;
	case WMSG_COME2EMER:
		break;
	case WMSG_TIME_ERROR:
		break;
	case WMSG_SHOW_FW_VERSION:
		iStrID = IDS_DS_FW_VERSION_INFO;
		break;
	case WMSG_START_RECORD:
		break;
	case WMSG_START_NORMAL_RECORD:
		break;
	case WMSG_SPEEDCAM_ADDED:
		break;
	case WMSG_ADD_SPEEDCAM:
		break;
	case WMSG_DEL_SPEEDCAM:
		break;
	case WMSG_NO_POI_SPACE:
		break;
	case WMSG_WAIT_GPS_FIX:
		break;
	case WMSG_PARKING_MODE_DISABLE:
		break;
	case WMSG_LDWS_RightShift:
		break;
	case WMSG_LDWS_LeftShift:
		break;
	case WMSG_SEAMLESS_ERROR:
		iStrID = IDS_DS_MSG_CYCLE_RECORD_CLEAN_SPACE;
		break;
	case WMSG_FORMAT_SD_CARD:
		iStrID = IDS_DS_MSG_FORMAT_SD_CARD;
		break;
	case WMSG_LOCK_CURRENT_FILE:
		iStrID = IDS_DS_MSG_LOCK_CURRENT_FILE;
		break;
	case WMSG_UNLOCK_CUR_FILE:
		iStrID = IDS_DS_MSG_UNLOCK_FILE;
		break;
	case WMSG_DELETE_FILE_OK:
		iStrID = IDS_DS_MSG_DELETE_FILE_OK;
		break;
	case WMSG_PROTECT_FILE_OK:
		iStrID = IDS_DS_MSG_PROTECT_FILE_OK;
		break;
	case WMSG_GOTO_POWER_OFF:
		iStrID = IDS_DS_MSG_GOTO_POWER_OFF;
		break;
	case WMSG_NO_FILE_IN_BROWSER:
		iStrID = IDS_DS_MSG_NO_FILE;
		break;
	case WMSG_CAPTURE_CUR_FRAME:
		iStrID = IDS_DS_MSG_CAPTURE_SCREEN;
		break;
	case WMSG_FCWS:
		iStrID = IDS_DS_STOP;
		break;
	case WMSG_FATIGUEALERT:
		iStrID = IDS_DS_NO;
		break;
	case WMSG_OPENFILE_WAIT:
		iStrID = IDS_DS_MSG_OPENFILE_WAIT;
		break;
	case WMSG_NO_CARD1:
		break;
	case WMSG_NO_CARD2:
		break;
	case WMSG_CARD2_FULL:
		break;
	case WMSG_NO_CARD2_2:
		break;
	case WMSG_BACKUP_FINISH:
		break;
	case WMSG_MOTION_OPERATING_DESCRIPTION:
		break;
	case WMSG_MSG_EMERGENCY_FILE_FULL:
		break;
	case WMSG_REMIND_HEADLIGHT:
		break;
	case WMSG_PARKING_OPERATING_DESCRIPTION:
		break;
	case WMSG_SYSTEM_POWER_OFF:
		iStrID = IDS_DS_SYSTEM_POWER_OFF;
		break;
	default:
		break;
	}
	mwarningmsg_textPtr->setTextTr(MAP_STRINGID(iStrID));
	m_WMSGShowTime = 0;
}

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	{0,  1000}, //定时器id=0, 时间间隔1秒
	//{1,  1000},
};

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");

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
    //串口数据回调接口
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
		if (EASYUICONTEXT->isStatusBarShow()) {
			m_WMSGShowTime++;
			if (m_WMSGShowTime >= WMSGTIMEOUT &&
				carimpl_get_wmsginfo() != WMSG_FORMAT_SD_PROCESSING) {
				EASYUICONTEXT->hideStatusBar();
				m_WMSGShowTime = 0;
				if (carimpl_get_wmsginfo() == WMSG_SYSTEM_POWER_OFF) {
					carimpl_SysFunc_SetPowerOnGSensorSensitivity();
					system("./sbin/poweroff");
					printf("-----%s-----\n",__func__);
				}
			}
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
static bool onstatusbarActivityTouchEvent(const MotionEvent &ev) {
    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			break;
		default:
			break;
	}
	return false;
}
