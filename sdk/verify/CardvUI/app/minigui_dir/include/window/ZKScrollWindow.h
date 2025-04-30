/*
 * ZKScrollWindow.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: Sep 5, 2023
 *      Author: zkswe@zkswe.com
 */

#ifndef _WINDOW_ZKSCROLLWINDOW_H_
#define _WINDOW_ZKSCROLLWINDOW_H_

#include "ZKWindow.h"

class ZKScrollWindowPrivate;

/**
 * @brief 滚动窗口控件
 */
class ZKScrollWindow : public ZKWindow {
	ZK_DECLARE_PRIVATE(ZKScrollWindow)

public:
	ZKScrollWindow(ZKBase *pParent);
	virtual ~ZKScrollWindow();

protected:
	ZKScrollWindow(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual void onAfterCreateWindow(const Json::Value &json);

	virtual const char* getClassName() const { return ZK_SCROLLWINDOW; }
	virtual bool onTouchEvent(const MotionEvent &ev);
	virtual void onTimer(int id);

private:
	void parseScrollWindowAttributeFromJson(const Json::Value &json);
};

#endif /* _WINDOW_ZKSCROLLWINDOW_H_ */
