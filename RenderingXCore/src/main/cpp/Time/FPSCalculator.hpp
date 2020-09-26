//
// Created by Constantin on 06.01.2018.
//

#ifndef FPV_VR_FPSCALCULATOR_H
#define FPV_VR_FPSCALCULATOR_H

#include <cstdint>
#include <string>
#include <chrono>
#include "AndroidLogger.hpp"
#include "TimeHelper.hpp"

class FPSCalculator {
private:
    const std::string NAME;
    const std::chrono::steady_clock::duration LOG_INTERVAL;
    std::chrono::steady_clock::time_point lastFPSCalculation=std::chrono::steady_clock::now();
    double ticksSinceLastFPSCalculation=0;
    double currFPS=0;
    std::chrono::steady_clock::time_point lastTick;
    AvgCalculator avgCalculator;
public:
    FPSCalculator(std::string name="FPSCalc",std::chrono::steady_clock::duration intervall=std::chrono::seconds(1)):
            NAME(std::move(name)),LOG_INTERVAL(intervall) {
    }
    void tick(){
        ticksSinceLastFPSCalculation++;
        const auto now=std::chrono::steady_clock::now();
        const auto delta=now-lastFPSCalculation;
        if(lastTick!=std::chrono::steady_clock::time_point()){
            const auto delta1=std::chrono::steady_clock::now()-lastTick;
            avgCalculator.add(delta1);
            lastTick=std::chrono::steady_clock::now();
        }else{
            lastTick=std::chrono::steady_clock::now();
        }
        if(delta>LOG_INTERVAL){
            double exactElapsedSeconds=std::chrono::duration_cast<std::chrono::microseconds>(delta).count()*0.001*0.001;
            currFPS=ticksSinceLastFPSCalculation/exactElapsedSeconds;
            ticksSinceLastFPSCalculation=0;
            lastFPSCalculation=now;
            MLOGD<<NAME<<" "<<currFPS<<" "<<avgCalculator.getAvgReadable();
            avgCalculator.reset();
        }
    }
    float getCurrentFPS()const {
        return (float)currFPS;
    }
};

class FrameTimeCalculator{
private:
    const std::string NAME;
    const std::chrono::steady_clock::duration LOG_INTERVAL;
    std::chrono::steady_clock::time_point lastLog=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastTick={};
    AvgCalculator avgCalculator;
public:
    FrameTimeCalculator(std::string name="FTCalc",std::chrono::steady_clock::duration intervall=std::chrono::seconds(1)):
    NAME(std::move(name)),LOG_INTERVAL(intervall) {
    }
    void tick(){
        const auto now=std::chrono::steady_clock::now();
        if(lastTick!=std::chrono::steady_clock::time_point()){
            const auto delta=std::chrono::steady_clock::now()-lastTick;
            avgCalculator.add(delta);
            lastTick=std::chrono::steady_clock::now();
        }else{
            lastTick=std::chrono::steady_clock::now();
        }
        const auto deltaLastLog=now-lastLog;
        if(deltaLastLog>LOG_INTERVAL){
            MLOGD<<NAME<<" "<<getCurrentFPS()<<" "<<avgCalculator.getAvgReadable();
            avgCalculator.reset();
            lastLog=std::chrono::steady_clock::now();
        }
    }
    float getCurrentFPS()const {
        return (float)1000.0f * 1000.0f/std::chrono::duration_cast<std::chrono::microseconds>(avgCalculator.getAvg()).count();
    }
};


#endif //FPV_VR_FPSCALCULATOR_H
