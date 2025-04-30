#include "PcmRecord.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <mi_ai.h>
#include <mi_sys.h>



#define LOGD		printf
#define LOGE		printf

#define ARRSIZE(t)		(sizeof(t)/sizeof(t[0]))

#define USE_AMIC	1

#define AUDIO_PT_NUMBER_FRAME (1024)
#define AUDIO_SAMPLE_RATE (E_MI_AUDIO_SAMPLE_RATE_24000)
#define AUDIO_SOUND_MODE (E_MI_AUDIO_SOUND_MODE_MONO)

#define AUDIO_AO_DEV_ID_LINE_OUT 0
#define AUDIO_AO_DEV_ID_I2S_OUT  1

#define AUDIO_AI_DEV_ID_AMIC_IN   0
#define AUDIO_AI_DEV_ID_DMIC_IN   1
#define AUDIO_AI_DEV_ID_I2S_IN    2

#define RECORD_24K_TO_48K		1

#if USE_AMIC
#define AI_DEV_ID (AUDIO_AI_DEV_ID_AMIC_IN)
#else
#define AI_DEV_ID (AUDIO_AI_DEV_ID_DMIC_IN)
#endif
#define AO_DEV_ID (AUDIO_AO_DEV_ID_LINE_OUT)

#ifndef ExecFunc
#define ExecFunc(_func_, _ret_) \
    do{ \
        MI_S32 s32Ret = _func_; \
        if (s32Ret != _ret_) \
        { \
            LOGD("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            goto _exit; \
        } \
        else \
        { \
            LOGD("[%s %d]exec function pass\n", __func__, __LINE__); \
        } \
    } while(0)
#endif

#ifndef ExecFuncNoRet
#define ExecFuncNoRet(_func_, _ret_) \
    do{ \
        MI_S32 s32Ret = _func_; \
        if (s32Ret != _ret_) \
        { \
            LOGD("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
        } \
        else \
        { \
            LOGD("[%s %d]exec function pass\n", __func__, __LINE__); \
        } \
    } while(0)
#endif


#define COLOR_NONE          "\033[0m"
#define COLOR_BLACK         "\033[0;30m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_WHITE         "\033[1;37m"

#define ST_DBG(fmt, ...)  LOGD(COLOR_GREEN "CPcmRecord [DBG]:%s[%d]: " fmt COLOR_NONE, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define ST_ERR(fmt, ...)  LOGE(COLOR_RED "CPcmRecord [ERR]:%s[%d]: " fmt COLOR_NONE, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define AD_LOG(fmt, ...)  LOGD("\033[1;34mCPcmRecord:%s[%d]:" fmt COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)


static MI_AUDIO_SampleRate_e getSamplerate(int rate) {
	MI_AUDIO_SampleRate_e sampleRate;

	switch(rate) {
	case 8000:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_8000;
		break;
	case 11025:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_11025;
		break;
	case 12000:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_12000;
		break;
	case 16000:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
		break;
	case 22050:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_22050;
		break;
	case 24000:
#if RECORD_24K_TO_48K
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_48000;
#else
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_24000;
#endif
		break;
	case 32000:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_32000;
		break;
	case 44100:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_44100;
		break;
	case 48000:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_48000;
		break;
	case 96000:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_96000;
		break;
	default:
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
		break;
	}

	return sampleRate;
}

static MI_AUDIO_BitWidth_e getBitWidth(int bits) {
	MI_AUDIO_BitWidth_e bitWidth;

	switch(bits) {
	case 16:
		bitWidth = E_MI_AUDIO_BIT_WIDTH_16;
		break;
	case 24:
		bitWidth = E_MI_AUDIO_BIT_WIDTH_24;
		break;
	case 32:
		bitWidth = E_MI_AUDIO_BIT_WIDTH_32;
		break;
	default:
		bitWidth = E_MI_AUDIO_BIT_WIDTH_MAX;
		break;
	}

	return bitWidth;
}


class CPcmRecord : public IPcmRecord
{
public:
	CPcmRecord(int rate, int bits, int chns);
	~CPcmRecord();

public:
	void Start() override;
	
	void Stop() override;

	int Read(const void **buffer) override;

private:
	enum {
		State_Stop,
		State_24k,
		State_Start,
	} rec_state;
	int rate;
	int bits;
	int chns;
	MI_AUDIO_Frame_t stAudioFrame;
	MI_AUDIO_AecFrame_t stAecFrame;
};

CPcmRecord::CPcmRecord(int rate, int bits, int chns)
{
	rec_state = State_Stop;
	this->rate = rate;
	this->bits = bits;
	this->chns = chns;
	stAudioFrame.u32Len[0] = 0;
    LOGE("%s [%d,%d,%d]\n", __FUNCTION__, rate, bits, chns);
}

CPcmRecord::~CPcmRecord()
{
	Stop();
}

void CPcmRecord::Start()
{
	const MI_U16 u16SocId = 0;
	int state = 0;

	//Ai
	const MI_AUDIO_DEV AiDevId = AI_DEV_ID;
	const MI_AI_CHN AiChn = 0;
	MI_AUDIO_Attr_t stAiSetAttr;
	MI_SYS_ChnPort_t stAiChn0OutputPort0;
	MI_AI_VqeConfig_t stAiVqeConfig;

	//Ao
	const MI_AUDIO_DEV AoDevId = AO_DEV_ID;
	const MI_AO_CHN AoChn = 0;

	LOGD("CPcmRecord %p %s rate = %d, bits = %d, chns = %d\n", this, __FUNCTION__, rate, bits, chns);

	if (rec_state != State_Stop) {
		return;
	}

	stAiSetAttr.eSamplerate = getSamplerate(rate);
	stAiSetAttr.eBitwidth = getBitWidth(bits);
	stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
	stAiSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiSetAttr.u32FrmNum = 6;  // useless
    stAiSetAttr.u32PtNumPerFrm = stAiSetAttr.eSamplerate / 16; // for aec
    stAiSetAttr.u32CodecChnCnt = 0; // useless
	stAiSetAttr.u32ChnCnt = 1;
    stAiSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE; // useless
    stAiSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAiSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;

	// 设置 AI 设备属性
	ExecFunc(MI_AI_SetPubAttr(AiDevId, &stAiSetAttr), MI_SUCCESS);
	// 启用 AI 设备
	ExecFunc(MI_AI_Enable(AiDevId), MI_SUCCESS);

	state = 1;

	// set ai vqe (MI_AI_VqeConfig_t stAiVqeConfig)
	stAiVqeConfig.bHpfOpen = FALSE;
	if (rate == E_MI_AUDIO_SAMPLE_RATE_8000 || rate == E_MI_AUDIO_SAMPLE_RATE_16000) {
		stAiVqeConfig.bAecOpen = TRUE;
	} else {
		stAiVqeConfig.bAecOpen = FALSE;
	}	
	stAiVqeConfig.bAnrOpen = FALSE;
	stAiVqeConfig.bAgcOpen = FALSE;
	stAiVqeConfig.bEqOpen = FALSE;
	stAiVqeConfig.u32ChnNum = 1;
	stAiVqeConfig.s32WorkSampleRate = stAiSetAttr.eSamplerate;
	stAiVqeConfig.s32FrameSample = 128;

	if (stAiVqeConfig.bHpfOpen) {
		MI_AUDIO_HpfConfig_t stHpfCfg = {
			.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
			.eHpfFreq = E_MI_AUDIO_HPF_FREQ_150,
		};
		stAiVqeConfig.stHpfCfg = stHpfCfg;
	}

	if (stAiVqeConfig.bAecOpen) {
		MI_AI_AecConfig_t stAecCfg = {
		    .bComfortNoiseEnable = FALSE,
		    .s16DelaySample = 0,
		    .u32AecSupfreq = {4, 6, 36, 49, 50, 51},
		    .u32AecSupIntensity = {5, 4, 4, 5, 10, 10, 10},
		};
		stAiVqeConfig.stAecCfg = stAecCfg;
	}

	if (stAiVqeConfig.bAnrOpen) {
		MI_AUDIO_AnrConfig_t stAnrCfg = {
			.eMode = E_MI_AUDIO_ALGORITHM_MODE_MUSIC,
			.u32NrIntensityBand = {20, 40 ,60, 80, 100, 120},
			.u32NrIntensity = {15, 15, 15, 15, 15, 15, 15},
			.u32NrSmoothLevel = 10,
			.eNrSpeed = E_MI_AUDIO_NR_SPEED_MID,
		};
		stAiVqeConfig.stAnrCfg = stAnrCfg;
	}

	if (stAiVqeConfig.bAgcOpen) {
		MI_AUDIO_AgcConfig_t stAgcCfg = {
			.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
			.stAgcGainInfo = {
				.s32GainMax = 40,
				.s32GainMin = -20,
				.s32GainInit = 0,
			},
			.u32DropGainMax = 60,
			.u32AttackTime = 1,
			.u32ReleaseTime = 1,
			.s16Compression_ratio_input = {-80, -60, -40, -20, -10, 0, 0},
			.s16Compression_ratio_output = {-80, -60, -40, -20, -10, 0, 0},
			.s32DropGainThreshold= -3,
			.s32NoiseGateDb = -55,
			.u32NoiseGateAttenuationDb = 0,
		};
		stAiVqeConfig.stAgcCfg = stAgcCfg;
	}

	if (stAiVqeConfig.bEqOpen) {
		MI_AUDIO_EqConfig_t stEqCfg = {
		    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
		};
		for (MI_U32 i = 0; i < ARRSIZE(stEqCfg.s16EqGainDb); i++) {
			stEqCfg.s16EqGainDb[i] = 3;
		}
		stAiVqeConfig.stEqCfg = stEqCfg;
	}

	// set ai output port depth ()
	stAiChn0OutputPort0.eModId = E_MI_MODULE_ID_AI;
	stAiChn0OutputPort0.u32DevId = AiDevId;
	stAiChn0OutputPort0.u32PortId = 0;
	stAiChn0OutputPort0.u32ChnId = AiChn;

#if USE_AMIC
	ExecFunc(MI_AI_SetVqeVolume(AiDevId, 0, 15), MI_SUCCESS);
#else
	ExecFunc(MI_AI_SetVqeVolume(AiDevId, 0, 4), MI_SUCCESS);
#endif

	ExecFunc(MI_SYS_SetChnOutputPortDepth(u16SocId, &stAiChn0OutputPort0, 1, 8), MI_SUCCESS);
	ExecFunc(MI_AI_EnableChn(AiDevId, AiChn), MI_SUCCESS);

	state = 2;
	
	ExecFunc(MI_AI_SetVqeAttr(AiDevId, AiChn, AoDevId, AoChn, &stAiVqeConfig), MI_SUCCESS);
    ExecFuncNoRet(MI_AI_EnableVqe(AiDevId, AiChn), MI_SUCCESS);

	rec_state = rate == E_MI_AUDIO_SAMPLE_RATE_24000 ? State_24k : State_Start;
	state = 0;

_exit:	
	if (state > 0) {
		if (state > 1) {
			MI_AI_DisableChn(AiDevId, AiChn);
		}
		MI_AI_Disable(AiDevId);
	}
}

void CPcmRecord::Stop()
{
	const MI_AUDIO_DEV AiDevId = AI_DEV_ID;
	const MI_AI_CHN AiChn = 0;	

	if (rec_state == State_Stop) {
		return;
	}

	if (stAudioFrame.u32Len > 0) {
		MI_AI_ReleaseFrame(AiDevId, AiChn, &stAudioFrame, &stAecFrame);
	}
	
	MI_AI_DisableVqe(AiDevId, AiChn);
	MI_AI_DisableChn(AiDevId, AiChn);
	MI_AI_Disable(AiDevId);
	rec_state = State_Stop;
}

int CPcmRecord::Read(const void **buffer)
{
	const MI_AUDIO_DEV AiDevId = AI_DEV_ID;
	const MI_AI_CHN AiChn = 0;

	if (stAudioFrame.u32Len[0] > 0) {
		MI_AI_ReleaseFrame(AiDevId, AiChn, &stAudioFrame, &stAecFrame);
	}
	
	//MI_AI_GetFrame(AiDevId, AiChn, &stAudioFrame, &stAecFrame, 32); //1024 / 8000 = 128ms
	stAudioFrame.u32Len[0] = 0;
	if (0 == stAudioFrame.u32Len[0]) {
		return 0;
	}
	
	if (rec_state == State_24k) {
		MI_U32 i;
		MI_U16 *pData = (MI_U16 *)stAudioFrame.apVirAddr[0];
		stAudioFrame.u32Len[0] /= 2;
		for (i = 1; i < stAudioFrame.u32Len[0]; i++) {
			pData[i] = pData[i * 2];
		}
	}

	*buffer = stAudioFrame.apVirAddr[0];

	return stAudioFrame.u32Len[0];
}

IPcmRecord *IPcmRecord::Create(int rate, int bits, int chns)
{
	return new CPcmRecord(rate, bits, chns);
}

