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
    FPSCalculator(std::string name="FPSCalculator",std::chrono::steady_clock::duration intervall=std::chrono::seconds(1));
    void tick();
    float getCurrentFPS()const;
};


#endif //FPV_VR_FPSCALCULATOR_H
