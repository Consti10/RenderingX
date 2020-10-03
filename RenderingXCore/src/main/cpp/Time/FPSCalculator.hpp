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

// Deprecated. Use FrameTimeCalculator for same and more functionalities
class FPSCalculator {
private:
    const std::string NAME;
    const std::chrono::steady_clock::duration LOG_INTERVAL;
    std::chrono::steady_clock::time_point lastFPSCalculation=std::chrono::steady_clock::now();
    double ticksSinceLastFPSCalculation=0;
    double currFPS=0;
    std::chrono::steady_clock::time_point lastTick;
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
            MLOGD<<NAME<<" "<<currFPS;
        }
    }
    float getCurrentFPS()const {
        return (float)currFPS;
    }
};

// Frame times are more meaningful than fps
// But can also be converted to fps for everyone to understand
class FrameTimeCalculator{
private:
    const std::string NAME;
    const std::chrono::steady_clock::duration LOG_INTERVAL;
    std::chrono::steady_clock::time_point lastLog=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastTick={};
    AvgCalculator avgCalculator;
    float currentFps=0;
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
            currentFps=(float)1000.0f * 1000.0f/std::chrono::duration_cast<std::chrono::microseconds>(avgCalculator.getAvg()).count();
            MLOGD<<NAME<<" FPS: "<<getCurrentFPS()<<"FT: "<<avgCalculator.getAvgReadable();
            avgCalculator.reset();
            lastLog=std::chrono::steady_clock::now();
        }
    }
    float getCurrentFPS()const {
        return currentFps;
    }
};

/**
 * Unfortunately, the clock speed of android devices can change massively even though the load stays pretty much the same
 * Unless the 'sustained performance mode' is available
 * This class tries to calculate a usable 'sleep time' such that roughly the wanted fps is achieved
 */
class FrameTimeLimiter{
public:
    using CLOCK=std::chrono::steady_clock;
    static constexpr const auto FRAME_TIME_30_FPS=std::chrono::nanoseconds(33300000);
    static constexpr const auto FRAME_TIME_60_FPS=std::chrono::nanoseconds(16600000);
    static constexpr const auto FRAME_TIME_90_FPS=std::chrono::nanoseconds(11100000);
    static constexpr const auto FRAME_TIME_120_FPS=std::chrono::nanoseconds(8330000);
private:
    const CLOCK::duration wantedAvgFrameTime;
    const std::chrono::steady_clock::duration RECALCULATION_INTERVAL=std::chrono::seconds(1);
    std::chrono::steady_clock::time_point lastRecalculation=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastTick={};
    AvgCalculator avgFrameTimeCalculator;
    CLOCK::duration currentWaitTime=std::chrono::nanoseconds(0);
public:
    FrameTimeLimiter(CLOCK::duration wantedAvgFrameTime1):
    wantedAvgFrameTime(wantedAvgFrameTime1){}
    /**
     * Call this in your drawing loop
     */
    void tick(){
        const auto now=std::chrono::steady_clock::now();
        if(lastTick!=std::chrono::steady_clock::time_point()){
            const auto delta=std::chrono::steady_clock::now()-lastTick;
            avgFrameTimeCalculator.add(delta);
            lastTick=std::chrono::steady_clock::now();
        }else{
            lastTick=std::chrono::steady_clock::now();
            return;
        }
        const auto deltaLastLog= now - lastRecalculation;
        if(deltaLastLog > RECALCULATION_INTERVAL){
            const auto avgFrameTime=avgFrameTimeCalculator.getAvg();
            // Positive value means we are rendering faster than wanted.
            // Negative value means we spent too much time rendering (or rendering +sleeping)
            const auto diff=wantedAvgFrameTime-avgFrameTime;
            // Adjust the sleep time slightly with each recalculation - e.g. increase / decrease by 2/3
            const auto diffTwoThirds=diff*3/2;
            MLOGD << "Diff " << MyTimeHelper::R(diff) << " Diff 2/3 " << MyTimeHelper::R(diffTwoThirds) << " " << avgFrameTimeCalculator.getAvgReadable();
            if(diff>std::chrono::nanoseconds(0)){
                currentWaitTime+=diffTwoThirds;
            }else{
                currentWaitTime+=diffTwoThirds;
                // If we do not have enough cpu & gpu resources to render at N fps we cannot
                // Do anything. Render as fast as possible in this case ( do not sleep)
                // But make sure that the wait time does not get negative, else the algorithm won't work if
                // suddenly more resources become available ( increased clock speed )
                if(currentWaitTime<std::chrono::nanoseconds(0)){
                    currentWaitTime=std::chrono::nanoseconds(0);
                }
            }
            avgFrameTimeCalculator.reset();
            lastRecalculation=CLOCK::now();
            MLOGD<<"Sleep time "<<MyTimeHelper::R(currentWaitTime);
        }
        if(currentWaitTime>std::chrono::nanoseconds(0)){
            std::this_thread::sleep_for(currentWaitTime);
        }
    }
};


#endif //FPV_VR_FPSCALCULATOR_H
