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
// Choreographer as workaround on android. This means that more advanced configuration data like
// *front and back porch* cannot be taken into consideration
class VSYNC{
private:
    // last registered VSYNC. Indirectly set by the callback, but it is guaranteed that this value
    // is never smaller than the current system time in NS (In other words, the timestamp
    // always lies in the past, never in the future. However, it is not guaranteed to be the
    // 'last' VSYNC - see getLatestVSYNC()
    int64_t lastRegisteredVSYNC=0;
    static constexpr int64_t DEFAULT_REFRESH_TIME=16666666;
    // display refresh rate becomes more accurately over time
    int64_t displayRefreshTime=DEFAULT_REFRESH_TIME;
    int64_t eyeRefreshTime= displayRefreshTime/2;
    // how many samples of the refresh rate I currently have to determine the accurate refresh rate
    uint64_t displayRefreshTimeSum=0,displayRefreshTimeC=0;
    int64_t previousVSYNCSetByCallback=0;
    static constexpr const auto N_SAMPLES=600;
public:
    using CLOCK=std::chrono::steady_clock;
    /**
     * Return the current time in ns (same as java System.nanoseconds)
     */
    static cNanoseconds getSystemTimeNS(){
        const auto time=std::chrono::steady_clock::now().time_since_epoch();
        return (cNanoseconds)std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
    }
    /**
     * pass the last vsync timestamp from java (setLastVSYNC) to cpp
     * @param lastVSYNC as obtained by Choreographer.doFrame but without the offset(s)
     */
    void setLastVSYNC(const int64_t lastVSYNC){
        //MLOGD<<"setLastVSYNC"<<lastVSYNC;
        // Make sure we do not have a 'future' VSYNC.
        // E.g. a VSYNC that is about to happen (we want the 'latest' or at least a previous VSYNC)
        {
            const auto systemTime=getSystemTimeNS();
            int64_t tmp=lastVSYNC;
            while(tmp>systemTime){
                tmp-=displayRefreshTime;
            }
            lastRegisteredVSYNC=tmp;
            // Check that the registered VSYNC does not lie in the future
            assert((systemTime-lastRegisteredVSYNC) >= 0);
        }
        //Stop as soon we have n samples (that should be more than enough)
        if(displayRefreshTimeC<N_SAMPLES){
            const int64_t delta= lastVSYNC - previousVSYNCSetByCallback;
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
            previousVSYNCSetByCallback=lastVSYNC;
            //MLOGD<<"delta"<<MyTimeHelper::R(std::chrono::nanoseconds(delta));
        } //else We have at least n samples. This is enough.
        //MLOGD<<"DISPLAY_REFRESH_TIME"<<MyTimeHelper::R(std::chrono::nanoseconds(DISPLAY_REFRESH_TIME));
    }
    /**
    * @return The timestamp of EXACTLY the last VSYNC event, or in other words the rasterizer being at 0
     * This value is guaranteed to be in the past and its age is not more than displayRefreshTime
    */
    CLOCK::time_point getLatestVSYNC()const{
        const auto currTime=getSystemTimeNS();
        const auto currDisplayRefreshTime=displayRefreshTime;
        int64_t lastVSYNC=lastRegisteredVSYNC;
        const auto delta=currTime-lastVSYNC;
        const int64_t factor=delta/currDisplayRefreshTime;
        lastVSYNC+=factor*currDisplayRefreshTime;
        return CLOCK::time_point(std::chrono::nanoseconds(lastVSYNC));
    }
    /**
     * Rough estimation of the rasterizer position ( I do not know blanking usw)
     * @return The current rasterizer position, with range: 0<=position<DISPLAY_REFRESH_TIME
     * For example, a value of 0 means that the rasterizer is at the most left position of the display in landscape mode
     * and a value of DISPLAY_REFRESH_TIME means the rasterizer is at its right most position
     */
    int64_t getVsyncRasterizerPosition()const{
        const int64_t position=getSystemTimeNS()-lastRegisteredVSYNC;
        //auto lastRegisteredVsyncAge=position / DISPLAY_REFRESH_TIME;
        //MLOGD<<"last registered vsync is "<<lastRegisteredVsyncAge<<" events old";
        // It is possible that the last registered VSYNC is not the latest VSYNC, but a number of events in the past
        // The less accurate the DISPLAY_REFRESH_TIME is and the older the last registered VSYNC the more inaccurate this value becomes
        return position % displayRefreshTime;
    }
    // same as above but position is in range ( 0.0f ... 1.0f)
    float getVsyncRasterizerPositionNormalized()const{
        const auto pos=getVsyncRasterizerPosition();
        return (float)pos/displayRefreshTime;
    }
public:
    CLOCK::duration getDisplayRefreshTime()const{
        return CLOCK::duration(std::chrono::nanoseconds(displayRefreshTime));
    }
    CLOCK::duration getEyeRefreshTime()const{
        return  CLOCK::duration(std::chrono::nanoseconds(eyeRefreshTime));
    }
};

#endif //RENDERINGX_VSYNC_HPP
