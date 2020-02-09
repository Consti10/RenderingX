//
// Created by Constantin on 25.11.2017.
//

#include "VRFrameCPUChronometer.h"
#include <android/log.h>
#include <sstream>
#include <numeric>

#define TAG "FrameCPUChronometer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

using namespace std::chrono;


void VREyeDurations::print() {
    std::stringstream ss;
    ss<<"VR Frame Time Stamp"<<"\n";
    for(unsigned int i=0;i<stamps.size();i++){
        const auto [ name,time ] = stamps.at(i);
        ss<<name<<": "<<std::chrono::duration_cast<std::chrono::nanoseconds>(time).count()<<"\n";
    }
    ss<<"Total:"<<std::chrono::duration_cast<std::chrono::nanoseconds>(totalTime).count();
    LOGD("%s",ss.str().c_str());
}

VRFrameTimeAccumulator::VRFrameTimeAccumulator(
        const std::vector<std::string> &timestampNames) {
    for (const auto &timestampName : timestampNames) {
        auto frameCPUTimestamp=std::make_unique<VRFrameDurations>(timestampName);
        mFrameCPUTimestamps.push_back(std::move(frameCPUTimestamp));
    }
    lastLog=steady_clock::now();
}

void VRFrameTimeAccumulator::add(const VREyeDurations &timeStamps) {
    const int size=timeStamps.stamps.size();
    if(size!=mFrameCPUTimestamps.size()){
        LOGD("ERROR not matching size");
        return;
    }
    for(unsigned int i=0;i<size;i++){
        const auto [name, delta]=timeStamps.stamps.at(i);
        if(name!=mFrameCPUTimestamps.at(i).get()->name){
            LOGD("ERROR not matching name");
            return;
        }
        auto& frameCPUTimeStamp=timeStamps.whichEye ?mFrameCPUTimestamps.at(i).get()->leftEye :
                mFrameCPUTimestamps.at(i).get()->rightEye;
        frameCPUTimeStamp.add(delta);
    }
    if(timeStamps.whichEye){
        leftEyeTotal.add(timeStamps.totalTime);
    }else{
        rightEyeTotal.add(timeStamps.totalTime);
    }

}



void VRFrameTimeAccumulator::print() {
    std::stringstream ss;
    ss<<"....................VRFrameTimeAccumulator....................";
    for (auto& currentTS : mFrameCPUTimestamps) {
        const float leftEyeDelta= currentTS->leftEye.getAvgMS();
        const float rightEyeDelta= currentTS->rightEye.getAvgMS();
        const float frameDelta=(leftEyeDelta+rightEyeDelta)/2.0f;
        ss<<"\n";
        ss<<currentTS->name.c_str()<<": leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<frameDelta;
    }
    const float leftEyeDelta= leftEyeTotal.getAvgMS();
    const float rightEyeDelta= rightEyeTotal.getAvgMS();
    ss<<"\n"<<"CPUTimeSum:"<<" leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<(leftEyeDelta+rightEyeDelta)/2.0f;
    ss<<"\n----  -----  ----  ----  ----  ----  ----  ----  ";
    LOGD("%s",ss.str().c_str());
}

void VRFrameTimeAccumulator::reset() {
    for (auto& currentTS : mFrameCPUTimestamps) {
        currentTS->reset();
    }
    leftEyeTotal.reset();
    rightEyeTotal.reset();
}

void VRFrameTimeAccumulator::printEveryXSeconds(const int intervalS) {
    const auto now=steady_clock::now();
    if(duration_cast<std::chrono::seconds>(now-lastLog).count()>intervalS) {//every X  seconds
        lastLog=now;
        print();
    }
}
