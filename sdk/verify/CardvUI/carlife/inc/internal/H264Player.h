#ifndef __INTERNAL_H264_PLAYER_H__
#define __INTERNAL_H264_PLAYER_H__


class IH264Player
{
public:
	virtual ~IH264Player() {}

	virtual void InitScreen(
		unsigned short videoWidth, 
		unsigned short videoHeight, 
		unsigned short outputWidth, 
		unsigned short outputHeight) = 0;

	virtual void ConfigureCodec(void *data, int len) = 0;
	
	virtual void Process(
		unsigned long long timestamp,
		unsigned char *dataPtr,
		int len) = 0;
			
	virtual void Start() = 0;
	
	virtual void Stop() = 0;
public:
	static IH264Player *Create();
};


#endif
