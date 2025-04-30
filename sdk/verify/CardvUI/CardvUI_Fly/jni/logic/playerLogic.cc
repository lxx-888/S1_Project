#pragma once
#include "uart/ProtocolSender.h"
#include "carimpl/KeyEventContext.h"
#include "carimpl/MenuCommon.h"

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

#ifdef SUPPORT_PLAYER_MODULE
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include "mi_sys.h"
#include "mi_disp.h"
#include "usbdetect.h"
#include "frame.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"
#include "player.h"
#include "carimpl/carimpl.h"

#if defined(SUPPORT_FB1)
#include "window.h"
#include "base.h"
#include "text.h"
#include "seekbar.h"
#endif

#if (!defined CHIP_I6E)
#define USE_PANEL_1024_600		1
#if USE_PANEL_1024_600
#define PANEL_MAX_WIDTH			1024
#define PANEL_MAX_HEIGHT		600
#else
#define PANEL_MAX_WIDTH			896     //=(854+63)/64 * 64
#define PANEL_MAX_HEIGHT		512     //(480+63)/64 * 64
#endif
#endif

#define ALIGN_DOWN(x, n)		(x / n * n)
#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#define DISP_DEV                (0)
#define DISP_LAYER              (0)
#define DISP_INPUTPORT          (0)

#define AUDIO_DEV               (0)
#define AUDIO_CHN               (0)
#define AUDIO_SAMPLE_PER_FRAME  (1024)
#define AUDIO_MAX_DATA_SIZE     (25000)
#define MIN_AO_VOLUME           (-60)
#define MAX_AO_VOLUME           (30)
#define MIN_ADJUST_AO_VOLUME    (-10)
#define MAX_ADJUST_AO_VOLUME    (20)
#define VOL_ADJUST_FACTOR       (2)

#define SEEK_OFFSET_MAX         (20000000) // 20s
#define SEEK_OFFSET_MIN         (4000000)  // 4s

#define DEBUG_AVSYNC            (0)

typedef enum
{
    E_PLAY_PRE = -1,
    E_PLAY_NEXT = 1,
} PlayFile_e;

#if (DEBUG_AVSYNC)
typedef struct
{
#define MAX_VIDEO_FRAME 3600
#define MAX_AUDIO_FRAME 1800
    MI_U32 u32VideoFrameCnt;
    MI_U32 u32AudioFrameCnt;
    MI_U64 u64VideoFramePts[MAX_VIDEO_FRAME];
    MI_U64 u64AudioFramePts[MAX_AUDIO_FRAME];
    MI_U32 u32AudioFrameSize[MAX_AUDIO_FRAME];
} DebugAVSync_t;

static DebugAVSync_t g_stDebugAVSync = {0};
#endif

#if defined(SUPPORT_FB1)
struct rect {
    int x;
    int y;
    int w;
    int h;
};

static Window *pWin = NULL;
static Text *pFileNameStr = NULL;
static struct rect stFileNameStrRect = {200, 10, 1500, 64};
static SeekBar *pDurationBar = NULL;
static struct rect stDurationBarRect = {60, 900, 1800, 5};
static Text *pFileDurStr = NULL;
static struct rect stFileDurStrRect = {1550, 960, 300, 64};
#endif

// playing page
static bool g_bShowPlayToolBar = FALSE;          // select file list page or playing page
static bool g_bPlaying = FALSE;
static bool g_bPause = FALSE;
static bool g_bMute = FALSE;
static int g_s32VolValue = 0;
static int g_ePlayDirection = FAST_FORWARD;
static int g_eSpeedMode = SPEED_NORMAL;
static unsigned int g_u32SpeedNumerator = 1;
static unsigned int g_u32SpeedDenomonator = 1;

static unsigned char g_u8CurPlayCamId = 0;

static player_stat_t *g_pstPlayStat = NULL;

// play pos
static long long g_firstPlayPos = PLAY_INIT_POS;
static long long g_duration = 0;
static long long g_curPlayPos = 0;

static bool onButtonClick_sys_back(ZKButton *pButton);
static bool onButtonClick_Button_play(ZKButton *pButton);
static bool onButtonClick_Button_fast(ZKButton *pButton);
static bool onButtonClick_Button_slow(ZKButton *pButton);
static MI_S32 PlayStart(char *pszFileName, long long llStartPos);
static MI_S32 PlayStart(int iOffset);
static MI_S32 PlayStop();
static void Seek(bool bForward);

