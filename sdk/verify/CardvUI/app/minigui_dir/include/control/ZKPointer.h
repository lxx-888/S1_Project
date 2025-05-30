/*
 * ZKPointer.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: Jul 17, 2017
 *      Author: zkswe@zkswe.com
 */

#ifndef _CONTROL_ZKPOINTER_H_
#define _CONTROL_ZKPOINTER_H_

#include "ZKBase.h"

class ZKPointerPrivate;

/**
 * @brief 指针控件
 */
class ZKPointer : public ZKBase {
	ZK_DECLARE_PRIVATE(ZKPointer)

public:
	ZKPointer(ZKBase *pParent);
	virtual ~ZKPointer();

	/**
	 * @brief 设置旋转角度
	 */
	void setTargetAngle(float angle);

	/**
	 * @brief 设置指针图片
	 */
	void setPointerPic(const char *pPicPath);

protected:
	ZKPointer(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual const char* getClassName() const { return ZK_POINTER; }

	virtual bool onInterceptMessage(const struct _message_t *pMsg);
	virtual void onDraw(ZKCanvas *pCanvas);
	virtual void onTimer(int id);

private:
	void parsePointerAttributeFromJson(const Json::Value &json);
};

#endif /* _CONTROL_ZKPOINTER_H_ */
