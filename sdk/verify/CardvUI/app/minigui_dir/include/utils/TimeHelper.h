/*
 * TimeHelper.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: May 22, 2017
 *      Author: zkswe@zkswe.com
 */

#ifndef _UTILS_TIME_HELPER_H_
#define _UTILS_TIME_HELPER_H_

#include <time.h>

/**
 * @brief 时间操作帮助类
 */
class TimeHelper {
public:
	/**
	 * @brief 获取当前日期时间，单位为毫秒
	 */
	static long long getCurrentTime();

	/**
	 * @brief 获取系统运行时间，单位为毫秒
	 */
	static long long getSystemRunTime();

	/**
	 * @brief 获取当前日期
	 */
	static struct tm* getDateTime();

	/**
	 * @brief 设置日期
	 */
	static bool setDateTime(struct tm *ptm);

	/**
	 * @brief 设置日期
	 * @param pDate 格式：2017-09-13 16:00:00
	 */
	static bool setDateTime(const char *pDate);
};

#endif /* _UTILS_TIME_HELPER_H_ */
