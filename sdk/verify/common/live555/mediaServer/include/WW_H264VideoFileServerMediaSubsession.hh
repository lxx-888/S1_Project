/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2017 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 Elementary Stream video file.
// C++ header

#ifndef _WW_H264_VIDEO_FILE_SERVER_MEDIA_SUBSESSION_HH
#define _WW_H264_VIDEO_FILE_SERVER_MEDIA_SUBSESSION_HH

#include "OnDemandServerMediaSubsession.hh"

#include "WW_CallbackFunction.hh"

class WW_H264VideoFileServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
	static WW_H264VideoFileServerMediaSubsession*createNew(UsageEnvironment& env
									, char const* fileName, OPENSTREAM  OpenStreamFun
									, READSTREAM  ReadStreamFun, CLOSESTREAM CloseStreamFun
									, double iFrameRate
									, void* CallbackFunArg=NULL, int iCallbackFunArgSize=0
									, Boolean copyArgFlag=True, Boolean reuseFirstSource=False
                                    , Boolean usingDiscreteFramer=False, unsigned maxSize = 600000
                                    , unsigned bitRate = 4000 /*kbps*/);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1();
	void afterPlayingDummy1();
	static int GetVideoFrame(char* data, int iDataLen, char** pcOutData
									, int* piOutLen, char* NALUHead);
protected:
	WW_H264VideoFileServerMediaSubsession(UsageEnvironment& env, char const* fileName
									, Boolean reuseFirstSource, OPENSTREAM  OpenStreamFun
									, READSTREAM  ReadStreamFun, CLOSESTREAM CloseStreamFun
									, double iFrameRate
									, void* CallbackFunArg, int iCallbackFunArgSize
									, Boolean copyArgFlag, Boolean usingDiscreteFramer
                                    , unsigned maxSize, unsigned bitRate);
	// called only by createNew();
	virtual ~WW_H264VideoFileServerMediaSubsession();

	void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
	virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic
									, FramedSource* inputSource);
	virtual void startStream(unsigned clientSessionId, void* streamToken,
                             TaskFunc* rtcpRRHandler,
                             void* rtcpRRHandlerClientData,
                             unsigned short& rtpSeqNum,
                             unsigned& rtpTimestamp,
                             ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                             void* serverRequestAlternativeByteHandlerClientData);
private:
	char* fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	RTPSink* fDummyRTPSink; // ditto
    unsigned fMaxInputBufSize; // the size of input buffer created by Sink
    unsigned fStreamBitRate; // bitrate of live stream.
protected:
	char fFileName[1024];
	u_int64_t fFileSize; // if known
	OPENSTREAM  m_OpenStreamFun;
	READSTREAM  m_ReadStreamFun;
	CLOSESTREAM m_CloseStreamFun;
	void*       m_CallbackFunArg;
	int         m_iCallbackFunArgSize;
	Boolean     m_CopyArgFlag;
	double      m_fFrameRate;
    Boolean     m_usingDiscreteFramer;
};

#endif
