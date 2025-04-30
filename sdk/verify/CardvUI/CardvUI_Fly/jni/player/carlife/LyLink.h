

#ifndef __LY_LINK_H__
#define __LY_LINK_H__


namespace lylink
{
	enum LinkType_e{
		LINK_TYPE_NULL = 0,
		LINK_TYPE_WIFICP,
		LINK_TYPE_USBCP,
		LINK_TYPE_USBAUTO,
		LINK_TYPE_WIFIAUTO,
		LINK_TYPE_USBLIFE,
		LINK_TYPE_WIFILIFE,
	};

	enum DrivingMode_e
	{
		DrivingMode_LeftHand = 0, 
		DrivingMode_RightHand
	};

	enum WifiFrequency_e
	{
		WifiFreq_2_4GHz = 0,
		WifiFreq_5GHz
	};

	enum TouchMode_e
	{
		TouchMode_Down = 0,
		TouchMode_Up,
		TouchMode_Move
	};

	enum ErrCode_e
	{
		ErrCode_None = 0,
		ErrCode_mFi = 1,  // Carplay加密芯片读取失败
		ErrCode_Verify = 2,  // 授权校验失败，请激活
		ErrCode_NoVideo = 3,  // 无画面
	};

	enum KeyCode_CarPlay
	{
		ConsumerActionACHome = 0x100,
		ConsumerActionACBack,
		ConsumerActionRecord,
		ConsumerActionScanNext,
		ConsumerActionScanPrev,
		ConsumerActionPlay,
		ConsumerActionPause,
		ConsumerActionPlayOrPause,
	
		TelePhonyActionHold = 0x200,
		TelePhonyActionDial,
		TelePhonyActionHang,
		TelePhonyActionMute,
		TelePhonyActionPhoneKey0,
		TelePhonyActionPhoneKey1,
		TelePhonyActionPhoneKey2,
		TelePhonyActionPhoneKey3,
		TelePhonyActionPhoneKey4,
		TelePhonyActionPhoneKey5,
		TelePhonyActionPhoneKey6,
		TelePhonyActionPhoneKey7,
		TelePhonyActionPhoneKey8,
		TelePhonyActionPhoneKey9,
		TelePhonyActionPhoneKeyStar,
		TelePhonyActionPhoneKeyPound,
		TelePhonyActionKeyboardDelete,

		VRAction = 0x300,
		VRLongPress,

		knobActionLeft = 0x400,
		knobActionRight,
		knobActionUp,
		knobActionDown,
		knobActionSelect,
		knobActionHome,
		knobActionBack,
		knobActionCounterClockWise,  // 逆时针旋转，不区分按下和抬起动作
		knobActionClockWise,  // 顺时针旋转，不区分按下和抬起动作
	};

	class ILyLink
	{
	public:
		struct InitData_t
		{
			DrivingMode_e driving;
			unsigned short width;
			unsigned short height;		
			char iconname[24];
			LinkType_e usb_android;
		};

	public:
		class Callback
		{
		public:
			virtual ~Callback() {}
			
			/**
			 *	\brief 手机互联连接成功的回调函数
			 *
			 *	\param type 手机互联类型
			 */
			virtual void OnLinkEstablished(LinkType_e type) = 0;

			/**
			 *	\brief 手机互联断开连接的回调函数
			 *
			 *	\param type 手机互联类型
			 */
			virtual void OnLinkDisconnect(LinkType_e type) = 0;

			/**
			 *	\brief 请求视频流焦点的回调函数
			 *
			 *	\param bNative true 请求进入HU主界面，false 请求进入手机互联
			 *      请求进入HU主界面时，请显示本地UI；请求进入手机互联时，如果允许，
			 *		需要调用ChangeVideoFocus(false)，不允许则调用ChangeVideoFocus(true)
			 */
			virtual void OnVideoFocusChanged(bool bNative) = 0;

			/**
			 *	\brief 视频流传输开始的回调函数
			 *
			 *	\param width 视频流的宽度，单位：像素
			 *	\param height 视频流的高度，单位：像素
			 */
			virtual void OnVideoStart(int width, int height) = 0;

			/**
			 *	\brief 视频流传输停止的回调函数
			 */
			virtual void OnVideoStop() = 0;

			/**
			 *	\brief 主声道音频流传输开始的回调函数
			 */
			virtual void OnMainAudioStart() = 0;

			/**
			 *	\brief 主声道音频流传输停止的回调函数
			 */
			virtual void OnMainAudioStop() = 0;

