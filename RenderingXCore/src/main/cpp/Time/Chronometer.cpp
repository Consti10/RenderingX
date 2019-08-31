//
// Created by Constantin on 22.10.2017.
//

#include <sstream>
#include "Chronometer.h"
#include "jni.h"
#include "android/log.h"

#define LOGT(...) __android_log_print(ANDROID_LOG_DEBUG,"Chronometer:", __VA_ARGS__)

using namespace std::chrono;

Chronometer::Chronometer(const std::string& name): mName(name) {
    lastLog=startTS=steady_clock::now();
    reset();
}

Chronometer::Chronometer(): mName("") {
    lastLog=startTS=steady_clock::now();
    reset();
}

void Chronometer::start() {
    startTS=steady_clock::now();
}

void Chronometer::stop() {
    const auto now=steady_clock::now();
    const auto delta=(now-startTS);
    timeSumUS+=duration_cast<microseconds>(delta).count();
    timeCount++;
}

void Chronometer::reset() {
    timeSumUS=0;
    timeCount=0;
}

const int64_t Chronometer::getAvgUS()const {
    if(timeCount>0){
        return (timeSumUS/timeCount);
    }
    return 0;
}


const float Chronometer::getAvgMS()const{
    if(timeCount>0){
        const auto avgUS=(timeSumUS/timeCount);
        return avgUS/1000.0f;
    }
    return 0;
}


void Chronometer::printAvg(int intervalMS) {
    const auto now=steady_clock::now();
    if(duration_cast<milliseconds>(now-lastLog).count()>intervalMS){
        lastLog=now;
        const int64_t currAvgT= getAvgUS();
        LOGT("Avg: %s:%f",mName.c_str(),(float)(((double)currAvgT)*0.001f));
        reset();
    }
}


