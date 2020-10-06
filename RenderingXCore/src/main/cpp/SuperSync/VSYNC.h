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
#include <ATraceCompbat.hpp>
#include <jni.h>


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
    // A VSYNC state consists of a the time point when it occurred and a count
    // which is increasing which each occurred vsync
    struct VSYNCState{
        CLOCK::time_point base;
        int count=-1;
    };
    static inline jlong jptr(VSYNC *p) {
        return reinterpret_cast<intptr_t>(p);
    }
    static inline VSYNC *native(jlong ptr) {
        return reinterpret_cast<VSYNC*>(ptr);
    }
    // When passed 0,create new VSYNC
    // Else,convert to VSYNC*
    static inline VSYNC *createFrom(jlong ptr){
        if(ptr==0)return new VSYNC();
        return native(ptr);
    }
private:
    // last registered VSYNC. Indirectly set by the callback, but it is not guaranteed that this is the last occurred VSYNC
    // The only thread writing the variable is the choreographer
    // Any other thread can read it without worrying about locks, for example the Front Buffer Renderer
    std::atomic<VSYNCState> lastVSYNCStateFromChoreographer;
    static constexpr CLOCK::duration DEFAULT_REFRESH_TIME=16666666ns;
    // display refresh rate becomes more accurately over time when I got enough samples
    CLOCK::duration displayRefreshTime=DEFAULT_REFRESH_TIME;
    CLOCK::duration eyeRefreshTime= displayRefreshTime/2;
    AvgCalculator displayRefreshTimeCalculator;
    static constexpr const auto N_SAMPLES=60;
public:
    /**
     * pass the last vsync timestamp from java (setLastVSYNC) to cpp
     * @param newVSYNC as obtained by Choreographer.doFrame but without the offset(s)
     */
    void setVSYNCSentByChoreographer(const CLOCK::time_point newVSYNC){
        ATrace_beginSection("setVSYNCSentByChoreographer");
        // When count is 0 the first timestamp ever is recorded. Do not validate,
        // Just register and then return
        const auto tmp=lastVSYNCStateFromChoreographer.load();
        if(tmp.count==-1){
            lastVSYNCStateFromChoreographer.store({newVSYNC,0});
            return;
        }
        // Validate the new VSYNC value.
        // A value in the future is not possible (e.g. we should only get VSYNC events from the past)
        assert((CLOCK::now() - newVSYNC) >= 0ns);
        // Also the VSYNC values should be strictly increasing
        assert(newVSYNC > tmp.base);

        const auto deltaBetweenVSYNCs= newVSYNC-tmp.base;

        const auto latestVSYNCBefore=getLatestVSYNC();
        if(!isInRange(deltaBetweenVSYNCs,displayRefreshTime-1ms,displayRefreshTime+1ms)){
            // The elapsed time between the last and new VSYNC is greatly bigger/smaller than the display refresh time
            // I cannot determine what the new 'count' should be
            MLOGE<<"Big/small out of order delta: "<<MyTimeHelper::R(deltaBetweenVSYNCs);
        }
        lastVSYNCStateFromChoreographer.store({newVSYNC,tmp.count+1});

        const auto latestVSYNCAfter=getLatestVSYNC();
        if(latestVSYNCAfter.count!=latestVSYNCBefore.count){
            MLOGE<<"VSYNC count changed out of order old:"<<latestVSYNCAfter.count<<" new:"<<latestVSYNCAfter.count;
        }
        // use the delta between VSYNC events to calculate the display refresh rate more accurately
        //Stop as soon we have n samples (that should be more than enough)
        if(displayRefreshTimeCalculator.getNSamples() < N_SAMPLES){
            //Assumption: There are only displays on the Market with refresh Rates that differ from 16.666ms +-n ms
            //This is needed because vsync callbacks might be dropped or executed at the wrong time
            //so delta might be 32ms. In this case delta is not the display refresh time
            const auto minDisplayRR=DEFAULT_REFRESH_TIME-std::chrono::duration_cast<std::chrono::nanoseconds>(0.5ms);
            const auto maxDisplayRR=DEFAULT_REFRESH_TIME+std::chrono::duration_cast<std::chrono::nanoseconds>(0.5ms);
            if(isInRange(deltaBetweenVSYNCs,minDisplayRR,maxDisplayRR)){
                displayRefreshTimeCalculator.add(deltaBetweenVSYNCs);
                //we can use the average Value for "displayRefreshTime" when we have n samples
                if(displayRefreshTimeCalculator.getNSamples()==N_SAMPLES ) {
                    displayRefreshTime = displayRefreshTimeCalculator.getAvg();
                    eyeRefreshTime = displayRefreshTime / 2;
                    MLOGD<<"NEW DISPLAY_REFRESH_TIME "<<displayRefreshTimeCalculator.getAvgReadable();
                }
            }
        }
        ATrace_endSection();
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
    VSYNCState getLatestVSYNC()const{
        return calculateLatestVSYNCFromBase(CLOCK::now(),lastVSYNCStateFromChoreographer.load(),displayRefreshTime);
    }
    /**
     * Given @param vsyncState and @param displayRefreshTime this method
     * calculates the proper count and base for the latest VSYNC that occurred before @param currTime
     */
    static VSYNCState calculateLatestVSYNCFromBase(const CLOCK::time_point currTime,const VSYNCState vsyncState,const CLOCK::duration displayRefreshTime){
        const std::chrono::nanoseconds delta=currTime-vsyncState.base;
        const int64_t factor=delta.count()/displayRefreshTime.count();
        //MLOGD<<"Factor "<<factor;
        const VSYNCState ret{vsyncState.base+factor*displayRefreshTime,(int)(vsyncState.count+factor)};
        //auto age=CLOCK::now()-ret.base;
        //if(age>displayRefreshTime){
        //    MLOGE<<"HUHU"<<MyTimeHelper::R(age);
        //}
        return ret;
    }
    /**
     * @return The current rasterizer position, with range: 0<=position<DISPLAY_REFRESH_TIME
     * For example, a value of 0 means that the rasterizer is at the most left position of the display in landscape mode
     * and a value of DISPLAY_REFRESH_TIME means the rasterizer is at its right most position
     */
    int64_t getVsyncRasterizerPosition()const{
        const int64_t position=std::chrono::duration_cast<std::chrono::nanoseconds>(CLOCK::now() - lastVSYNCStateFromChoreographer.load().base).count();
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
private:
    // return true if the given duration is inside the intervall [min,max]
    static bool isInRange(CLOCK::duration value,CLOCK::duration min,CLOCK::duration max){
        return value>=min && value<=max;
    }
};
using CLOCK=VSYNC::CLOCK;

#endif //RENDERINGX_VSYNC_HPP