			/**
			 *	\brief 辅声道音频流传输开始的回调函数
			 */
			virtual void OnAltAudioStart() = 0;

			/**
			 *	\brief 辅声道音频流传输停止的回调函数
			 */
			virtual void OnAltAudioStop() = 0;

			/**
			 *	\brief 录音开始的回调函数
			 */
			virtual void OnMicAudioStart() = 0;

			/**
			 *	\brief 录音结束的回调函数
			 */			
			virtual void OnMicAudioStop() = 0;

			/**
			 *	\brief 手机正在连接的回调函数
			 *
			 *	\param type 手机互联类型
			 *	\param name 手机名称
			 */
			virtual void OnPhoneConnecting(LinkType_e type, const char *name) = 0;

			/**
			 *	\brief 手机连接断开的回调函数
			 *
			 *	\param type 手机互联类型
			 *	\param type 手机名称
			 */
			virtual void OnPhoneDisconnect(LinkType_e type, const char *name) = 0;

			/**
			 *	\brief 手机通话状态变化的回调函数
			 *
			 *	\param Calling 是否进入通话
			 */
			virtual void OnPhoneCalling(bool Calling) = 0;
			
			/**
			 *	\brief 内部错误的回调函数
			 *
			 *	\param 错误码
			 */
			virtual void OnErrorOccur(ErrCode_e code) = 0;
		};

	public:
		virtual ~ILyLink() {}

		/**
		 *	\brief 设置回调函数
		 *
		 *	\param cb 注：该回调函数会在实例析构时自动释放
		 */
		virtual void SetCallback(Callback *cb) = 0;

		/**
		 *	\brief 启动手机互联服务
		 *
		 *  \return true 服务开启成功，false 服务开启失败
		 */	
		virtual bool Start() = 0;

		/**
		 *	\brief 设置连接参数
		 *
		 *	\param data 连接参数
		 *
		 *  \return true 设置成功，false 设置失败。
		 *      注：服务开启过程中，可能会设置失败，程序将自动在服务开启后重新设置一次。
		 *			必须在Start()之前设置分辨率宽度和高度。
		 */
		virtual bool SetParams(const InitData_t &data) = 0;

		/**
		 *	\brief 改变手机互联的视频焦点
		 *
		 *	\param bNative true 返回HU主界面，false 进入手机互联
		 *
		 *  \return true 设置成功，false 设置失败。
		 */
		virtual bool ChangeVideoFocus(bool bNative) = 0;

		/**
		 *	\brief 改变手机互联的音频焦点
		 *
		 *	\param bNative true 播放本地声音，false 播放手机互联声音
		 *
		 *  \return true 设置成功，false 设置失败。
		 */
		virtual bool ChangeAudioFocus(bool bNative) = 0;

		/**
		 *	\brief 发送触摸事件
		 *
		 *	\param x 相对于屏幕的横轴坐标
		 *	\param y 相对于屏幕的竖轴坐标
		 *	\param mode 触摸事件类型，包括按下，抬起和移动
		 *
		 *  \return true 发送成功，false 发送失败。
		 */
		virtual bool SendTouchEvent(int x, int y, TouchMode_e mode) = 0;

		/**
		 *	\brief 发送按键事件
		 *
		 *	\param keycode 	按键键值。Carplay参考KeyCode_CarPlay
		 *	\param isDown 	按键是否按下。carlife不区分按下还是抬起。
		 *  \return true 发送成功，false 发送失败。
		 */
		virtual bool SendKey(int keycode, bool isDown = true) = 0;	

		/**
		 *	\brief 通知主程序热点已打开，一般在收到苹果手机连接无线Carplay消息时调用该接口
		 *
		 *	\param ssid 热点名，可以为空
		 *	\param pwd 热点密码，可以为空
		 *	\param mode 频段 2.4G或者是5G
		 *
		 *  \return true 通知成功，false 通知失败。
		 */
		virtual bool NotifyWifiApOpened(const char ssid[32] = nullptr, const char pwd[64] = nullptr, WifiFrequency_e mode = WifiFreq_5GHz) = 0;
		
		/**
		 *	\brief 获取版本号字符串
		 *
		 *  \return 版本号字符串，参考 LyLink_Version
		 */
		virtual const char *GetVersion() = 0;

	public:
		/**
		 *	\brief 本程序为单例模式，请使用该接口获取实例
		 *
		 *  \return 接口实例指针
		 */
		static ILyLink *getInstance();
	};

};


#define LyLink_Version	"1.01.00"


#endif
