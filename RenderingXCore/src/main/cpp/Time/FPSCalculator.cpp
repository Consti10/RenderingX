//
// Created by Constantin on 06.01.2018.
//

#include "FPSCalculator.h"

#define TAG "FPSCalculator"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

using namespace std::chrono;

FPSCalculator::FPSCalculator(const std::string& name, int printIntervalMS):
        NAME(name),LOG_INTERVAL_MS(printIntervalMS) {
}

void FPSCalculator::tick() {
    ticksSinceLastFPSCalculation++;
    const auto now=steady_clock::now();
    int64_t deltaMS=duration_cast<milliseconds>(now-lastFPSCalculation).count();
    if(deltaMS>LOG_INTERVAL_MS){
        double exactElapsedSeconds=deltaMS*0.001;
        currFPS=ticksSinceLastFPSCalculation/exactElapsedSeconds;
        ticksSinceLastFPSCalculation=0;
        lastFPSCalculation=now;
        //LOGD("%s:%f",PATH.c_str(),currFPS);
    }
}

float FPSCalculator::getCurrentFPS() {
    return (float)currFPS;
}

