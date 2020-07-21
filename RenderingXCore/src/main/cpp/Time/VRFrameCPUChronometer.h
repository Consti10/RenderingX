//
// Created by Constantin on 25.11.2017.
//

#ifndef FPV_VR_FRAMECPUCHRONOMETER_H
#define FPV_VR_FRAMECPUCHRONOMETER_H


#include <string>
#include <utility>
#include <vector>
#include <tuple>
#include <chrono>
#include <sstream>
#include <android/log.h>
#include <TimeHelper.hpp>

/**
 * Helper to measure the time spend by the CPU on various different OpenGL calls
 * Helps when rendering vr frames where both left and right eye see the same scene
 */

//In FPV-VR, each frame the OpenGL calls/ Program flow is the same for left and right eye
//measures the durations of OpenGL cpu-side calls for one VR Eye (half a frame)
class VREyeDurations{
public:
    VREyeDurations(const bool whichEye):whichEye{whichEye}{
        startTS= std::chrono::steady_clock::now();
    }
    void setTimestamp(const std::string name){
        const auto now= std::chrono::steady_clock::now();
        const auto delta=now-startTS;
        startTS=now;
        stamps.push_back(std::make_tuple(name,delta));
        totalTime+=delta;
    }
    void print();
    const bool whichEye;
    std::vector<std::tuple<std::string,std::chrono::steady_clock::duration>> stamps{};
    std::chrono::steady_clock::duration totalTime{0};
private:
    std::chrono::steady_clock::time_point startTS;
};

//Each Frame consists of rendering 2 eyes in exactly the same manner
class VRFrameDurations{
public:
    explicit VRFrameDurations(std::string name):
            name(std::move(name)){
    }
    void reset(){
        leftEye.reset();
        rightEye.reset();
    }
    const std::string name;
    AvgCalculator leftEye;
    AvgCalculator rightEye;
};

//convenient debugging of the time spent on OpenGL calls/ cpp code execution
//accumulates times spent on left and right eye
class VRFrameTimeAccumulator{
public:
    explicit VRFrameTimeAccumulator(const std::vector<std::string>& timestampNames);
    void add(const VREyeDurations& timeStamps);
    void print();
    void printEveryXSeconds(int intervalS);
    void reset();
private:
    std::vector<std::unique_ptr<VRFrameDurations>> mFrameCPUTimestamps;
    AvgCalculator leftEyeTotal;
    AvgCalculator rightEyeTotal;
    std::chrono::steady_clock::time_point lastLog;
};




#endif //FPV_VR_FRAMECPUCHRONOMETER_H
