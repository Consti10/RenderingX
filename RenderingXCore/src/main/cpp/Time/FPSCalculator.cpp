//
// Created by Constantin on 06.01.2018.
//

#include "FPSCalculator.h"

#include <utility>


using namespace std::chrono;

FPSCalculator::FPSCalculator(std::string name,std::chrono::steady_clock::duration intervall):
        NAME(std::move(name)),LOG_INTERVAL(intervall) {
}

void FPSCalculator::tick() {
    ticksSinceLastFPSCalculation++;
    const auto now=steady_clock::now();
    const auto delta=now-lastFPSCalculation;
    if(delta>LOG_INTERVAL){
        double exactElapsedSeconds=duration_cast<microseconds>(delta).count()*0.001*0.001;
        currFPS=ticksSinceLastFPSCalculation/exactElapsedSeconds;
        ticksSinceLastFPSCalculation=0;
        lastFPSCalculation=now;
    }
}

float FPSCalculator::getCurrentFPS()const {
    return (float)currFPS;
}

