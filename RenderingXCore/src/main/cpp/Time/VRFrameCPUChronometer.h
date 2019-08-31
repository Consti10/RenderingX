//
// Created by Constantin on 25.11.2017.
//

#ifndef FPV_VR_FRAMECPUCHRONOMETER_H
#define FPV_VR_FRAMECPUCHRONOMETER_H


#include <string>
#include <utility>
#include <vector>
#include "Chronometer.h"

/**
 * Helper to measure the time spend by the CPU on various different OpenGL calls
 * Helps when rendering vr frames where both left and right eye see the same scene
 */

class VRFrameCPUChronometer {
public:
    explicit VRFrameCPUChronometer(const std::vector<std::string>& timestampNames);
    void start(bool whichEye);
    void setTimestamp(bool whichEye,int whichTimestamp);
    void stop(bool whichEye);
    void print();
    void resetTS();
private:
    class FrameCPUTimestamp{
    public:
        explicit FrameCPUTimestamp(std::string name):name(std::move(name)){
            reset();
        }
        void reset(){
            leftEye.reset();
            rightEye.reset();
        }
        const std::string name;
        Chronometer leftEye;
        Chronometer rightEye;
    };
    std::vector<std::unique_ptr<FrameCPUTimestamp>> mFrameCPUTimestamps;
    Chronometer leftEyeFull;
    Chronometer rightEyeFull;
    std::chrono::steady_clock::time_point lastLog;
};


#endif //FPV_VR_FRAMECPUCHRONOMETER_H
