#include "PcmPlayer.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mi_ao.h>


#define COLOR_NONE          "\033[0m"
#define COLOR_BLACK         "\033[0;30m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_WHITE         "\033[1;37m"

#define LOGD(fmt, ...)		printf (COLOR_GREEN "%s " fmt COLOR_NONE "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOGE(fmt, ...)		printf (COLOR_RED "%s " fmt COLOR_NONE "\n", __FUNCTION__, ##__VA_ARGS__)


#define AO_DEV_ID				0
#define AUDIO_SAMPLE_PER_FRAME	1024
#define MIN_AO_VOLUME           -60
#define MAX_AO_VOLUME           30
#define MIN_ADJUST_AO_VOLUME    -30
#define MAX_ADJUST_AO_VOLUME    20


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
		sampleRate = E_MI_AUDIO_SAMPLE_RATE_24000;
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

static MI_AUDIO_SoundMode_e getSoundChannelMode(int channels) {
	MI_AUDIO_SoundMode_e soundMode;

	switch(channels) {
	case 1:
		soundMode = E_MI_AUDIO_SOUND_MODE_MONO;
		break;
	case 2:
		soundMode = E_MI_AUDIO_SOUND_MODE_STEREO;
		break;
	default:
		soundMode = E_MI_AUDIO_SOUND_MODE_MAX;
		break;
	}

	return soundMode;
}


class CPcmPlayer : public IPcmPlayer
{
public:
	CPcmPlayer(int rate, int bits, int chns)
	{
		mRate = rate;
		mBits = bits;
		mChns = chns;
		bStart = false;
        LOGE(":[%d,%d,%d]\n", rate, bits, chns);
	}
	
	~CPcmPlayer()
	{
		Stop();
	}

	void SetVolume(int vol) override
	{
		MI_S32 s32VolumeDb;
		MI_AO_ChnState_t stAoState;
		
		if (vol > 0) {
			s32VolumeDb = vol * (MAX_ADJUST_AO_VOLUME - MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
		} else {
			s32VolumeDb = MIN_AO_VOLUME;
		}
		
		LOGD ("SetVolume %d", vol);
		memset(&stAoState, 0, sizeof(MI_AO_ChnState_t));
		if (MI_SUCCESS == MI_AO_QueryChnStat(AO_DEV_ID, 0, &stAoState)) {
			MI_AO_SetVolume(AO_DEV_ID, 0, s32VolumeDb, E_MI_AO_GAIN_FADING_OFF);
		}
		if (MI_SUCCESS == MI_AO_QueryChnStat(AO_DEV_ID, 1, &stAoState)) {
			MI_AO_SetVolume(AO_DEV_ID, 1, s32VolumeDb, E_MI_AO_GAIN_FADING_OFF);
		}
	}
	
	void Start() override
	{
		const MI_AUDIO_DEV AoDevId = AO_DEV_ID;
		MI_S32 iRet;
		//MI_AUDIO_Attr_t stSetAttr;
		MI_AUDIO_Attr_t stGetAttr;

		if (bStart) {
			return;
		}
		
		//set Ao Attr struct
		memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
		stSetAttr.eSamplerate = getSamplerate(mRate);
		stSetAttr.eBitwidth = getBitWidth(mBits);
		stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
		stSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;
		stSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
		stSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
		stSetAttr.u32FrmNum = 6;
		stSetAttr.u32PtNumPerFrm = AUDIO_SAMPLE_PER_FRAME;
		stSetAttr.u32ChnCnt = mChns;
		stSetAttr.eSoundmode = getSoundChannelMode(mChns);
		
		/* set ao public attr*/
		iRet = MI_AO_SetPubAttr(AoDevId, &stSetAttr);
		if (iRet != MI_SUCCESS) {
			LOGE("MI_AO_SetPubAttr %X", iRet);
		}
		
		/* get ao device*/
		iRet = MI_AO_GetPubAttr(AoDevId, &stGetAttr);
		if (iRet != MI_SUCCESS) {
			LOGE("MI_AO_SetPubAttr %X", iRet);
		}
		
		/* enable ao device */
		iRet = MI_AO_Enable(AoDevId);
		if (iRet != MI_SUCCESS) {
			return;
		}
		
		/* enable ao channel of device*/
		iRet = MI_AO_EnableChn(AoDevId, 0);
		if (iRet != MI_SUCCESS) {
			return;
		}
		
		if (mChns > 1) {
			iRet = MI_AO_EnableChn(AoDevId, 1);
			if (iRet != MI_SUCCESS) {
				return;
			}
		}

		bStart = true;
	}
	
	void Stop() override
	{
		const MI_AUDIO_DEV AoDevId = AO_DEV_ID;
		MI_S32 iRet;
		
		if (!bStart) {
			return;
		}
		
		/* disable ao channel of */
		MI_AO_DisableChn(AoDevId, 0);
		if (mChns > 1) {
			MI_AO_DisableChn(AoDevId, 1);
		}
		
		/* disable ao device */
		iRet = MI_AO_Disable(AoDevId);
		if (iRet != MI_SUCCESS) {
			return;
		}

		bStart = false;
	}
	
	void Process(unsigned long long timestamp, unsigned char *data_ptr, int len) override
	{
		const MI_AUDIO_DEV AoDevId = AO_DEV_ID;
		const MI_AO_CHN AoChn = 0;
		MI_S32 s32Ret;
		MI_AUDIO_Frame_t stAoSendFrame;
		
		memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
		stAoSendFrame.u32Len[0] = len;
		stAoSendFrame.apVirAddr[0] = data_ptr;
		stAoSendFrame.apVirAddr[1] = NULL;
		
		do {
			s32Ret = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, -1);
		} while (s32Ret == MI_AO_ERR_NOBUF);
	}

private:
	int mRate;
	int mBits;
	int mChns;
	bool bStart;
    MI_AUDIO_Attr_t stSetAttr;
};


IPcmPlayer *IPcmPlayer::Create(int rate, int bits, int chns)
{
	return new CPcmPlayer(rate, bits, chns);
}

