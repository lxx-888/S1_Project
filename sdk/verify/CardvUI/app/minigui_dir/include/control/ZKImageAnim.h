/*
 * ZKImageAnim.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: Sep 11, 2023
 *      Author: zkswe@zkswe.com
 */

#ifndef _CONTROL_ZKIMAGEANIM_H_
#define _CONTROL_ZKIMAGEANIM_H_

#include "ZKBase.h"

class ZKImageAnimPrivate;

/**
 * @brief 图像动画控件
 */
class ZKImageAnim : public ZKBase {
	ZK_DECLARE_PRIVATE(ZKImageAnim)

public:
	ZKImageAnim(ZKBase *pParent);
	virtual ~ZKImageAnim();

	/**
	 * @brief 播放
	 */
	void play(const std::string &file);

	/**
	 * @brief 停止播放
	 */
	void stop();

	/**
	 * @brief 暂停播放
	 */
	void pause();

	/**
	 * @brief 恢复播放
	 */
	void resume();

	/**
	 * @brief 设置循环次数
	 * @param count <=0 持续循环播放, >0 播放count次后停止播放
	 */
	void setLoopCount(int count);

	/**
	 * @brief 获取图像宽度
	 */
	int getImageWidth() const;

	/**
	 * @brief 获取图像高度
	 */
	int getImageHeight() const;

public:
	class IImageAnimStatusListener {
	public:
		virtual ~IImageAnimStatusListener() { }
		virtual void onAnimStarted(ZKImageAnim *pImageAnim) = 0;
		virtual void onAnimEnd(ZKImageAnim *pImageAnim) = 0;
		virtual void onAnimProgress(ZKImageAnim *pImageAnim, int progress) { }
	};

	void setStatusListener(IImageAnimStatusListener *pListener);

protected:
	ZKImageAnim(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual void onAfterCreateWindow(const Json::Value &json);
	virtual const char* getClassName() const { return ZK_IMAGEANIM; }

	virtual bool onInterceptMessage(const struct _message_t *pMsg);
	virtual void onDraw(ZKCanvas *pCanvas);
	virtual void onTimer(int id);

private:
	void parseImageAnimAttributeFromJson(const Json::Value &json);
};

#endif /* _CONTROL_ZKIMAGEANIM_H_ */
