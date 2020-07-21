//
// Created by geier on 17/07/2020.
//

#ifndef RENDERINGX_VSYNC_HPP
#define RENDERINGX_VSYNC_HPP

#include <sys/types.h>
#include <TimeHelper.hpp>


// Helper to obtain the current VSYNC position (e.g. which scan line is currently read out)
// While ideally I would like to use the exact data from the display manufacturer, I have to use the
// Choreographer as workaround on android. This means that more advanced configuration data like
// *front and back porch* cannot be taken into consideration. However, even with the assumption
// that the VSYNC moves from [scan line 0 to SCREEN_WIDTH] in the display refresh time intervall FBR works
using namespace std::chrono_literals;
class VSYNC{
public:
    // Java System.nanoTime is the same as std::chrono::steady_clock
    using CLOCK=std::chrono::steady_clock;
private:
    // last registered VSYNC. Indirectly set by the callback, but it is guaranteed that this value
    // is never smaller than the current system time in NS (In other words, the timestamp
    // always lies in the past, never in the future). However, it is not guaranteed to be the
    // 'last' VSYNC - see getLatestVSYNC()
    CLOCK::time_point lastRegisteredVSYNC;
    static constexpr CLOCK::duration DEFAULT_REFRESH_TIME=16666666ns;
    // display refresh rate becomes more accurately over time
    CLOCK::duration displayRefreshTime=DEFAULT_REFRESH_TIME;
    CLOCK::duration eyeRefreshTime= displayRefreshTime/2;
    // how many samples of the refresh rate I currently have to determine the accurate refresh rate
    AvgCalculator displayRefreshTimeCalculator;
    CLOCK::time_point previousVSYNCSetByCallback;
    static constexpr const auto N_SAMPLES=600;
public:
    /**
     * pass the last vsync timestamp from java (setLastVSYNC) to cpp
     * @param lastVSYNC as obtained by Choreographer.doFrame but without the offset(s)
     */
    void setLastVSYNC(const CLOCK::time_point lastVSYNC){
        //MLOGD<<"setLastVSYNC"<<lastVSYNC;
        // Make sure we do not have a 'future' VSYNC.
        // E.g. a VSYNC that is about to happen (we want the 'latest' or at least a previous VSYNC)
        {
            const auto systemTime=CLOCK::now();
            auto tmp=lastVSYNC;
            while(tmp>systemTime){
                tmp-=displayRefreshTime;
            }
            lastRegisteredVSYNC=tmp;
            // Check that the registered VSYNC does not lie in the future
            assert((systemTime-lastRegisteredVSYNC) >= 0ns);
        }
        //Stop as soon we have n samples (that should be more than enough)
        if(displayRefreshTimeCalculator.getNSamples() < N_SAMPLES){
            const auto delta= lastVSYNC - previousVSYNCSetByCallback;
            if(delta>0ns){
                //Assumption: There are only displays on the Market with refresh Rates that differ from 16.666ms +-1.0ms
                //This is needed because i am not sure if the vsync callbacks always get executed in the right order
                //so delta might be 32ms. In this case delta is not the display refresh time
                const auto minDisplayRR=DEFAULT_REFRESH_TIME-1ms;
                const auto maxDisplayRR=DEFAULT_REFRESH_TIME+1ms;
                if(delta>minDisplayRR && delta<maxDisplayRR){
                    displayRefreshTimeCalculator.add(delta);
                    //we can start using the average Value for "displayRefreshTime" when we have roughly n samples
                    if(displayRefreshTimeCalculator.getNSamples() > 120){
                        displayRefreshTime= displayRefreshTimeCalculator.getAvg();
                        eyeRefreshTime= displayRefreshTime / 2;
                    }
                }
            }
            previousVSYNCSetByCallback=lastVSYNC;
            //MLOGD<<"delta "<<MyTimeHelper::R(delta);
        } //else We have at least n samples. This is enough.
        //MLOGD<<"DISPLAY_REFRESH_TIME "<<MyTimeHelper::R(displayRefreshTime);
    }
    /**
     * Java System.nanoTime is the same as std::chrono::steady_clock::time_point
     * @param lastVSYNC: value in ns as obtained by choreographer without offset(s)
     */
    void setLastVSYNC(const uint64_t lastVSYNC){
        setLastVSYNC(CLOCK::time_point(std::chrono::nanoseconds(lastVSYNC)));
    }
    /**
    * @return The timestamp of EXACTLY the last VSYNC event, or in other words the rasterizer being at 0
     * This value is guaranteed to be in the past and its age is not more than displayRefreshTime
    */
    CLOCK::time_point getLatestVSYNC()const{
        const auto currTime=CLOCK::now();
        const auto currDisplayRefreshTime=displayRefreshTime;
        auto lastVSYNC=lastRegisteredVSYNC;
        const auto delta=currTime-lastVSYNC;
        const int64_t factor=delta/currDisplayRefreshTime;
        lastVSYNC+=factor*currDisplayRefreshTime;
        return lastVSYNC;
    }
    /**
     * @return The current rasterizer position, with range: 0<=position<DISPLAY_REFRESH_TIME
     * For example, a value of 0 means that the rasterizer is at the most left position of the display in landscape mode
     * and a value of DISPLAY_REFRESH_TIME means the rasterizer is at its right most position
     */
    int64_t getVsyncRasterizerPosition()const{
        const int64_t position=std::chrono::duration_cast<std::chrono::nanoseconds>(CLOCK::now()-lastRegisteredVSYNC).count();
        //auto lastRegisteredVsyncAge=position / DISPLAY_REFRESH_TIME;
        //MLOGD<<"last registered vsync is "<<lastRegisteredVsyncAge<<" events old";
        // It is possible that the last registered VSYNC is not the latest VSYNC, but a number of events in the past
        // The less accurate the DISPLAY_REFRESH_TIME is and the older the last registered VSYNC the more inaccurate this value becomes
        return position % std::chrono::duration_cast<std::chrono::nanoseconds>(displayRefreshTime).count();
    }
    // same as above but position is in range ( 0.0f ... 1.0f)
    float getVsyncRasterizerPositionNormalized()const{
        const auto pos=getVsyncRasterizerPosition();
        return (float)pos/std::chrono::duration_cast<std::chrono::nanoseconds>(displayRefreshTime).count();
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