static void ShowTouchToolbar(bool bShow)
{
    if (!g_bPlaying)
        goto end_handle;

    mSeekbar_volumnPtr->setVisible(bShow);
    mTextview_slashPtr->setVisible(bShow);
    mTextview_durationPtr->setVisible(bShow);
    mTextview_curtimePtr->setVisible(bShow);
    mTextview_speedPtr->setVisible(bShow);
    mButton_voicePtr->setVisible(bShow);
    mButton_fastPtr->setVisible(bShow);
    mButton_slowPtr->setVisible(bShow);
    mButton_stopPtr->setVisible(bShow);
    mButton_playPtr->setVisible(bShow);
    mSeekbar_progressPtr->setVisible(bShow);
    mTextview_playBarPtr->setVisible(bShow);

#if defined(SUPPORT_FB1)
    pDurationBar->setVisible(bShow);
    pFileDurStr->setVisible(bShow);
#endif

end_handle:
    // TODO
    // it will crash in libeasyui.so,so we just temporary comment this to prevent crash,but don't know why
    // issueId: 83201bde-4616-49d6-9171-d00a0661e907
    // mBtn_NextFilePtr->setVisible(bShow);
    return;
}

static void ShowKeyEventToolbar(bool bShow)
{
    mTextview_slashPtr->setVisible(bShow);
    mTextview_durationPtr->setVisible(bShow);
    mTextview_curtimePtr->setVisible(bShow);
    mTextview_speedPtr->setVisible(bShow); // TODO
    mButton_playPtr->setVisible(bShow);
    mSeekbar_progressPtr->setVisible(bShow);

#if defined(SUPPORT_FB1)
    pDurationBar->setVisible(bShow);
    pFileDurStr->setVisible(bShow);
#endif
}

class ToolbarHideThread : public Thread {
public:
    void setCycleCnt(int cnt, int sleepMs) { nCycleCnt = cnt; nSleepMs = sleepMs; }

protected:
    virtual bool threadLoop() {
        if (!nCycleCnt)
        {
            ShowTouchToolbar(false);
            return false;
        }

        sleep(nSleepMs);
        nCycleCnt--;

        return true;
    }

private:
    int nCycleCnt;
    int nSleepMs;
};

static ToolbarHideThread g_hideToolbarThread;

// auto hide toolbar after displaying 5s
void AutoDisplayToolbar()
{
    unsigned char u8DB = Carimpl_GetCurDB();
    if (u8DB == DB_PHOTO)
    {
        ShowTouchToolbar(false);
        mBtn_NextFilePtr->setVisible(true);
        return;
    }

    if (!g_hideToolbarThread.isRunning())
    {
        printf("start hide toolbar thread\n");
        g_hideToolbarThread.setCycleCnt(100, 50);
        g_hideToolbarThread.run("hideToolbar");
    }
    else
    {
        printf("wait thread exit\n");
        g_hideToolbarThread.requestExitAndWait();
        g_hideToolbarThread.setCycleCnt(100, 50);
        g_hideToolbarThread.run("hideToolbar");
    }

    ShowTouchToolbar(true);
}

void SetPlayingStatus(bool bPlaying)
{
    mButton_playPtr->setSelected(bPlaying);
}

void SetMuteStatus(bool bMute)
{
    mButton_voicePtr->setSelected(bMute);
}

int SetPlayerVolumn(int vol)
{
    mSeekbar_volumnPtr->setProgress(vol);
    return 0;
}

int GetPlayerVolumn()
{
    return mSeekbar_volumnPtr->getProgress();
}

MI_S32 CreatePlayerDev()
{
    IPC_CarInfo_Read_DispInfo(&carimpl.stDispInfo);
    Carimpl_Send2Fifo(CARDV_CMD_START_PLAYBACK, sizeof(CARDV_CMD_START_PLAYBACK));
    usleep(300000); // TBD : Wait for cardv create playback pipe.
    return MI_SUCCESS;
}

void DestroyPlayerDev()
{
    Carimpl_Send2Fifo(CARDV_CMD_STOP_PLAYBACK, sizeof(CARDV_CMD_STOP_PLAYBACK));
}

MI_S32 StartPlayVideo()
{
    return 0;
}

void StopPlayVideo()
{
}

MI_S32 StartPlayAudio()
{
    MI_AO_Attr_t stSetAttr;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;

    memset(&stSetAttr, 0, sizeof(MI_AO_Attr_t));
    stSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stSetAttr.u32PeriodSize = AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_MONO;
    stSetAttr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
    stSetAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_ONLY_LEFT;

    MI_AO_Open(AoDevId, &stSetAttr);
    MI_AO_AttachIf(AoDevId, E_MI_AO_IF_DAC_AB, 0);

    return 0;
}

void StopPlayAudio()
{
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_DetachIf(AoDevId, E_MI_AO_IF_DAC_AB);
    MI_AO_Close(AoDevId);
}

static void ResetSpeedMode()
{
    g_ePlayDirection = FAST_FORWARD;
    g_eSpeedMode = SPEED_NORMAL;
    g_u32SpeedNumerator = 1;
    g_u32SpeedDenomonator = 1;
}

// duration, format, width, height, I-frame/P-frame, etc.
MI_S32 GetMediaInfo()
{
    return 0;
}

MI_S32 GetDuration(long long duration)
{
    char totalTime[32];
    long long durationSec = duration / AV_TIME_BASE;

    if (durationSec / 3600 > 99)
    {
        printf("file size is limited\n");
        return -1;
    }

    memset(totalTime, 0, sizeof(totalTime));
    sprintf(totalTime, "%02lld:%02lld:%02lld", durationSec/3600, (durationSec%3600)/60, durationSec%60);
    mTextview_durationPtr->setText(totalTime);
    g_duration = duration;

    return 0;
}

