//
// Created by Constantin on 22.10.2017.
//

#include <sstream>
#include "Chronometer.h"
#include "jni.h"
#include <AndroidLogger.hpp>


using namespace std::chrono;

int64_t DurationAccumulator::getAvgUS() const {
    if(acc>0){
        return std::chrono::duration_cast<std::chrono::microseconds>(accDuration).count()/acc;
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


Chronometer::Chronometer(std::string name): mName(std::move(name)) {
    lastLog=startTS=steady_clock::now();
    reset();
}

Chronometer::Chronometer(): mName("Unknown") {
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

void Chronometer::printAvg(const steady_clock::duration& interval) {
    const auto now=steady_clock::now();
    if(now-lastLog>interval){
        lastLog=now;
        MLOGD2(mName)<<"Avg: "<<getAvgMS()<<"ms";
        reset();
    }
}

