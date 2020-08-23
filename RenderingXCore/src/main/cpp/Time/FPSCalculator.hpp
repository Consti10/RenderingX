//
// Created by Constantin on 06.01.2018.
//

#ifndef FPV_VR_FPSCALCULATOR_H
#define FPV_VR_FPSCALCULATOR_H

#include <cstdint>
#include <string>
#include <chrono>

class FPSCalculator {
private:
    const std::string NAME;
    const std::chrono::steady_clock::duration LOG_INTERVAL;
    std::chrono::steady_clock::time_point lastFPSCalculation=std::chrono::steady_clock::now();
    double ticksSinceLastFPSCalculation=0;
    double currFPS=0;
public:
    FPSCalculator(std::string name="FPSCalc",std::chrono::steady_clock::duration intervall=std::chrono::seconds(1)):
            NAME(std::move(name)),LOG_INTERVAL(intervall) {
    }
    void tick(){
        ticksSinceLastFPSCalculation++;
        const auto now=std::chrono::steady_clock::now();
        const auto delta=now-lastFPSCalculation;
        if(delta>LOG_INTERVAL){
            double exactElapsedSeconds=std::chrono::duration_cast<std::chrono::microseconds>(delta).count()*0.001*0.001;
            currFPS=ticksSinceLastFPSCalculation/exactElapsedSeconds;
            ticksSinceLastFPSCalculation=0;
            lastFPSCalculation=now;
        }
    }
    float getCurrentFPS()const {
        return (float)currFPS;
    }
};


#endif //FPV_VR_FPSCALCULATOR_H