MI_S32 GetCurrentPlayPos(long long currentPos, long long frame_duration)
{
    char curTime[32];
    long long curSec = 0;
    static long long lastSec = -1;

    if (currentPos > g_duration)
    {
        printf("curPos [%lld] exceed duration [%lld]\n", currentPos, g_duration);
        currentPos = g_duration;
    }

    g_curPlayPos = currentPos;

    // update playtime static
    if (g_firstPlayPos < 0)
        curSec = 0;

    #if 0	// reflesh UI time info always.
    else
    {
        long long u64curTime = currentPos % 1000000;
        if (u64curTime > frame_duration * 2 && u64curTime <= (1000000 - frame_duration * 2))
        {
            printf("u64curTime [%lld] > 2*frame_duration [%lld]\n", u64curTime, frame_duration);
            // NOT reflesh UI time info.
            return 0;
        }
    }
    #endif

    curSec = currentPos / AV_TIME_BASE;

    memset(curTime, 0, sizeof(curTime));
    sprintf(curTime, "%02lld:%02lld:%02lld", curSec/3600, (curSec%3600)/60, curSec%60);
    if (lastSec != curSec) {
        // reduce CPU loding
        mTextview_curtimePtr->setText(curTime);
#if defined(SUPPORT_FB1)
        pFileDurStr->setText(curTime);
#endif
    }
    lastSec = curSec;

    // update progress bar
    int trackPos = (currentPos * mSeekbar_progressPtr->getMax()) / g_duration;
    mSeekbar_progressPtr->setProgress(trackPos);

#if defined(SUPPORT_FB1)
    trackPos = (currentPos * pDurationBar->getMax()) / g_duration;
    pDurationBar->setProgress(trackPos);
#endif

    if (g_firstPlayPos < 0) {
        g_firstPlayPos = currentPos;
    }

    return 0;
}

// MI display video
MI_S32 DisplayVideo(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData, void *pUData, void *pVData)
{
    #if (DEBUG_AVSYNC)
    MI_U64 u64Pts;
    MI_SYS_GetCurPts(&u64Pts);
    if (g_stDebugAVSync.u32VideoFrameCnt < MAX_VIDEO_FRAME)
        g_stDebugAVSync.u64VideoFramePts[g_stDebugAVSync.u32VideoFrameCnt++] = u64Pts;
    #endif

    return carimpl_refresh_display_yuv420p(s32DispWidth, s32DispHeight, pYData, pUData, pVData);
}

MI_S32 DisplayVideoYuv422(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYuvData)
{
    #if (DEBUG_AVSYNC)
    MI_U64 u64Pts;
    MI_SYS_GetCurPts(&u64Pts);
    if (g_stDebugAVSync.u32VideoFrameCnt < MAX_VIDEO_FRAME)
        g_stDebugAVSync.u64VideoFramePts[g_stDebugAVSync.u32VideoFrameCnt++] = u64Pts;
    #endif

    return carimpl_refresh_display_yuv422(s32DispWidth, s32DispHeight, pYuvData);
}

MI_S32 DisplayVideoYuv422p(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData, void *pUData, void *pVData)
{
    #if (DEBUG_AVSYNC)
    MI_U64 u64Pts;
    MI_SYS_GetCurPts(&u64Pts);
    if (g_stDebugAVSync.u32VideoFrameCnt < MAX_VIDEO_FRAME)
        g_stDebugAVSync.u64VideoFramePts[g_stDebugAVSync.u32VideoFrameCnt++] = u64Pts;
    #endif

    return carimpl_refresh_display_yuv422p(s32DispWidth, s32DispHeight, pYData, pUData, pVData);
}
// MI play audio
MI_S32 PlayAudio(MI_U8 *pu8AudioData, MI_U32 u32DataLen, MI_U32 *pu32DataBusyLen)
{
    MI_U32 u32Remaining;
    MI_U64 u64TStamp;
    MI_S32 s32RetSendStatus = 0;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;

    #if (DEBUG_AVSYNC)
    MI_U64 u64Pts;
    MI_SYS_GetCurPts(&u64Pts);
    if (g_stDebugAVSync.u32AudioFrameCnt < MAX_AUDIO_FRAME) {
        g_stDebugAVSync.u32AudioFrameSize[g_stDebugAVSync.u32AudioFrameCnt] = u32DataLen;
        g_stDebugAVSync.u64AudioFramePts[g_stDebugAVSync.u32AudioFrameCnt++] = u64Pts;
    }
    #endif

    if (pu8AudioData && u32DataLen) {
        do {
            s32RetSendStatus = MI_AO_Write(AoDevId, pu8AudioData, u32DataLen, 0, -1);
        } while (s32RetSendStatus == MI_AO_ERR_NOBUF);

        if (s32RetSendStatus != MI_SUCCESS) {
            printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n",s32RetSendStatus);
        }
    }

    MI_AO_GetTimestamp(AoDevId, &u32Remaining, &u64TStamp);
    *pu32DataBusyLen = u32Remaining;

    return 0;
}

