/*
 * ZKPageWindow.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: Jul 27, 2023
 *      Author: zkswe@zkswe.com
 */

#ifndef _WINDOW_ZKPAGEWINDOW_H_
#define _WINDOW_ZKPAGEWINDOW_H_

#include "ZKWindow.h"

class ZKPageWindowPrivate;

/**
 * @brief 翻页窗口控件
 */
class ZKPageWindow : public ZKWindow {
	ZK_DECLARE_PRIVATE(ZKPageWindow)

public:
	ZKPageWindow(ZKBase *pParent);
	virtual ~ZKPageWindow();

	/**
	 * @brief 翻页监听接口
	 */
	class IPageChangeListener {
	public:
		virtual ~IPageChangeListener() { }
		virtual void onPageChange(ZKPageWindow *pPageWindow, int page) = 0;
	};

	void setPageChangeListener(IPageChangeListener *pListener);

	/**
	 * @brief 获取页数
	 */
	int getPageSize() const;

	/**
	 * @brief 设置当前页位置
	 */
	void setCurrentPage(int page);

	/**
	 * @brief 获取当前页位置
	 */
	int getCurrentPage() const;

	/**
	 * @brief 切换到下一页
	 * @param isAnimatable 是否开启翻页动画，默认为false，不开启动画
	 */
	void turnToNextPage(bool isAnimatable = false);

	/**
	 * @brief 切换到上一页
	 * @param isAnimatable 是否开启翻页动画，默认为false，不开启动画
	 */
	void turnToPrevPage(bool isAnimatable = false);

protected:
	ZKPageWindow(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual void onAfterCreateWindow(const Json::Value &json);

	virtual const char* getClassName() const { return ZK_PAGEWINDOW; }
	virtual bool onTouchEvent(const MotionEvent &ev);
	virtual void onTimer(int id);

private:
	void parsePageWindowAttributeFromJson(const Json::Value &json);
};

#endif /* _WINDOW_ZKPAGEWINDOW_H_ */
