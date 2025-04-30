#include "H264Player.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include <mi_vdec.h>
#include <mi_common.h>
#include <mi_sys.h>
#include <mi_disp.h>
#include <mi_panel.h>
#include "dictionary.h"
#include "iniparser.h"


#define MAX_ONE_FRM_SIZE (2 * 1024 * 1024)
#define DROP_SLICE_ID (3)
#define VDEC_DEV_ID     0
#define VDEC_CHN_ID     0   //REF::CH8->H264     CH9->H265
#define USE_MIPI        1
#define SCL_DEV_ID      3
#define SCL_CHN_ID      6


#define DISABLE_LOW_LATENCY	0
#define FRAME_MODE_SUPPORT	0
#define ROTATE_ENABLE		1


#define COLOR_NONE          "\033[0m"
#define COLOR_BLACK         "\033[0;30m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_WHITE         "\033[1;37m"

#define ST_NOP(fmt, args...)
#define ST_DBG(fmt, args...) ({do{printf(COLOR_GREEN "[DBG]:%s[%d]: " COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_WARN(fmt, args...) ({do{printf(COLOR_YELLOW "[WARN]:%s[%d]: " COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_INFO(fmt, args...) ({do{printf("[INFO]:%s[%d]: \n", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_ERR(fmt, args...) ({do{printf(COLOR_RED "[ERR]:%s[%d]: " COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return;\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)

#define MIN(a, b) 			((a) > (b) ? (b) : (a))


class CVideoPlayer : public IH264Player
{
public:
	CVideoPlayer();
	
	~CVideoPlayer();
public:
	void InitScreen(
		MI_U16 videoWidth, 
		MI_U16 videoHeight, 
		MI_U16 outputWidth, 
		MI_U16 outputHeight) override;

	void ConfigureCodec(void *data, int len) override;
	
	void Process(
			unsigned long long timestamp,
			unsigned char *dataPtr,
			int len) override;
			
	void Start() override;
	
	void Stop() override;

private:
	MI_U32 u32InWidth;
	MI_U32 u32InHeight;
    MI_U32 u32DevId;
    MI_U32 u32ChlId;
	MI_U16 u16OutWidth;
	MI_U16 u16OutHeight;
	MI_U32 u32RefFrmNum;
	pthread_mutex_t m_lock;
	bool mStart;
	MI_VDEC_VideoStream_t m_stVdecStream;
};

CVideoPlayer::CVideoPlayer()
{   
    //MI_U32 BindPBSrcMod;
    dictionary *pstDict = iniparser_load("/bootconfig/bin/default.ini");
    if (pstDict == NULL)
    {
        ST_DBG("ini NOT exist\n");
        u32DevId = SCL_DEV_ID;
        u32ChlId = SCL_CHN_ID;
    }
    else{
        //BindPBSrcMod = iniparser_getint(pstDict, "Disp:BindPBSrcMod", 34);
        u32DevId = iniparser_getint(pstDict, "Disp:BindPBSrcDev", SCL_DEV_ID);
        u32ChlId = iniparser_getint(pstDict, "Disp:BindPBSrcChn", SCL_CHN_ID);
    }
    
    u32InWidth = 748;
    u32InHeight = 480;
    u16OutWidth = 748;
    u16OutHeight = 480;
    u32RefFrmNum = 16;
    pthread_mutex_init(&m_lock, NULL);
    mStart = false;
}

CVideoPlayer::~CVideoPlayer()
{
	if (mStart) {
		Stop();
	}
	pthread_mutex_destroy(&m_lock);
}

void CVideoPlayer::InitScreen(MI_U16 width_in, MI_U16 height_in, MI_U16 width_out, MI_U16 height_out)
{
	u32InWidth = width_in;
	u32InHeight = height_in;
	u16OutWidth = width_out;
	u16OutHeight = height_out;

	ST_DBG ("init video %dx%d -> %dx%d\n", width_in, height_in, width_out, height_out);
}

void CVideoPlayer::ConfigureCodec(void *data, int len)
{
	int result;

	pthread_mutex_lock(&m_lock);
	if (!mStart) {
		goto notStarted;
	}
	
	m_stVdecStream.pu8Addr = (MI_U8 *)data;
	m_stVdecStream.u32Len = len;
	m_stVdecStream.bEndOfFrame = 1;
	m_stVdecStream.bEndOfStream = 0;

	{
		const MI_VDEC_DEV VdecDev = VDEC_DEV_ID;
		const MI_VDEC_CHN VdecChn = VDEC_CHN_ID;
		int s32TimeOutMs = 20;
		if (MI_SUCCESS != (result = MI_VDEC_SendStream(VdecDev, VdecChn, &m_stVdecStream, s32TimeOutMs)))
		{
			ST_ERR("%s chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", __FUNCTION__, VdecChn, m_stVdecStream.u32Len, result);
			usleep(30 * 1000);
		}
	}
	
	m_stVdecStream.u64PTS += 33;

notStarted:
	pthread_mutex_unlock(&m_lock);
}