// pause audio
MI_S32 PauseAudio()
{
    MI_AO_Pause(AUDIO_DEV);
    return 0;
}

// resume audio
MI_S32 ResumeAudio()
{
    MI_AO_Resume(AUDIO_DEV);
    return 0;
}

// stay in playing page, clear play status
MI_S32 PlayComplete()
{
    #if (DEBUG_AVSYNC)
    printf("Video Play :\n");
    for (MI_U32 i = 0; i < g_stDebugAVSync.u32VideoFrameCnt; i++) {
        if (i)
            printf("\tframe [%4d] pts [%10lld] duration [%lld]\n",
                i, g_stDebugAVSync.u64VideoFramePts[i], g_stDebugAVSync.u64VideoFramePts[i] - g_stDebugAVSync.u64VideoFramePts[i-1]);
        else
            printf("\tframe [%4d] pts [%10lld]\n", i, g_stDebugAVSync.u64VideoFramePts[i]);
    }

    printf("Audio Play :\n");
    for (MI_U32 i = 0; i < g_stDebugAVSync.u32AudioFrameCnt; i++) {
        if (i)
            printf("\tframe [%4d] pts [%10lld] size [%d] duration [%lld]\n",
                i, g_stDebugAVSync.u64AudioFramePts[i], g_stDebugAVSync.u32AudioFrameSize[i], g_stDebugAVSync.u64AudioFramePts[i] - g_stDebugAVSync.u64AudioFramePts[i-1]);
        else
            printf("\tframe [%4d] pts [%10lld] size [%d]\n", i, g_stDebugAVSync.u64AudioFramePts[i], g_stDebugAVSync.u32AudioFrameSize[i]);
    }
    #endif

    PlayStop();
    g_bShowPlayToolBar = FALSE;

    if (Carimpl_GetCurDB() != DB_PHOTO) {
        gu8LastUIMode = UI_PLAYBACK_MODE;
        EASYUICONTEXT->goBack();
    }

    return 0;
}

// stay in playing page , clear play status,
MI_S32 PlayError(int error)
{
    printf("PlayError [%d]\n", error);
    PlayComplete();
    return 0;
}

static void SetPlayerControlCallBack(player_stat_t *is)
{
    #if (DEBUG_AVSYNC)
    g_stDebugAVSync.u32VideoFrameCnt = 0;
    g_stDebugAVSync.u32AudioFrameCnt = 0;
    #endif
    is->playerController.fpGetMediaInfo = GetMediaInfo;
    is->playerController.fpGetDuration = GetDuration;
    is->playerController.fpGetCurrentPlayPos = GetCurrentPlayPos;
    is->playerController.fpGetCurrentPlayPosFromVideo = NULL;
    is->playerController.fpGetCurrentPlayPosFromAudio = NULL;
    is->playerController.fpDisplayVideo = DisplayVideo;
    is->playerController.fpDisplayVideoYuv422 = DisplayVideoYuv422;
    is->playerController.fpDisplayVideoYuv422p = DisplayVideoYuv422p;
    is->playerController.fpPlayAudio = PlayAudio;
    is->playerController.fpPauseAudio = PauseAudio;
    is->playerController.fpResumeAudio = ResumeAudio;
    is->playerController.fpPlayError = PlayError;
}

static MI_S32 PlayStop()
{
    if (g_pstPlayStat && g_bPlaying) {
        g_bPlaying = false;
        g_bPause = false;

        player_deinit(g_pstPlayStat);
        g_pstPlayStat = NULL;
        if (Carimpl_GetCurDB() != DB_PHOTO)
            StopPlayAudio();
        StopPlayVideo();
        ResetSpeedMode();

        SetPlayingStatus(false);
        mTextview_speedPtr->setText("");

        // reset pts
        g_firstPlayPos = PLAY_INIT_POS;
    }
    return 0;
}

static MI_S32 PlayStart(int iOffset) {
    #ifdef SUPPORT_PLAYER_MODULE
    unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
    unsigned int u32TotalFile = Carimpl_GetTotalFiles();
    char *pszFileName = NULL;

    if (iOffset > 0) {
        u32CurFileIdx += 1;
        if (u32CurFileIdx >= u32TotalFile)
            u32CurFileIdx = 0;
    } else if (iOffset < 0) {
        u32CurFileIdx -= 1;
        if (u32CurFileIdx >= u32TotalFile)
            u32CurFileIdx = u32TotalFile - 1;
    }
    Carimpl_SetCurFileIdx(u32CurFileIdx);
    pszFileName = Carimpl_IsSubFileEnable() ?
        Carimpl_GetCurSubFileName(u32CurFileIdx) : Carimpl_GetCurFileName(u32CurFileIdx);
    if (pszFileName == NULL)
        pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
    if (pszFileName)
        PlayStart(pszFileName, PLAY_INIT_POS);
    #endif
    return 0;
}

