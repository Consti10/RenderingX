//
// Created by Constantin on 06.01.2018.
//

#include "FPSCalculator.h"


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
    }
}

float FPSCalculator::getCurrentFPS() {
    return (float)currFPS;
}

