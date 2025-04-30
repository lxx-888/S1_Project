/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __BROWSERACTIVITY_H__
#define __BROWSERACTIVITY_H__


#include "app/Activity.h"
#include "entry/EasyUIContext.h"

#include "uart/ProtocolData.h"
#include "uart/ProtocolParser.h"

#include "utils/Log.h"
#include "control/ZKDigitalClock.h"
#include "control/ZKButton.h"
#include "control/ZKCircleBar.h"
#include "control/ZKDiagram.h"
#include "control/ZKListView.h"
#include "control/ZKPointer.h"
#include "control/ZKQRCode.h"
#include "control/ZKTextView.h"
#include "control/ZKSeekBar.h"
#include "control/ZKEditText.h"
#include "control/ZKVideoView.h"
#include "window/ZKSlideWindow.h"

/*TAG:Macro宏ID*/
#define ID_BROWSER_Thumb5_dot    50017
#define ID_BROWSER_Thumb4_dot    50016
#define ID_BROWSER_Thumb3_dot    50015
#define ID_BROWSER_Thumb2_dot    50014
#define ID_BROWSER_Thumb1_dot    50013
#define ID_BROWSER_Thumb0_dot    50006
#define ID_BROWSER_Thumb3    20008
#define ID_BROWSER_Thumb5    20007
#define ID_BROWSER_Thumb4    20006
#define ID_BROWSER_Thumb2    20005
#define ID_BROWSER_Thumb1    20004
#define ID_BROWSER_Thumb0    20003
#define ID_BROWSER_TxtSdStatus    50012
#define ID_BROWSER_TXT_Page    50011
#define ID_BROWSER_TXT_ObLine    50010
#define ID_BROWSER_TXT_TotalPage    50009
#define ID_BROWSER_TXT_CurPage    50008
#define ID_BROWSER_BTN_Down    20002
#define ID_BROWSER_BTN_Up    20001
#define ID_BROWSER_TXT_MovFlag    50007
#define ID_BROWSER_TXT_FileName    50005
#define ID_BROWSER_TXT_FileDur    50004
#define ID_BROWSER_TXT_SysTime    50003
#define ID_BROWSER_Textview2    50002
#define ID_BROWSER_Textview1    50001
/*TAG:Macro宏ID END*/

class browserActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener, 
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
                     public EasyUIContext::ITouchListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:
    browserActivity();
    virtual ~browserActivity();

    /**
     * 注册定时器
     */
	void registerUserTimer(int id, int time);
	/**
	 * 取消定时器
	 */
	void unregisterUserTimer(int id);
	/**
	 * 重置定时器
	 */
	void resetUserTimer(int id, int time);

protected:
    /*TAG:PROTECTED_FUNCTION*/
    virtual const char* getAppName() const;
    virtual void onCreate();
    virtual void onClick(ZKBase *pBase);
    virtual void onResume();
    virtual void onPause();
    virtual void onIntent(const Intent *intentPtr);
    virtual bool onTimer(int id);

    virtual void onProgressChanged(ZKSeekBar *pSeekBar, int progress);

    virtual int getListItemCount(const ZKListView *pListView) const;
    virtual void obtainListItemData(ZKListView *pListView, ZKListView::ZKListItem *pListItem, int index);
    virtual void onItemClick(ZKListView *pListView, int index, int subItemIndex);

    virtual void onSlideItemClick(ZKSlideWindow *pSlideWindow, int index);

    virtual bool onTouchEvent(const MotionEvent &ev);

    virtual void onTextChanged(ZKTextView *pTextView, const string &text);

    void registerActivityTimer();
    void unregisterActivityTimer();

    virtual void onVideoPlayerMessage(ZKVideoView *pVideoView, int msg);
    void videoLoopPlayback(ZKVideoView *pVideoView, int msg, size_t callbackTabIndex);
    void startVideoLoopPlayback();
    void stopVideoLoopPlayback();
    bool parseVideoFileList(const char *pFileListPath, std::vector<string>& mediaFileList);
    int removeCharFromString(string& nString, char c);


private:
    /*TAG:PRIVATE_VARIABLE*/
    int mVideoLoopIndex;
    int mVideoLoopErrorCount;

};

#endif