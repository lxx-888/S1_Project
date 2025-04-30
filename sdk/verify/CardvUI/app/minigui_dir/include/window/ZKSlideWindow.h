/*
 * ZKSlideWindow.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: Jul 3, 2017
 *      Author: zkswe@zkswe.com
 */

#ifndef _WINDOW_ZKSLIDEWINDOW_H_
#define _WINDOW_ZKSLIDEWINDOW_H_

#include "ZKWindow.h"

class ZKSlideWindowPrivate;

/**
 * @brief 滑动窗口控件
 */
class ZKSlideWindow : public ZKWindow {
	ZK_DECLARE_PRIVATE(ZKSlideWindow)

public:
	ZKSlideWindow(ZKBase *pParent);
	virtual ~ZKSlideWindow();

public:
	/**
	 * @brief 滑动项点击监听接口
	 */
	class ISlideItemClickListener {
	public:
		virtual ~ISlideItemClickListener() { }
		virtual void onSlideItemClick(ZKSlideWindow *pSlideWindow, int index) = 0;
	};

	void setSlideItemClickListener(ISlideItemClickListener *pListener);

	/**
	 * @brief 翻页监听接口
	 */
	class ISlidePageChangeListener {
	public:
		virtual ~ISlidePageChangeListener() { }
		virtual void onSlidePageChange(ZKSlideWindow *pSlideWindow, int page) = 0;
	};

	void setSlidePageChangeListener(ISlidePageChangeListener *pListener);

	/**
	 * @brief 获取行数
	 */
	uint32_t getRows() const;

	/**
	 * @brief 获取列数
	 */
	uint32_t getCols() const;

	/**
	 * @brief 获取子项数
	 */
	int getItemSize() const;

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

	/**
	 * @brief 设置子项状态背景图
	 * @param index  子项索引
	 * @param status 状态
	 *    正常状态： ZK_CONTROL_STATUS_NORMAL
	 *    按下状态： ZK_CONTROL_STATUS_PRESSED
	 *    选中状态： ZK_CONTROL_STATUS_SELECTED
	 *    选中按下状态： ZK_CONTROL_STATUS_PRESSED | ZK_CONTROL_STATUS_SELECTED
	 *    无效状态： ZK_CONTROL_STATUS_INVALID
	 * @param pPicPath 图片路径
	 */
	void setItemStatusPic(int index, int status, const char *pPicPath);

	/**
	 * @brief 设置子项文本状态颜色
	 * @param index  子项索引
	 * @param status 状态
	 *    正常状态： ZK_CONTROL_STATUS_NORMAL
	 *    按下状态： ZK_CONTROL_STATUS_PRESSED
	 *    选中状态： ZK_CONTROL_STATUS_SELECTED
	 *    选中按下状态： ZK_CONTROL_STATUS_PRESSED | ZK_CONTROL_STATUS_SELECTED
	 *    无效状态： ZK_CONTROL_STATUS_INVALID
	 * @param color 颜色值为0x ARGB
	 */
	void setItemTextStatusColor(int index, int status, uint32_t color);

	/**
	 * @brief 设置子项选中状态
	 */
	void setItemSelected(int index, bool isSelected);

	/**
	 * @brief 设置子项文本
	 */
	void setItemText(int index, const char *text);

	/**
	 * @brief 设置文本大小
	 */
	void setItemTextSize(uint32_t size);

	/**
	 * @brief 创建子项 (仅UI主线程中能调用)
	 * @param index 在该位置创建子项, -1表示在末尾处创建
	 * @return 返回子项索引位置, -1表示创建失败
	 */
	int createItem(int index = -1);

	/**
	 * @brief 删除子项 (仅UI主线程中能调用)
	 */
	void deleteItem(int index);

protected:
	ZKSlideWindow(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual const char* getClassName() const { return ZK_SLIDEWINDOW; }

	virtual void onDraw(ZKCanvas *pCanvas);
	virtual bool onTouchEvent(const MotionEvent &ev);
	virtual void onTimer(int id);

private:
	void parseSlideWindowAttributeFromJson(const Json::Value &json);
};

#endif /* _WINDOW_ZKSLIDEWINDOW_H_ */
