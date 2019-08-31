//
// Created by Constantin on 25.11.2017.
//

#include "VRFrameCPUChronometer.h"
#include <android/log.h>
#include <sstream>

#define TAG "FrameCPUChronometer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

//Defining MY_RELEASE disables all measurements
//#define MY_RELEASE

using namespace std::chrono;

VRFrameCPUChronometer::VRFrameCPUChronometer(const std::vector<std::string>& timestampNames) {
#ifndef MY_RELEASE
    for (const auto &timestampName : timestampNames) {
        auto frameCPUTimestamp=std::make_unique<FrameCPUTimestamp>(timestampName);
        mFrameCPUTimestamps.push_back(std::move(frameCPUTimestamp));
    }
    lastLog=steady_clock::now();
#endif
}

void VRFrameCPUChronometer::start(bool whichEye) {
#ifndef MY_RELEASE
    auto& firstTS=mFrameCPUTimestamps.at(0);
    if(whichEye){
        leftEyeFull.start();
        firstTS->leftEye.start();
    }else{
        rightEyeFull.start();
        firstTS->rightEye.start();
    }
#endif
}

void VRFrameCPUChronometer::setTimestamp(bool whichEye,int whichTimestamp) {
#ifndef MY_RELEASE
    auto& prevTS=mFrameCPUTimestamps.at((unsigned long)whichTimestamp);
    auto& nextTS=mFrameCPUTimestamps.at((unsigned long)whichTimestamp+1);
    if(whichEye){
        prevTS->leftEye.stop();
        nextTS->leftEye.start();
    }else{
        prevTS->rightEye.stop();
        nextTS->rightEye.start();
    }
#endif
}

void VRFrameCPUChronometer::stop(bool whichEye) {
#ifndef MY_RELEASE
    auto& lastTS=mFrameCPUTimestamps.at(mFrameCPUTimestamps.size()-1);
    if(whichEye){
        lastTS->leftEye.stop();
        leftEyeFull.stop();
    }else{
        lastTS->rightEye.stop();
        rightEyeFull.stop();
    }
#endif
}

void VRFrameCPUChronometer::print() {
#ifndef MY_RELEASE
    const auto now=steady_clock::now();
    if(duration_cast<std::chrono::milliseconds>(now-lastLog).count()>5*1000){//every 5  seconds
        lastLog=now;
        std::ostringstream oss2;
        oss2<<"....................Avg. CPU frame times....................";
        for (auto& currentTS : mFrameCPUTimestamps) {
            const float leftEyeDelta=currentTS->leftEye.getAvgMS();
            const float rightEyeDelta=currentTS->rightEye.getAvgMS();
            const float frameDelta=(leftEyeDelta+rightEyeDelta)/2.0f;
            oss2<<"\n";
            oss2<<currentTS->name.c_str()<<": leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<frameDelta;
        }
        const float leftEyeDelta=leftEyeFull.getAvgMS();
        const float rightEyeDelta=rightEyeFull.getAvgMS();
        oss2<<"\n"<<"CPUTimeSum:"<<" leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<(leftEyeDelta+rightEyeDelta)/2.0f;
        oss2<<"\n----  -----  ----  ----  ----  ----  ----  ----  ";
        LOGD("%s",oss2.str().c_str());
    }
#endif
}

void VRFrameCPUChronometer::resetTS() {
#ifndef MY_RELEASE
    for (auto& currentTS : mFrameCPUTimestamps) {
        currentTS->reset();
    }
    leftEyeFull.reset();
    rightEyeFull.reset();
#endif
}


