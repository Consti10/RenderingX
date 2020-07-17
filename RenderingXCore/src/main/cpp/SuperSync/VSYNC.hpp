//
// Created by geier on 17/07/2020.
//

#ifndef RENDERINGX_VSYNC_HPP
#define RENDERINGX_VSYNC_HPP

#include <sys/types.h>
#include <Chronometer.h>
#include <TimeHelper.hpp>

// https://stackoverflow.com/questions/43451565/store-timestamps-in-nanoseconds-c
// using int64_t for nanoseconds is safe until we are all dead
using cNanoseconds=int64_t;

// Helper to obtain the current VSYNC position (e.g. which scan line is currently read out)
// While ideally I would like to use the exact data from the display manufacturer, I have to use the
// Choreographer as workaround on android
class VSYNC{
private:
    // last VSYNC set by the callback
    int64_t lastRegisteredVSYNC=0;
    // display refresh rate becomes more accurately over time
    static constexpr int64_t DEFAULT_REFRESH_TIME=16666666;
    int64_t displayRefreshTime=DEFAULT_REFRESH_TIME;
    int64_t eyeRefreshTime= displayRefreshTime/2;
    // how many samples of the refresh rate I currently have to determine the accurate refresh rate
    uint64_t displayRefreshTimeSum=0,displayRefreshTimeC=0;
    int64_t previousVSYNC=0;
    static constexpr const auto N_SAMPLES=600;
public:
    /**
     * Return the current time in ns (same as java System.nanoseconds)
     */
    static cNanoseconds getSystemTimeNS(){
        const auto time=std::chrono::steady_clock::now().time_since_epoch();
        return (cNanoseconds)std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
    }
    //pass the last vsync timestamp from java (setLastVSYNC) to cpp
    /**
     * pass the last vsync timestamp from java (setLastVSYNC) to cpp
     * @param lastVSYNC as obtained by Choreographer.doFrame but without the offset(s)
     */
    void setLastVSYNC(const int64_t lastVSYNC){
        //MLOGD<<"setLastVSYNC"<<lastVSYNC;
        // Make sure we do not have a 'future' VSYNC.
        // E.g. a VSYNC that is about to happen (we want the 'latest' or at least a previous VSYNC)
        int64_t tmp=lastVSYNC;
        while(getSystemTimeNS()<tmp){
            tmp-=displayRefreshTime;
        }
        lastRegisteredVSYNC=tmp;
        //Stop as soon we have n samples (that should be more than enough)
        if(displayRefreshTimeC<N_SAMPLES){
            const int64_t delta=lastVSYNC-previousVSYNC;
            if(delta>0){
                //Assumption: There are only displays on the Market with refresh Rates that differ from 16.666ms +-1.0ms
                //This is needed because i am not sure if the vsync callbacks always get executed in the right order
                //so delta might be 32ms. In this case delta is not the display refresh time
                const int64_t minDisplayRR=DEFAULT_REFRESH_TIME-1000000;
                const int64_t maxDisplayRR=DEFAULT_REFRESH_TIME+1000000;
                if(delta>minDisplayRR&&delta<maxDisplayRR){
                    displayRefreshTimeSum+=delta;
                    displayRefreshTimeC++;
                    //we can start using the average Value for "displayRefreshTime" when we have roughly n samples
                    if(displayRefreshTimeC>120){
                        displayRefreshTime= displayRefreshTimeSum / displayRefreshTimeC;
                        eyeRefreshTime= displayRefreshTime / 2;
                    }
                }
            }
            previousVSYNC=lastVSYNC;
            //MLOGD<<"delta"<<MyTimeHelper::R(std::chrono::nanoseconds(delta));
        } //else We have at least n samples. This is enough.
        //MLOGD<<"DISPLAY_REFRESH_TIME"<<MyTimeHelper::R(std::chrono::nanoseconds(DISPLAY_REFRESH_TIME));
    }
    /**
     * Rough estimation of the rasterizer position ( I do not know blanking usw)
     * @return The current rasterizer position, with range: 0<=position<DISPLAY_REFRESH_TIME
     * For example, a value of 0 means that the rasterizer is at the most left position of the display in landscape mode
     * and a value of DISPLAY_REFRESH_TIME means the rasterizer is at its right most position
     */
    int64_t getVsyncRasterizerPosition(){
        const int64_t position=getSystemTimeNS()-lastRegisteredVSYNC;
        if(position<0){
           MLOGE<<" getVsyncRasterizerPosition - lastRegisteredVSYNC is in the future"<<getSystemTimeNS()<<" "<<lastRegisteredVSYNC;
        }
        //auto lastRegisteredVsyncAge=position / DISPLAY_REFRESH_TIME;
        //MLOGD<<"last registered vsync is "<<lastRegisteredVsyncAge<<" events old";
        // It is possible that the last registered VSYNC is not the latest VSYNC, but a number of events in the past
        // The less accurate the DISPLAY_REFRESH_TIME is and the older the last registered VSYNC the more inaccurate this value becomes
        return position % displayRefreshTime;
    }
public:
    int64_t getDisplayRefreshTime()const{
        return displayRefreshTime;
    }
    int64_t getEyeRefreshTime()const{
        return eyeRefreshTime;
    }
};

#endif //RENDERINGX_VSYNC_HPP
