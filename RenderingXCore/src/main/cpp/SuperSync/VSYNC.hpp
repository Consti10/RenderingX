//
// Created by geier on 17/07/2020.
//

#ifndef RENDERINGX_VSYNC_HPP
#define RENDERINGX_VSYNC_HPP

#include <sys/types.h>
#include <queue>
#include <list>
#include <deque>
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
    static constexpr CLOCK::duration DEFAULT_REFRESH_TIME=16666666ns;
    CLOCK::time_point lastVSYNCFromChoreographer={};
    // display refresh rate becomes more accurately over time
    CLOCK::duration displayRefreshTime=DEFAULT_REFRESH_TIME;
    CLOCK::duration eyeRefreshTime= displayRefreshTime/2;
    // how many samples of the refresh rate I currently have to determine the accurate refresh rate
    AvgCalculator displayRefreshTimeCalculator;
    static constexpr const auto N_SAMPLES=60;
    std::deque<CLOCK::time_point> lastVSYNCs;
    int vsyncCount;
public:
    /**
     * pass the last vsync timestamp from java (setLastVSYNC) to cpp
     * @param newVSYNC as obtained by Choreographer.doFrame but without the offset(s)
     */
    void setVSYNCSentByChoreographer(const CLOCK::time_point newVSYNC){
        // on the first call the lastRegisteredVSYNC is unknown
        if(lastVSYNCFromChoreographer==CLOCK::time_point{}){
            lastVSYNCFromChoreographer=newVSYNC;
            return;
        }
        // Validate the new VSYNC value.
        // A value in the future is not possible (e.g. we should only get VSYNC events from the past)
        assert((CLOCK::now() - newVSYNC) >= 0ns);
        // Also the VSYNC values should be strictly increasing
        assert(newVSYNC > lastVSYNCFromChoreographer);

        const auto deltaBetweenVSYNCs= newVSYNC-lastVSYNCFromChoreographer;


        lastVSYNCs.push_back(newVSYNC);
        if(lastVSYNCs.size()>4){
            lastVSYNCs.pop_front();
        }
        AvgCalculator lulatsch;
        const auto currTime=CLOCK::now();
        for(int i=0;i<lastVSYNCs.size();i++){
            const auto & vsync=lastVSYNCs.at(i);
            auto latestVSYNC=calculateLatestVSYNC(currTime,vsync,displayRefreshTime);
            auto age=currTime-latestVSYNC;
            MLOGD<<"VSYNC from sample "<<i<<" is old:"<<MyTimeHelper::R(age);
            lulatsch.add(age);
        }
        if(lulatsch.getMaxDifferenceMinMaxAvg()>std::chrono::microseconds(100)){
            MLOGD<<"Exceeded: "<<lulatsch.getAvgReadable()<<" "<<MyTimeHelper::R(lulatsch.getMaxDifferenceMinMaxAvg());
        }

        MLOGD<<"X";


        // use the delta between VSYNC events to calculate the display refresh rate more accurately
        //Stop as soon we have n samples (that should be more than enough)
        if(displayRefreshTimeCalculator.getNSamples() < N_SAMPLES){
            //Assumption: There are only displays on the Market with refresh Rates that differ from 16.666ms +-n ms
            //This is needed because i am not sure if the vsync callbacks always get executed in the right order
            //so delta might be 32ms. In this case delta is not the display refresh time
            const auto minDisplayRR=DEFAULT_REFRESH_TIME-0.5ms;
            const auto maxDisplayRR=DEFAULT_REFRESH_TIME+0.5ms;
            if(deltaBetweenVSYNCs>minDisplayRR && deltaBetweenVSYNCs<maxDisplayRR){
                displayRefreshTimeCalculator.add(deltaBetweenVSYNCs);
                //we can use the average Value for "displayRefreshTime" when we have n samples
                if(displayRefreshTimeCalculator.getNSamples()==N_SAMPLES ) {
                    displayRefreshTime = displayRefreshTimeCalculator.getAvg();
                    eyeRefreshTime = displayRefreshTime / 2;
                    MLOGD<<"NEW DISPLAY_REFRESH_TIME "<<displayRefreshTimeCalculator.getAvgReadable();
                }
            }
        }
        if(!(deltaBetweenVSYNCs>16.4ms && deltaBetweenVSYNCs<16.8ms)){
             MLOGE<<"out of order delta: "<<MyTimeHelper::R(deltaBetweenVSYNCs);
        }
        const auto beforeVSYNCPositionNormalized=getVsyncRasterizerPositionNormalized();
        const auto beforeLatestVSYNC=getLatestVSYNC();
        const auto age1= CLOCK::now() - lastVSYNCFromChoreographer;
        const auto age2= CLOCK::now() - newVSYNC;
        //MLOGD<<"age1 "<<MyTimeHelper::R(age1)<<" age2 "<<MyTimeHelper::R(age2);
        lastVSYNCFromChoreographer=newVSYNC;

        //MLOGD<<"setLastVSYNC"<<lastVSYNC;
        const auto afterVSYNCPosition=getVsyncRasterizerPositionNormalized();
        const auto afterLatestVSYNC=getLatestVSYNC();
        const auto diffVsyncPositionNormalized=std::abs(afterVSYNCPosition - beforeVSYNCPositionNormalized);
        if(diffVsyncPositionNormalized>0.02f){
            // MLOGE<<"VSYNC changed too much: "<<diffVsyncPositionNormalized;
        }
        const auto diffVsyncBeforeAndAFter=afterLatestVSYNC-beforeLatestVSYNC;
        if(diffVsyncBeforeAndAFter> 1ms){
            // MLOGE<<"VSYNC changed in between: "<<MyTimeHelper::R(diffVsyncBeforeAndAFter);
        }
    }
    /**
     * Java System.nanoTime is the same as std::chrono::steady_clock::time_point
     * @param newVSYNC: value in ns as obtained by choreographer without offset(s)
     */
    void setVSYNCSentByChoreographer(const uint64_t newVSYNC){
        setVSYNCSentByChoreographer(CLOCK::time_point(std::chrono::nanoseconds(newVSYNC)));
    }
    /**
    * @return The timestamp of EXACTLY the last VSYNC event, or in other words the rasterizer being at 0
     * This value is guaranteed to be in the past and its age is not more than displayRefreshTime
    */
    CLOCK::time_point getLatestVSYNC()const{
        return calculateLatestVSYNC(CLOCK::now(),lastVSYNCFromChoreographer,displayRefreshTime);
    }
    // return the latest VSYNC that happened before currTime
    static CLOCK::time_point calculateLatestVSYNC(const CLOCK::time_point currTime,const CLOCK::time_point vsync,const CLOCK::duration displayRefreshTime){
        auto vsyncBase=vsync;
        const std::chrono::nanoseconds delta=currTime-vsyncBase;
        const int64_t factor=delta.count()/displayRefreshTime.count();
        vsyncBase+=factor*displayRefreshTime;
        return vsyncBase;
    }
    /**
     * @return The current rasterizer position, with range: 0<=position<DISPLAY_REFRESH_TIME
     * For example, a value of 0 means that the rasterizer is at the most left position of the display in landscape mode
     * and a value of DISPLAY_REFRESH_TIME means the rasterizer is at its right most position
     */
    int64_t getVsyncRasterizerPosition()const{
        const int64_t position=std::chrono::duration_cast<std::chrono::nanoseconds>(CLOCK::now() - lastVSYNCFromChoreographer).count();
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