void CVideoPlayer::Process(
		unsigned long long timestamp,
		unsigned char *dataPtr,
		int len)
{
	int result;
	
	pthread_mutex_lock(&m_lock);
	if (!mStart) {
		goto notStarted;
	}

	m_stVdecStream.pu8Addr = dataPtr;
	m_stVdecStream.u32Len = len;
	m_stVdecStream.bEndOfFrame = 1;
	m_stVdecStream.bEndOfStream = 0;
	
	{
		const MI_VDEC_DEV VdecDev = VDEC_DEV_ID;
		const MI_VDEC_CHN VdecChn = VDEC_CHN_ID;
		int s32TimeOutMs = 20;
		if (MI_SUCCESS != (result = MI_VDEC_SendStream(VdecDev, VdecChn, &m_stVdecStream, s32TimeOutMs)))
		{
			ST_ERR("%s chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", __FUNCTION__, VdecChn, m_stVdecStream.u32Len, result);
			usleep(30 * 1000);
		}
	}
	
	m_stVdecStream.u64PTS += 33;

notStarted:
	pthread_mutex_unlock(&m_lock);
}


void CVideoPlayer::Start()
{
	//init vdec
	const MI_VDEC_DEV VdecDev = VDEC_DEV_ID;
	const MI_VDEC_CHN VdecChn = VDEC_CHN_ID;
	const MI_U16 u16SocId = 0;
	MI_VDEC_ChnAttr_t stVdecChnAttr;
	MI_VDEC_OutputPortAttr_t stOutputPortAttr;
	MI_SYS_ChnPort_t  stChnPort; 
	MI_U16 vdec_width, vdec_height;

	pthread_mutex_lock(&m_lock);
	if (mStart) {
		goto Started;
	}

	vdec_width = MIN(u16OutWidth, u32InWidth);
	vdec_height = MIN(u16OutHeight, u32InHeight);

	memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
	stVdecChnAttr.eCodecType   = E_MI_VDEC_CODEC_TYPE_H264;
	stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = u32RefFrmNum;
	stVdecChnAttr.eVideoMode   = E_MI_VDEC_VIDEO_MODE_FRAME;
	stVdecChnAttr.u32BufSize   = 1920 * 1080;
	stVdecChnAttr.u32PicWidth  = 1920;
	stVdecChnAttr.u32PicHeight = 1080;
	stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_NORMAL;
	stVdecChnAttr.u32Priority  = 0;	
	STCHECKRESULT(MI_VDEC_CreateChn(VdecDev, VdecChn, &stVdecChnAttr));
	STCHECKRESULT(MI_VDEC_StartChn(VdecDev, VdecChn));

	memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
	stOutputPortAttr.u16Width  = vdec_width;
	stOutputPortAttr.u16Height = vdec_height;
	STCHECKRESULT(MI_VDEC_SetOutputPortAttr(VdecDev, VdecChn, &stOutputPortAttr));

	memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stChnPort.eModId	= E_MI_MODULE_ID_VDEC;
	stChnPort.u32DevId	= VdecDev;
	stChnPort.u32ChnId	= VdecChn;
	stChnPort.u32PortId = 0;
	STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(u16SocId, &stChnPort, 0, 5));

    MI_SYS_ChnPort_t stSrcChnPort, stDstChnPort;
	//bind vdec to disp
	stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
	stSrcChnPort.u32DevId = 0;
	stSrcChnPort.u32ChnId = VdecChn;
	stSrcChnPort.u32PortId = 0;

    //ref:
    //pstSysChnPort.eModId = carimpl.stDispInfo.stPlayBackChnPort.eModId;
    //pstSysChnPort.u32DevId = carimpl.stDispInfo.stPlayBackChnPort.u32DevId;
    //pstSysChnPort.u32ChnId = carimpl.stDispInfo.stPlayBackChnPort.u32ChnId;
    //pstSysChnPort.u32PortId = carimpl.stDispInfo.stPlayBackChnPort.u32PortId;
	stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
	stDstChnPort.u32DevId = u32DevId;
	stDstChnPort.u32ChnId = u32ChlId;
	stDstChnPort.u32PortId = 0;
	STCHECKRESULT(MI_SYS_BindChnPort(u16SocId, &stSrcChnPort, &stDstChnPort, 30, 30));

	m_stVdecStream.u64PTS = 0;	
	mStart = true;

Started:
	pthread_mutex_unlock(&m_lock);
}

void CVideoPlayer::Stop()
{
	const MI_VDEC_DEV VdecDev = VDEC_DEV_ID;
	const MI_VDEC_CHN VdecChn = VDEC_CHN_ID;
	const MI_U16 u16SocId = 0;
    MI_SYS_ChnPort_t stSrcChnPort, stDstChnPort;
	
	pthread_mutex_lock(&m_lock);
	if (!mStart) {
		goto notStarted;
	}
	
    //Unbind vdec 2 disp
    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = VdecChn;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId = SCL_DEV_ID;
    stDstChnPort.u32ChnId = SCL_CHN_ID;
    stDstChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_UnBindChnPort(u16SocId, &stSrcChnPort, &stDstChnPort));
    STCHECKRESULT(MI_VDEC_StopChn(VdecDev, VdecChn));
    STCHECKRESULT(MI_VDEC_DestroyChn(VdecDev, VdecChn));
    STCHECKRESULT(MI_VDEC_DestroyDev(VdecDev));

    mStart = false;

notStarted:
	pthread_mutex_unlock(&m_lock);
}

IH264Player *IH264Player::Create()
{
	return new CVideoPlayer;
}