static MI_S32 PlayStart(char *pszFileName, long long llStartPos) {
    #ifdef SUPPORT_PLAYER_MODULE
    if (!g_pstPlayStat && !g_bPlaying) {
        mTextview_videoInfoPtr->setText(pszFileName);
#if defined(SUPPORT_FB1)
        pFileNameStr->setText(pszFileName);
        pFileNameStr->setVisible(true);
#endif

        // init player
        ResetSpeedMode();
        StartPlayVideo();
        if (Carimpl_GetCurDB() != DB_PHOTO)
            StartPlayAudio();

        g_pstPlayStat = player_init(pszFileName);
        if (!g_pstPlayStat) {
            StopPlayAudio();
            StopPlayVideo();
            printf("Initilize player failed!\n");
            return 0;
        }

        printf("video file name is : %s\n", g_pstPlayStat->filename);

        // sendmessage to play file
        g_bPlaying = TRUE;
        g_bPause = FALSE;

        SetPlayerControlCallBack(g_pstPlayStat);
        if (llStartPos != PLAY_INIT_POS)
            stream_seek(g_pstPlayStat, llStartPos, 0, 0);

        if (open_demux(g_pstPlayStat) < 0) {
            carimpl_show_wmsg(true, WMSG_FILE_ERROR);
            PlayError(1);
            return 0;
        }

        if (open_video(g_pstPlayStat) < 0) {
            carimpl_show_wmsg(true, WMSG_FILE_ERROR);
            PlayError(2);
            return 0;
        }

        if (open_audio(g_pstPlayStat) < 0) {
            carimpl_show_wmsg(true, WMSG_FILE_ERROR);
            PlayError(3);
            return 0;
        }

        SetPlayingStatus(true);
        SetPlayerVolumn(g_s32VolValue);
    }
    #endif
    return 0;
}

static void Seek(bool bForward)
{
    long long seek_offset = g_duration / 5;
    if (seek_offset > SEEK_OFFSET_MAX)
        seek_offset = SEEK_OFFSET_MAX;
    if (seek_offset < SEEK_OFFSET_MIN)
        seek_offset = SEEK_OFFSET_MIN;

    if (bForward && ((g_curPlayPos + seek_offset) < g_duration))
        stream_seek(g_pstPlayStat, g_curPlayPos + seek_offset, 0, 0);
    else if (!bForward && ((g_curPlayPos - seek_offset) > 0))
        stream_seek(g_pstPlayStat, g_curPlayPos - seek_offset, 0, 0);
}

static void AdjustVolumeByTouch(int startPos, int endPos)
{
    int progress = mSeekbar_volumnPtr->getProgress();
    // move up, vol++; move down, vol--
    progress -= (endPos - startPos) / VOL_ADJUST_FACTOR;

    progress = (progress > mSeekbar_volumnPtr->getMax())? mSeekbar_volumnPtr->getMax() : progress;
    progress = (progress < 0)? 0 : progress;
    mSeekbar_volumnPtr->setProgress(progress);

    printf("set progress: %d\n", progress);
}

#endif
/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
    {0,  1000}, //定时器id=0, 时间间隔6秒
    {1,  50},
};

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
    printf("create player dev\n");
#ifdef SUPPORT_PLAYER_MODULE
    // init pts
    g_firstPlayPos = PLAY_INIT_POS;
    CreatePlayerDev();

#if defined(SUPPORT_FB1)
    pWin = new Window("Play", 0, 0, 1920, 1080, 0x00000000);
    if (pWin) {
        pDurationBar = new SeekBar(pWin, stDurationBarRect.x, stDurationBarRect.y, stDurationBarRect.w, stDurationBarRect.h);

        pFileNameStr = new Text(pWin, stFileNameStrRect.x, stFileNameStrRect.y, stFileNameStrRect.w, stFileNameStrRect.h);
        pFileNameStr->setTextSize(stFileNameStrRect.h);

        pFileDurStr = new Text(pWin, stFileDurStrRect.x, stFileDurStrRect.y, stFileDurStrRect.w, stFileDurStrRect.h);
        pFileDurStr->setTextSize(stFileDurStrRect.h);
    }
#endif
#endif
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
#ifdef SUPPORT_PLAYER_MODULE
    unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
#if defined(SUPPORT_FB1)
	char *pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
#else
    char *pszFileName = Carimpl_IsSubFileEnable() ?
        Carimpl_GetCurSubFileName(u32CurFileIdx) : Carimpl_GetCurFileName(u32CurFileIdx);
