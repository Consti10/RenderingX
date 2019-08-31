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
    const int LOG_INTERVAL_MS;
    std::chrono::steady_clock::time_point lastFPSCalculation=std::chrono::steady_clock::now();
    double ticksSinceLastFPSCalculation=0;
    double currFPS;
public:
    FPSCalculator(const std::string& name,int printIntervalMS);
    void tick();
    float getCurrentFPS();
};


#endif //FPV_VR_FPSCALCULATOR_H
