//
// Created by Constantin on 22.10.2017.
//

#ifndef OSDTESTER_CHRONOMETER_H
#define OSDTESTER_CHRONOMETER_H


#include <string>
#include <chrono>

class DurationAccumulator{
public:
    void add(const std::chrono::steady_clock::duration& duration){
        accDuration+=duration;
        acc++;
    }
    int64_t getAvgUS()const;
    float getAvgMS()const;
    void reset(){
        accDuration=std::chrono::nanoseconds::zero();
        acc=0;
    }
private:
    std::chrono::steady_clock::duration accDuration{0};
    int acc=0;
};

class Chronometer {
public:
    explicit Chronometer(std::string name);
    explicit Chronometer();
    void start();
    void stop();
    void reset();
    int64_t getAvgUS()const;
    float getAvgMS()const;
    void printAvg(const std::chrono::steady_clock::duration& interval);
private:
    const std::string mName;
    std::chrono::steady_clock::time_point startTS;
    std::chrono::steady_clock::time_point lastLog;
    DurationAccumulator average;
};


#endif //OSDTESTER_CHRONOMETER_H
