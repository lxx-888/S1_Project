#ifdef SUPPORT_CARLIFE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "../../carimpl/carimpl.h"
#include "LyLink.h"
#include "../../carimpl/KeyEventContext.h"
#include "utils/ScreenHelper.h"


#define COLOR_NONE          "\033[0m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_GREEN         "\033[0;32m"

#define DEBUG(fmt, ...)		printf (COLOR_GREEN "LylinkCB %s:" fmt COLOR_NONE "\n", __FUNCTION__, ##__VA_ARGS__);


using namespace lylink;
static bool gbIsShow;
static bool gbIsCnnt;
static int g32width;
static int g32height;
static int g32Fbwidth;
static int g32Fbheight;
extern void onVideoModeShowIcon(bool show);
int ParseWifiSet(const char *file, char *SSID, char *PWD);

class LylinkCB : public ILyLink::Callback
{
public:
	LylinkCB() {
		bStationMode = true;
	}
	
public:
	void OnLinkEstablished(LinkType_e type) override
	{
		DEBUG("type = %d", type);
        gbIsCnnt = true;
        if (type == LINK_TYPE_WIFICP) {
            
        }
	}
	
	void OnLinkDisconnect(LinkType_e type) override
	{
		DEBUG("type = %d", type);
        gbIsCnnt = false;
		onVideoModeShowIcon(TRUE);

        //BT discnt first, and then wifi discnt.
		if (type == LINK_TYPE_WIFICP) {
			if (!bStationMode) {
				bStationMode = true;
                
				/* ap_carplay.sh stop */
				system("killall hostapd udhcpd");
			
				// system("wpa_supplicant -B -Dnl80211 -iwlan0 -c/customer/wifi/wpa_supplicant.conf");
				system("/customer/carlife/sta_carlife.sh start");
			}
		}
	}
	
	void OnVideoFocusChanged(bool bNative) override
	{
		DEBUG("%s", bNative ? "native" : "lylink");
		/*
		一般需要发送，所以已移到.so中
		ILyLink::getInstance()->ChangeVideoFocus(bNative);
		*/
	}
	
	void OnVideoStart(int width, int height) override
    {
    	DEBUG("%dx%d", width, height);
        IPC_CarInfo_Read_DispInfo(&carimpl.stDispInfo);
        Carimpl_Send2Fifo(CARDV_CMD_START_PLAYBACK, sizeof(CARDV_CMD_START_PLAYBACK));
        usleep(100000); // TBD : Wait for cardv create playback pipe.
        gbIsShow = TRUE;
        g32width = width;
        g32height = height;
        g32Fbheight = ScreenHelper::getScreenHeight();
        g32Fbwidth = ScreenHelper::getScreenWidth();
    }

	void OnVideoStop() override
	{
		DEBUG("enter");
        Carimpl_Send2Fifo(CARDV_CMD_STOP_PLAYBACK, sizeof(CARDV_CMD_STOP_PLAYBACK));
        gbIsShow = FALSE;
	}
	
	void OnMainAudioStart() override
	{
		DEBUG("enter");
	}
	
	void OnMainAudioStop() override
	{
		DEBUG("enter");
	}
	
	void OnAltAudioStart() override
	{
		DEBUG("enter");
	}
	
	void OnAltAudioStop() override
	{
		DEBUG("enter");
	}
	
	void OnMicAudioStart() override
	{
		DEBUG("enter");
	}
	
	void OnMicAudioStop() override
	{
		DEBUG("enter");
	}
	
    void OnPhoneConnecting(LinkType_e type, const char *name) override
    {
        char ssid[64] = "CARPLAY_8826";
        char pwd[64] = "88888888";
        
        DEBUG("type = %d, name = %s", type, name);
        
        if (type == LINK_TYPE_WIFICP) {
            
			if (bStationMode) {
				bStationMode = false;

				// system("killall wpa_supplicant");
				system("/customer/carlife/sta_carlife.sh stop");

				/* ap_carplay.sh start */
				system("/customer/carlife/ap_carplay.sh start");
			}
			ParseWifiSet("/customer/wifi/net_config.bin",ssid,pwd);
            DEBUG("ssid %s,pwd %s", ssid,pwd);
			/* ssid & passwd from hostapd.conf */
            ILyLink::getInstance()->NotifyWifiApOpened(ssid, pwd);
        }
    }

	void OnPhoneDisconnect(LinkType_e type, const char *name) override
	{
		DEBUG("type = %d, name = %s", type, name);
        if (type == LINK_TYPE_WIFICP) {

        }
	}
	
	void OnPhoneCalling(bool Calling) override
	{
		DEBUG("%s", Calling ? "Calling" : "HangUp");
	}

	void OnErrorOccur(ErrCode_e code) override 
	{
		DEBUG("%d", code);
	}

private:
	bool bStationMode;
};

