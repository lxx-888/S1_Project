#ifndef __INTERNAL_PCM_PLAYER_H__
#define __INTERNAL_PCM_PLAYER_H__


class IPcmPlayer
{
public:
	virtual ~IPcmPlayer() {}

	/* 0 ~ 100 */
	virtual void SetVolume(int vol) = 0;
	
	virtual void Start() = 0;
	
	virtual void Stop() = 0;
	
	virtual void Process(unsigned long long timestamp, unsigned char *data_ptr, int len) = 0;
public:
	static IPcmPlayer *Create(int rate, int bits, int chns);
};


#endif
