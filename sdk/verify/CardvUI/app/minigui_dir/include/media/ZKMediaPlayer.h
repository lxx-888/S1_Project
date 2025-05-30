/*
 * ZKMediaPlayer.h - Zkswe
 *
 * Copyright (C) 2018 Zkswe Technology Corp.
 *
 *  Created on: Mar 6, 2018
 *      Author: zkswe@zkswe.com
 */

#ifndef _MEDIA_ZKMEDIAPLAYER_H_
#define _MEDIA_ZKMEDIAPLAYER_H_

#include "system/Thread.h"
#include "utils/MessageQueue.h"
#include "control/Common.h"

class InternalMediaPlayer;

/**
 * @brief 媒体播放
 */
class ZKMediaPlayer {
	friend class InternalMediaPlayer;
public:
	/**
	 * @brief 媒体类型
	 */
	typedef enum {
		E_MEDIA_TYPE_AUDIO,	/**< 音频类型 */
		E_MEDIA_TYPE_VIDEO	/**< 视频类型 */
	} EMediaType;

	typedef enum {
		E_MSGTYPE_PLAY_FILE = 1,
		E_MSGTYPE_PAUSE_FILE,
		E_MSGTYPE_RESUME_FILE,
		E_MSGTYPE_SEEK_FILE,
		E_MSGTYPE_STOP_FILE,
		E_MSGTYPE_PLAY_STARTED,
		E_MSGTYPE_PLAY_COMPLETED,
		E_MSGTYPE_ERROR_MEDIA_ERROR,  	// MEDIA_ERROR_UNKNOWN, ...
		E_MSGTYPE_ERROR_UNKNOWN,
		E_MSGTYPE_ERROR_INVALID_FILEPATH,
		E_MSGTYPE_ERROR_PREPARE_FAIL,
		E_MSGTYPE_ERROR_PLAY_FAIL,
		E_MSGTYPE_QUIT_THREAD,
	} EMessageType;

	typedef struct {			// for MSGTYPE_ERROR_MEDIA_ERROR
		int mediaErrorType;		// enum media_error_type
		int extra;
	} SMediaErrorInfo;

public:
	/**
	 * @param mediaType 媒体类型
	 */
	ZKMediaPlayer(EMediaType mediaType);
	virtual ~ZKMediaPlayer();

	/**
	 * @brief 播放指定路径媒体文件
	 * @param pFilePath 媒体文件路径
	 * @param msec 指定从哪个位置开始播放，单位为毫秒，默认从头开始播放
	 */
	void play(const char *pFilePath, int msec = 0);

	/**
	 * @brief 暂停播放
	 */
	void pause();

	/**
	 * @brief 恢复播放
	 */
	void resume();

	/**
	 * @brief 定位到msec位置播放
	 * @param msec 单位为毫秒
	 */
	void seekTo(int msec);

	/**
	 * @brief 停止播放
	 */
	void stop();

	/**
	 * @brief 是否播放中
	 */
	bool isPlaying();

	/**
	 * @brief 设置音量
	 * @param volume 音量范围：0.0 ～ 1.0
	 */
	void setVolume(float volume);

	/**
	 * @brief 设置增益
	 * @param gain 值范围：>= 0
	 */
	void setGain(float gain);

	void setPreviewPos(const LayoutPosition &pos);

	/* clockwise rotation: val=0 no rotation, val=1 90 degree; val=2 180 degree, val=3 270 degree */
	void setRotation(int val);

	/**
	 * @brief 获取总时长，单位为毫秒
	 */
	int getDuration();

	/**
	 * @brief 获取当前播放位置，单位为毫秒
	 */
	int getCurrentPosition();

	class IPlayerMessageListener {
	public:
		virtual ~IPlayerMessageListener() { }
		virtual void onPlayerMessage(ZKMediaPlayer *pMediaPlayer, int msg, void *pMsgData) = 0;
	};

	void setPlayerMessageListener(IPlayerMessageListener *pListener) {
		mPlayerMessageListenerPtr = pListener;
	}

private:
	bool playerThreadLoop();

	enum EPlayState {
		E_PLAY_STATE_IDLE = 0,
		E_PLAY_STATE_STARTING = 1,
		E_PLAY_STATE_STARTED = 2,
		E_PLAY_STATE_PAUSED = 3,
	};

	class PlayerThread : public Thread {
	public:
		PlayerThread(ZKMediaPlayer &mp) : mPlayer(mp) { }

		void start();
		void stop();
		virtual bool threadLoop();

	private:
		ZKMediaPlayer &mPlayer;
	};

private:
	PlayerThread mPlayerThread;

	EMediaType mMediaType;
	InternalMediaPlayer *mMediaPlayerPtr;

	MessageQueue mMsgQueue;

	mutable Mutex mLock;
	Condition mExecuteDoneCondition;

	EPlayState mCurrentPlayState;
	IPlayerMessageListener *mPlayerMessageListenerPtr;
};

#endif  /* _MEDIA_ZKMEDIAPLAYER_H_ */