int ParseWifiSet(const char *file, char *SSID, char *PWD)
{
    FILE *fp = NULL;
    char file_buf[128] = {0};
    int cnt = 0;

    //printf("open:%s\n",file);
    fp = fopen(file, "r");
    if (fp == NULL)
        return -1;

    while (!feof(fp)) {
        memset(file_buf, 0, sizeof(file_buf));
        if (fgets(file_buf, sizeof(file_buf), fp) != NULL) {
            char    *key, *val;

            if (strstr(file_buf, ";;\n") == NULL)
                continue;
            key = strtok(file_buf, "=");
            val = strtok(NULL, ";;\n");
            if (key == NULL)
                continue;
            if (val == NULL)
                continue;

            if(strcmp(key, "wireless.ap.ssid") == 0)
            {
                cnt += 1;
                printf("%s:%s=%s\n",__FUNCTION__,key,val);
                memcpy(SSID,val,strlen(val));
            }
            else if(strcmp(key, "wireless.ap.wpa.psk") == 0)
            {
                cnt += 1;
                printf("%s:%s=%s\n",__FUNCTION__,key,val);
                memcpy(PWD,val,strlen(val));
            }
        }
        if(cnt >= 2)
        break;
    }
    fclose(fp);
    //printf("close:%s\n",file);
    return 0;
}

int carimpl_CarLifeLaunch(void)
{
    ILyLink::InitData_t data = { 
        DrivingMode_LeftHand,
        1280, 720, "", LINK_TYPE_USBLIFE
    };

    ILyLink::getInstance()->SetCallback(new LylinkCB);
    ILyLink::getInstance()->SetParams(data);

    if (!ILyLink::getInstance()->Start()) {
        DEBUG("Start lylink fail");
        return -1;
    }
    return 0;
}

bool carimpl_CarLifeShow(bool bShow)
{
    return(ILyLink::getInstance()->ChangeVideoFocus(bShow));
}

bool carimpl_CarLifeSendTouchEvt(int x, int y, int mode)
{
    TouchMode_e md;
    int clx,cly;

    //DEBUG("x=%d y=%d md=%d\n",x,y,mode);
    /*
    typedef enum {
        E_ACTION_NONE,
        E_ACTION_DOWN,
        E_ACTION_UP,
        E_ACTION_MOVE,
        E_ACTION_CANCEL
    } EActionStatus;
    */
    md = (mode == 1) ? TouchMode_Down : \
        (mode == 2) ? TouchMode_Up : \
        (mode == 3)  ? TouchMode_Move : TouchMode_Move;

    //896x512->1280x720
    clx = x*g32width/g32Fbwidth;
    cly = y*g32height/g32Fbheight;
    clx = (clx > g32width)? g32width : clx;
    cly = (cly > g32height)? g32height : cly;

    return (ILyLink::getInstance()->SendTouchEvent(clx, cly, md));
}

bool carimpl_CarLifeSwitch(void)
{
    if(gbIsCnnt && (!gbIsShow)){
        return(carimpl_CarLifeShow(false));    //show carlife
    }
    else if(gbIsCnnt && gbIsShow){
        return(carimpl_CarLifeShow(true));     //hide carlife
    }
    return true;
}

//return    -1=zkgui handle key event
//          0 =lylinkapp handle key event
//          1 =show carlife
//          2 =hide carlife
int carimpl_CarLifeSendKey(int keycode, bool isDown)
{
    static int sLastcode = ConsumerActionACHome;
    
    #define KEYCODE_TO_CARPLAY(CODE) ( \
    CODE == KEY_ENTER ? ConsumerActionPlay : \
    CODE == KEY_RIGHT ? ConsumerActionScanPrev : \
    CODE == KEY_LEFT  ? ConsumerActionScanNext  : \
    CODE == KEY_UP    ? knobActionUp    : \
    CODE == KEY_DOWN  ? knobActionDown  : \
    CODE == KEY_MENU  ? ConsumerActionACHome  : \
    CODE == 127       ? ConsumerActionACHome  : \
    CODE == KEY_POWER ? ConsumerActionACBack : ConsumerActionACHome)

    int u32code = KEYCODE_TO_CARPLAY(keycode);

    if(gbIsCnnt && (!gbIsShow)){
        if(u32code != ConsumerActionACBack)
            return -1;   //handle by carcam.[not powerkey & not down]
        else{
            if(!isDown)
                return -1;
            if(carimpl_CarLifeShow(false))  //show carlife
                return 1;
        }
    }
    else if(gbIsCnnt && gbIsShow && (u32code == ConsumerActionACBack)){
            if(!isDown)
                return 0;
            if(carimpl_CarLifeShow(true))     //hide carlife
                return 2;
    }

    //Don`t care key release
    if(!isDown)
        return 0;
    
    if(sLastcode == ConsumerActionPlay && u32code == ConsumerActionPlay){
        u32code = ConsumerActionPause;
    }
    sLastcode = u32code;

    if(ILyLink::getInstance()->SendKey(u32code, isDown))
        return 0;

    return -1;
}

bool carimpl_CarLifeIsCnnt(void)
{
    return (gbIsShow || gbIsCnnt);
}

bool carimpl_CarLifeIsShow(void)
{
    return (gbIsShow);
}


#endif
