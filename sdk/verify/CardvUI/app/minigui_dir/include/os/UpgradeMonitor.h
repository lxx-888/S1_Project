/*
 * UpgradeMonitor.h - Zkswe
 *
 * Copyright (C) 2017 Zkswe Technology Corp.
 *
 *  Created on: Aug 4, 2017
 *      Author: zkswe@zkswe.com
 */

#ifndef _OS_UPGRADE_MONITOR_H_
#define _OS_UPGRADE_MONITOR_H_

#include <stdint.h>
#include <string>
#include <vector>
#include "MountMonitor.h"

class UpgradeMonitor : Thread {
public:
	virtual ~UpgradeMonitor();

	void startMonitoring();
	void stopMonitoring();

	bool startUpgrade();
	void stopUpgrade();

	bool isUpgrading() const;

	bool checkUpgrade();
	bool checkUpgradeFile(const char *pDir);

	static UpgradeMonitor* getInstance();

public:
	typedef enum {
		E_UPGRADE_STATUS_START,
		E_UPGRADE_STATUS_END,
	} EUpgradeStatus;

	typedef enum {
		E_UPGRADE_UBOOT,
		E_UPGRADE_BOOT,
		E_UPGRADE_SYSTEM,
		E_UPGRADE_RES,
		E_UPGRADE_DATA,
		E_UPGRADE_LOGO,
		E_UPGRADE_ENV,
		E_UPGRADE_PRIVATE,
		E_UPGRADE_FULL
	} ESystemUpgradeType;

	typedef struct {
		uint8_t partn;
		uint8_t reserve[3];
		uint8_t offset[4];
		uint8_t imageSize[4];
		uint8_t imageHead[16];
	} SPartInfo;

	typedef struct {
		uint8_t size[4];
		uint8_t perm;
		uint8_t type[4];
		uint8_t flag;
		uint8_t attr;
		uint8_t reserve[509];
		uint8_t crc32[4];
	} SExtendInfo;

	typedef struct {
		ESystemUpgradeType upgradeType;
		std::string upgradeFilePath;
		std::string partName;
		SPartInfo partInfo;
		bool needUpgrade;
	} SSystemUpgradeInfo;

	typedef struct {
		uint8_t hsize;
		uint8_t type;
		uint8_t flags;
		uint8_t reserve;
		uint8_t version[4];
		uint8_t pix[4];
		uint8_t date[4];
		uint8_t dataLen[4];
		uint8_t hmd5[16];
		uint8_t *pData;
		bool needUpgrade;
	} STSUpgradeInfo;

	class IUpgradeStatusListener {
	public:
		virtual ~IUpgradeStatusListener() { };
		virtual void notify(int what, int status, const char *msg) = 0;
	};

	void setUpgradeStatusListener(IUpgradeStatusListener *pListener) {
		mUpgradeStatusListenerPtr = pListener;
	}

protected:
	virtual bool threadLoop();

private:
	UpgradeMonitor();

	bool checkTouchCalib(const char *pPath);
	bool checkRestart(const char *pPath);

private:
	class UpgradeMountListener : public MountMonitor::IMountListener {
	public:
		UpgradeMountListener(UpgradeMonitor *pUM) : mUMPtr(pUM) {

		}

		virtual void notify(int what, int status, const char *msg) {
			switch (status) {
			case MountMonitor::E_MOUNT_STATUS_MOUNTED:
				if (mUMPtr->checkRestart(msg) ||
					mUMPtr->checkTouchCalib(msg) ||
					mUMPtr->checkUpgradeFile(msg)) {
					// do nothing
				}
				break;

			case MountMonitor::E_MOUNT_STATUS_REMOVE:
				break;
			}
		}

	private:
		UpgradeMonitor *mUMPtr;
	};

private:
	UpgradeMountListener mUpgradeMountListener;
	IUpgradeStatusListener *mUpgradeStatusListenerPtr;

	bool mHasStartMonitor;

	std::vector<SSystemUpgradeInfo*> mSystemUpgradeInfoList;
	std::string mUpgradeDir;

	bool mIsUpgrading;

	typedef struct {
		std::string dev;
		std::string name;
	} SMtdInfo;
	std::vector<SMtdInfo> mMtdInfoList;

	SExtendInfo mExtendInfo;
	STSUpgradeInfo mTSUpgradeInfo;

	const char *mErrorCode;
};

#define UPGRADEMONITOR		UpgradeMonitor::getInstance()

#endif /* _OS_UPGRADE_MONITOR_H_ */
