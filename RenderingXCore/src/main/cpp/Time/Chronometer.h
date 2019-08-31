//
// Created by Constantin on 22.10.2017.
//

#ifndef OSDTESTER_CHRONOMETER_H
#define OSDTESTER_CHRONOMETER_H


#include <string>
#include <chrono>

class Chronometer {
public:
    explicit Chronometer(const std::string& name);
    explicit Chronometer();
    void start();
    void stop();
    void reset();
    const int64_t getAvgUS()const;
    const float getAvgMS()const;
    void printAvg(int intervalMS);
private:
    const std::string mName;
    std::chrono::steady_clock::time_point startTS;
    std::chrono::steady_clock::time_point lastLog;
    uint64_t timeSumUS;
    int timeCount;
};


#endif //OSDTESTER_CHRONOMETER_H
