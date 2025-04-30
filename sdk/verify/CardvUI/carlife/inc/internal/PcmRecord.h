#ifndef __INTERNAL_PCM_RECORD_H__
#define __INTERNAL_PCM_RECORD_H__


class IPcmRecord
{
public:
	virtual ~IPcmRecord() {}

	virtual void Start() = 0;
		
	virtual void Stop() = 0;

	virtual int Read(const void **buffer) = 0;

public:
	static IPcmRecord *Create(int rate, int bits, int chns);
};


#endif
