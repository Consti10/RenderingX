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
    average.add(delta);
}

void Chronometer::reset() {
    average.reset();
}

int64_t Chronometer::getAvgUS()const {
    return average.getAvgUS();
}

float Chronometer::getAvgMS()const{
    return average.getAvgMS();
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


int64_t DurationAccumulator::getAvgUS() const {
    if(acc>0){
        return std::chrono::duration_cast<std::chrono::nanoseconds>(accDuration).count()/acc;
    }
    return 0;
}

float DurationAccumulator::getAvgMS() const {
    if(acc>0){
        const auto avgUS=std::chrono::duration_cast<std::chrono::microseconds>(accDuration).count()/acc;
        return avgUS/1000.0f;
    }
    return 0;
}
