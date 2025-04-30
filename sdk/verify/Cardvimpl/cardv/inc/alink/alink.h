///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __ALINKDEMO_H__
#define __ALINKDEMO_H__
//#include <google/protobuf/stubs/common.h>
#include <stdint.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ALinkServerNative.hpp>

class EventListener;
class VideoListener;
class SensorsListener;
typedef void (*AlinkStreamCB)(bool);

class AlinkDemo {
 public:
    //explicit AlinkDemo(alink::ALinkServerNative & server) : mServer(server) {
        // empty by design
    //}
    AlinkDemo():IsConnected(false),g_AlinkStream_Callback(NULL){
        // empty by design
    }

    void start();

    void stop();
    
    alink::ALinkServerNative * mServer;
    std::list<int32_t> mConnections;
    bool IsConnected;
    AlinkStreamCB g_AlinkStream_Callback;
    
 private:
    
    std::shared_ptr<EventListener> mEventListener;
    std::shared_ptr<VideoListener> mVideoListener;
    std::mutex mMutex;
    bool mExit;
    bool mSensorsStopped;
    //std::shared_ptr<std::thread> mThreadPtr;
    //std::shared_ptr<std::thread> mSensorsThreadPtr;

    friend class EventListener;
    friend class VideoListener;
    friend class SensorsListener;
};

class EventListener : public alink::IEventListener {
 public:
    explicit EventListener(AlinkDemo & o) : that(o) {
        // empty by design
    }

    void onEvent(int32_t cid, alink::Channel channel, const char * json, size_t size) override {
        std::cout << json << std::endl;
    }

 private:
    AlinkDemo & that;
};

class SensorsListener : public alink::ISensorOutListener {
 public:
    explicit SensorsListener(AlinkDemo & o) : that(o) {
        // empty by design
    }

    void onConfig(int32_t cid, alink::Channel channel,
                  const char * json, size_t size, alink::callback_t respond) override {
        std::cout << json << std::endl;
        if (respond) {
            std::string response = "{\"command\":\"schema_response\",\"schema\":[0,1]}";
            respond(cid, channel, response.c_str(), response.size());
        }
    }

    void onStart(int32_t cid, alink::Channel channel) override {
        if (alink::kSensorOut == channel) {
            that.mSensorsStopped = false;
            std::cout << "Start sensors " << std::endl;
        }
    }

    void onStop(int32_t cid, alink::Channel channel) override {
        if (alink::kSensorOut == channel) {
            that.mSensorsStopped = true;
            std::cout << "Stop sensors" << std::endl;
        }
    }

 private:
    AlinkDemo & that;
};

class VideoListener : public alink::IVideoOutListener {
 public:
    explicit VideoListener(AlinkDemo & o) : that(o) {
        // empty by design
    }

    void onConfig(int32_t cid, alink::Channel channel, const char * json, size_t size, alink::callback_t res) override {
        std::cout << json << std::endl;
        if (res) {
            std::string response = "{\"video_format\":\"H.264\",\"width\":800,\"height\":600}";
            res(cid, channel, response.c_str(), response.size());
        }
    }

    void onStart(int32_t cid, alink::Channel channel) override {
        if (alink::kVideoOut == channel) {
            std::lock_guard<std::mutex> guard(that.mMutex);
            that.mConnections.push_back(cid);
            std::cout << "Video start for connection " << cid << std::endl;
            that.IsConnected = true;
            if(that.g_AlinkStream_Callback){
                that.g_AlinkStream_Callback(1);
                }
    
        }
    }

    void onStop(int32_t cid, alink::Channel channel) override {
        if (alink::kVideoOut == channel) {
            std::lock_guard<std::mutex> guard(that.mMutex);
            for (auto it = that.mConnections.begin(); it != that.mConnections.end(); ++it) {
                if (cid == *it) {
                    that.mConnections.erase(it);
                    std::cout << "Video stop for connection " << cid << std::endl;
                    that.IsConnected = false;
                    if(that.g_AlinkStream_Callback){
                        that.g_AlinkStream_Callback(0);
                        }
                    break;
                }
            }
        }
    }

    void onPause(int32_t cid, alink::Channel channel) override {
        // pass by
    }

    void onResume(int32_t cid, alink::Channel channel) override {
        // pass by
    }

 private:
    AlinkDemo & that;
};


#endif