#endif

    if (pszFileName == NULL)
        pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);

    if (pszFileName != NULL) {
        g_u8CurPlayCamId = Carimpl_GetCurCamId();
        PlayStart(pszFileName, PLAY_INIT_POS);
    }

    if (Carimpl_GetCurDB() == DB_PHOTO) {
        ShowTouchToolbar(false);
        ShowKeyEventToolbar(false);
    } else {
        #if (defined USE_KEY_BOARD)
        ShowTouchToolbar(false);
        ShowKeyEventToolbar(true);
        mTextview_speedPtr->setText("Normal");
        #else
        AutoDisplayToolbar();
        #endif
    }
#endif
}

static void on_key_callback(int keyCode, int keyStatus) {
    printf("[Player] key [%s] %s\n", KEYCODE_TO_STRING(keyCode), KEYSTATUS_TO_STRING(keyStatus));
    switch (keyCode) {
    case KEY_UP:
        if (keyStatus == UI_KEY_RELEASE) {
            if (Carimpl_GetCurDB() == DB_PHOTO) {
                PlayStop();
                PlayStart(E_PLAY_PRE);
            } else {
                Seek(false);
            }
        } else if (keyStatus == UI_KEY_LRELEASE) {
            onButtonClick_Button_slow(NULL);
        }
        break;
    case KEY_DOWN:
        if (keyStatus == UI_KEY_RELEASE) {
            if (Carimpl_GetCurDB() == DB_PHOTO) {
                PlayStop();
                PlayStart(E_PLAY_NEXT);
            } else {
                Seek(true);
            }
        } else if (keyStatus == UI_KEY_LRELEASE) {
            onButtonClick_Button_fast(NULL);
        }
        break;
    case KEY_RIGHT:
        if (keyStatus == UI_KEY_RELEASE) {
            onButtonClick_sys_back(msys_backPtr);
        }
        break;
    case KEY_ENTER:
        if (keyStatus == UI_KEY_RELEASE) {
            if (Carimpl_GetCurDB() != DB_PHOTO)
                onButtonClick_Button_play(mButton_playPtr);
        }
        break;
    case KEY_LEFT:
        if (keyStatus == UI_KEY_RELEASE) {
            unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
            char *pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
            unsigned char u8CurPlayCamId = g_u8CurPlayCamId;
            do {
                g_u8CurPlayCamId ++;
                if (g_u8CurPlayCamId >= CAM_NUM)
                    g_u8CurPlayCamId = 0;
                char *pszNextFileName = Carimpl_GetPairFileName(pszFileName, g_u8CurPlayCamId);
				//printf("%s->%s,CamId%d\n",pszFileName,pszNextFileName,g_u8CurPlayCamId);
                if (pszNextFileName) {
                    PlayStop();
                    PlayStart(pszNextFileName, g_curPlayPos);
                    break;
                }
            } while (g_u8CurPlayCamId != u8CurPlayCamId);

            if (g_u8CurPlayCamId == u8CurPlayCamId) {
                printf("NO pair file\n");
            } else {
                Carimpl_SetCurFolder(Carimpl_GetCurDB(), g_u8CurPlayCamId);
                Carimpl_SetCurFileIdx(Carimpl_GetCurDB(), g_u8CurPlayCamId, u32CurFileIdx);
            }
        }
        break;
    case KEY_MENU:
    case KEY_MENU_2:
        if (keyStatus == UI_KEY_RELEASE) {
            if (g_bPlaying) {
                toggle_pause(g_pstPlayStat);
                g_bPause = !g_bPause;
            }
            gu8LastUIMode = UI_PLAYBACK_MODE;
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
    if (gu8LastUIMode == UI_MENU_MODE) {
        if (g_bPlaying) {
            // Menu back to video playback
            if (Carimpl_GetTotalFiles() == 0) {
                PlayComplete();
            } else if (access(g_pstPlayStat->filename, F_OK) != 0) {
                PlayComplete();
            } else {
                toggle_pause(g_pstPlayStat);
                g_bPause = !g_bPause;
            }
        } else {
            // Menu back to photo playback
            unsigned int u32CurFileIdx = Carimpl_GetCurFileIdx();
            char *pszFileName = Carimpl_IsSubFileEnable() ?
                Carimpl_GetCurSubFileName(u32CurFileIdx) : Carimpl_GetCurFileName(u32CurFileIdx);

            if (pszFileName == NULL)
                pszFileName = Carimpl_GetCurFileName(u32CurFileIdx);
            if (pszFileName != NULL) {
                g_u8CurPlayCamId = Carimpl_GetCurCamId();
                PlayStart(pszFileName, PLAY_INIT_POS);
            } else {
                // No files.
                onButtonClick_sys_back(msys_backPtr);
            }
        }
    }

    carimpl.stUIModeInfo.u8Mode = UI_PLAYBACK_MODE;
    IPC_CarInfo_Write_UIModeInfo(&carimpl.stUIModeInfo);

    //register CB FUNC
    set_key_event_callback(on_key_callback);
    carimpl_set_colorkey(FB0, true);
#if defined(SUPPORT_FB1)
    carimpl_set_colorkey(FB1, true);
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
    printf("destroy player dev\n");
#ifdef SUPPORT_PLAYER_MODULE
    PlayStop();
    DestroyPlayerDev();
    g_firstPlayPos = PLAY_INIT_POS;

#if defined(SUPPORT_FB1)
    if (pWin) {
        if (pDurationBar) {
            delete pDurationBar;
        }

        if (pFileNameStr) {
            delete pFileNameStr;
        }

        if (pFileDurStr) {
            delete pFileDurStr;
        }

        delete pWin;
    }
#endif
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
    switch (id) {
    case 0:
    IPC_CarInfo_Read_SdInfo(&carimpl.stSdInfo);
    if (!IMPL_SD_ISINSERT) {
        PlayComplete();
        Carimpl_DcfUnmount();
    }
    break;
    case 1:
    if (player_is_complete(g_pstPlayStat) == TRUE) {
        PlayComplete();
    }
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
static bool onplayerActivityTouchEvent(const MotionEvent &ev) {
#ifdef SUPPORT_PLAYER_MODULE
    static SZKPoint touchDown;
    static SZKPoint touchMove;
    static SZKPoint lastMove;
    static bool bValidMove = false;	// on the first move, delt y should be larger than delt x, or update touchDown point

    switch (ev.mActionStatus) {
        case MotionEvent::E_ACTION_DOWN://触摸按下
            //LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
            //printf("down: time=%ld, x=%d, y=%d\n", ev.mEventTime, ev.mX, ev.mY);
            touchDown.x = ev.mX;
            touchDown.y = ev.mY;
            bValidMove = false;

            // show play bar when touch down
            AutoDisplayToolbar();

            break;
        case MotionEvent::E_ACTION_MOVE://触摸滑动
            //printf("move: time=%ld, x=%d, y=%d\n", ev.mEventTime, ev.mX, ev.mY);
            touchMove.x = ev.mX;
            touchMove.y = ev.mY;

            if (!bValidMove)
            {
                if (touchMove.y == touchDown.y)
                {
                    touchDown = touchMove;
                }
                else if (touchMove.x == touchDown.x)
                {
                    bValidMove = true;
                    AdjustVolumeByTouch(touchDown.y, touchMove.y);
                    lastMove = touchMove;
                }
                else if ((touchMove.y-touchDown.y) > 0 && (touchMove.x-touchDown.x) > 0
                        && (touchMove.y-touchDown.y) >= (touchMove.x-touchDown.x))
                {
                    bValidMove = true;
                    AdjustVolumeByTouch(touchDown.y, touchMove.y);
                    lastMove = touchMove;
                }
                else if ((touchMove.y-touchDown.y) < 0 && (touchMove.x-touchDown.x) < 0
                        && (touchMove.y-touchDown.y) <= (touchMove.x-touchDown.x))
                {
                    bValidMove = true;
                    AdjustVolumeByTouch(touchDown.y, touchMove.y);
                    lastMove = touchMove;
                }
                else if ((touchMove.y-touchDown.y) > 0 && (touchMove.x-touchDown.x) < 0
                        && (touchMove.y-touchDown.y) >= (touchDown.x-touchMove.x))
                {
                    bValidMove = true;
                    AdjustVolumeByTouch(touchDown.y, touchMove.y);
                    lastMove = touchMove;
                }
                else if ((touchMove.y-touchDown.y) < 0 && (touchMove.x-touchDown.x) > 0
                        && (touchDown.y-touchMove.y) >= (touchMove.x-touchDown.x))
                {
                    bValidMove = true;
                    AdjustVolumeByTouch(touchDown.y, touchMove.y);
                    lastMove = touchMove;
                }
                else
                {
                    touchDown = touchMove;
                }
            }
            else
            {
                //printf("lastY:%d, curY:%d\n", lastMove.y, touchMove.y);
                AdjustVolumeByTouch(lastMove.y, touchMove.y);
                lastMove = touchMove;
            }

            AutoDisplayToolbar();
            break;
        case MotionEvent::E_ACTION_UP:  //触摸抬起
            //printf("up: time=%ld, x=%d, y=%d\n", ev.mEventTime, ev.mX, ev.mY);
            break;
        default:
            break;
    }
#endif
    return false;
}
static void onProgressChanged_Seekbar_progress(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged Seekbar_progress %d !!!\n", progress);
}

static void onStartTrackingTouch_Seekbar_progress(ZKSeekBar *pSeekBar) {
    //LOGD(" ProgressChanged Seekbar_progress %d %d!\n", progress,g_bPause);
#ifdef SUPPORT_PLAYER_MODULE
    if (!g_bPause)
        toggle_pause(g_pstPlayStat);
#endif
}

static void onStopTrackingTouch_Seekbar_progress(ZKSeekBar *pSeekBar) {
    //LOGD(" ProgressChanged Seekbar_progress %d !!!\n", progress);
#ifdef SUPPORT_PLAYER_MODULE
    int progress = pSeekBar->getProgress();
    long long curPos = progress * g_duration / mSeekbar_progressPtr->getMax();
    printf("progress value is %d, max value is %d, duration is %lld, curPos is %lld,%d\n", progress, mSeekbar_progressPtr->getMax(),
            g_duration, curPos,g_bPause);
    stream_seek(g_pstPlayStat, curPos, 0, 0);

    if (!g_bPause)
        toggle_pause(g_pstPlayStat);
#endif
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    PlayComplete();
    if (Carimpl_GetCurDB() == DB_PHOTO) {
        gu8LastUIMode = UI_PLAYBACK_MODE;
        EASYUICONTEXT->goBack();
    }
    return true;
}

static bool onButtonClick_Button_play(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_play %d!\n",g_bPlaying);
#ifdef SUPPORT_PLAYER_MODULE
    if (g_bPlaying)
    {
        g_bPause = !g_bPause;
        toggle_pause(g_pstPlayStat);
        SetPlayingStatus(!g_bPause);
    }
#endif
    return false;
}

static bool onButtonClick_Button_stop(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_stop !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
    PlayComplete();
#endif
    return false;
}

static bool onButtonClick_Button_slow(ZKButton *pButton) {
#ifdef SUPPORT_PLAYER_MODULE
    char speedMode[16] = {0};
    if (g_bPlaying)
    {
        if (g_ePlayDirection == FAST_FORWARD && g_eSpeedMode != SPEED_NORMAL)
        {
            g_eSpeedMode = SPEED_NORMAL;
            mTextview_speedPtr->setText("Normal");
            set_speed(g_pstPlayStat, (int)g_eSpeedMode, FAST_FORWARD);
        }
        else
        {
            g_eSpeedMode = (g_eSpeedMode + 1) % SPEED_NUM;
            if (g_eSpeedMode != SPEED_NORMAL)
            {
                sprintf(speedMode, "%dX <<", 1 << g_eSpeedMode);
                mTextview_speedPtr->setText(speedMode);
            }
            else
            {
                mTextview_speedPtr->setText("Normal");
            }
            set_speed(g_pstPlayStat, (int)g_eSpeedMode, FAST_BACKWARD);
            g_ePlayDirection = FAST_BACKWARD;
        }
    }
#endif
    return false;
}

static bool onButtonClick_Button_fast(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_fast !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
    char speedMode[16] = {0};

    if (g_bPlaying)
    {
        if (g_ePlayDirection == FAST_BACKWARD && g_eSpeedMode != SPEED_NORMAL)
        {
            g_eSpeedMode = SPEED_NORMAL;
            mTextview_speedPtr->setText("Normal");
            set_speed(g_pstPlayStat, (int)g_eSpeedMode, FAST_FORWARD);
        }
        else
        {
            g_eSpeedMode = (g_eSpeedMode + 1) % SPEED_NUM;
            if (g_eSpeedMode != SPEED_NORMAL)
            {
                sprintf(speedMode, "%dX >>", 1 << g_eSpeedMode);
                mTextview_speedPtr->setText(speedMode);
            }
            else
            {
                mTextview_speedPtr->setText("Normal");
            }
            set_speed(g_pstPlayStat, (int)g_eSpeedMode, FAST_FORWARD);
            g_ePlayDirection = FAST_FORWARD;
        }
    }
#endif
    return false;
}
static bool onButtonClick_Button_voice(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_voice !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
    g_bMute = !g_bMute;
    MI_AO_SetMute(AUDIO_DEV, g_bMute, g_bMute, E_MI_AO_GAIN_FADING_OFF);
    SetMuteStatus(g_bMute);
    printf("set mute to %d\n", g_bMute);
#endif
    return false;
}

static void onProgressChanged_Seekbar_volumn(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged Seekbar_volumn %d !!!\n", progress);
#ifdef SUPPORT_PLAYER_MODULE
    MI_S32 vol = 0;
    MI_AO_Attr_t stAttr;

    g_s32VolValue = GetPlayerVolumn();
    if (g_s32VolValue)
        vol = g_s32VolValue * (MAX_AO_VOLUME - MIN_AO_VOLUME) / 100 + MIN_AO_VOLUME;
    else
        vol = MIN_AO_VOLUME;
    printf("voice changed [%d]\n", vol);

    if (MI_SUCCESS == MI_AO_GetAttr(AUDIO_DEV, &stAttr))
    {
        MI_AO_SetVolume(AUDIO_DEV, vol, vol, E_MI_AO_GAIN_FADING_64_SAMPLE);
        MI_AO_SetMute(AUDIO_DEV, g_bMute, g_bMute, E_MI_AO_GAIN_FADING_OFF);
    }
#endif
}

static bool onButtonClick_Btn_NextFile(ZKButton *pButton) {
    LOGD(" ButtonClick Btn_NextFile !!!\n");
    PlayStop();
    PlayStart(E_PLAY_NEXT);
    return false;
}

static void onVideoViewPlayerMessageListener_Videoview_video(ZKVideoView *pVideoView, int msg) {
    switch (msg) {
    case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_STARTED:
        break;
    case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_COMPLETED:
        break;
    case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_ERROR:
        break;
    }
}
