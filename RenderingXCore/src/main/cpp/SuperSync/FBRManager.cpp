//
// Created by Constantin on 23.11.2016.
//
#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sstream>
#include "FBRManager.h"
#include "Extensions.hpp"
#include <AndroidLogger.hpp>

//#define RELEASE
constexpr auto MS_TO_NS=1000*1000;

using namespace std::chrono;

static std::chrono::steady_clock::duration ThisThreadSleepUntil(std::chrono::steady_clock::time_point tp){
    std::this_thread::sleep_until(tp);
    const auto now=std::chrono::steady_clock::now();
    return now-tp;
}

FBRManager::FBRManager(bool qcomTiledRenderingAvailable,bool reusableSyncAvailable,const RENDER_NEW_EYE_CALLBACK onRenderNewEyeCallback,const ERROR_CALLBACK onErrorCallback):
        directRender{qcomTiledRenderingAvailable},
        EGL_KHR_Reusable_Sync_Available(reusableSyncAvailable),
        onRenderNewEyeCallback(onRenderNewEyeCallback),
        onErrorCallback(onErrorCallback)
{
    KHR_fence_sync::init();
    Extensions::initOtherExtensions();
    lastLog=steady_clock::now();
    resetTS();
}

void FBRManager::drawLeftAndRightEye(JNIEnv* env) {
    const auto vsyncPosition=getVsyncRasterizerPosition();
    // VSYNC is currently scanning the left eye area
}


void FBRManager::enterDirectRenderingLoop(JNIEnv* env,int SCREEN_W,int SCREEN_H) {
    shouldRender= true;
    CLOCK::time_point lastRenderedVsync=CLOCK::now();

    while(shouldRender){
        const auto latestVSYNC=getLatestVSYNC();
        const auto nextVSYNCMiddle=latestVSYNC+getEyeRefreshTime()+std::chrono::milliseconds(0);
        const auto nextVSYNC=latestVSYNC+getDisplayRefreshTime()+std::chrono::milliseconds(0);
        //MLOGD<<"latestVSYNC"<<MyTimeHelper::R(CLOCK::now()-latestVSYNC)<<" nextVSYNCMiddle "<<MyTimeHelper::R(CLOCK::now()-nextVSYNCMiddle)<<" nextVSYNC "<<MyTimeHelper::R(CLOCK::now()-nextVSYNC);
        if(getVsyncRasterizerPositionNormalized()>0.1f){
            MLOGE<<"XYZ VSYNC should be at the beginning "<<getVsyncRasterizerPositionNormalized();
        }
        const auto diff=latestVSYNC-lastRenderedVsync;
        MLOGD<<"VSYNC diff "<<MyTimeHelper::R(diff);
        lastRenderedVsync=latestVSYNC;
        //
        // wait until nextVSYNCMiddle
        // Render left eye
        // wait until nextVSYNC
        // Render right eye
        // -> latestVSYNC becomes nextVSYNC
        //MLOGD<<"VSYNC rasterizer position "<<getVsyncRasterizerPositionNormalized();

        // Render right eye (1) first, then left eye(0)
        for(int eye=1;eye>=0;eye--){
            if(!shouldRender){
                break;
            }
            const bool isLeftEye=eye==0;
            //render new eye (right eye first)
            eyeChrono[eye].avgCPUTime.start();
            directRender.begin(getViewportForEye(isLeftEye,SCREEN_W,SCREEN_H));
            onRenderNewEyeCallback(env,isLeftEye);
            directRender.end();
            eyeChrono[eye].avgCPUTime.stop();
            std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
            glFlush();
            vsyncWaitTime[eye].start();
            if(eye==1){
                waitUntilTimePoint(nextVSYNCMiddle,*fenceSync);
            }else{
                waitUntilTimePoint(nextVSYNC,*fenceSync);
            }
            //MLOGD<<"Vsync pos "<<getVsyncRasterizerPositionNormalized();
            eyeChrono[eye].nEyes++;
            if(fenceSync->hasAlreadyBeenSatisfied()){
                eyeChrono[eye].avgGPUTime.add(fenceSync->getDeltaCreationSatisfied());
            }else{
                MLOGE<<"Couldnt measure GPU time";
                eyeChrono[eye].nEyesNotMeasurable++;
            }
            fenceSync.reset(nullptr);
            vsyncWaitTime[eye].stop();
            MLOGD<<"VSYNC pos "<<getVsyncRasterizerPositionNormalized();
        }
        printLog();
    }
}

void FBRManager::requestExitSuperSyncLoop() {
    shouldRender= false;
}

void FBRManager::waitUntilTimePoint(const std::chrono::steady_clock::time_point& timePoint,FenceSync& fenceSync) {
    auto diff=std::chrono::steady_clock::now()-timePoint;
    if(diff>std::chrono::milliseconds(1)){
        MLOGE<<"offset0 (overshoot)"<< MyTimeHelper::R(diff);
    }
    const auto timeLeft1=std::chrono::steady_clock::now()-timePoint;
    //MLOGD<<"time left1: "<<MyTimeHelper::R(timeLeft1);
    fenceSync.wait(timeLeft1);
    //const auto timeLeft2=std::chrono::steady_clock::now()-timePoint;
    //MLOGD<<"time left 2: "<< MyTimeHelper::R(timeLeft2);
    diff=std::chrono::steady_clock::now()-timePoint;
    if(diff>std::chrono::milliseconds(1)){
        MLOGE<<"offset1 (overshoot)"<< MyTimeHelper::R(diff);
    }
    std::this_thread::sleep_until(timePoint);
    //while (CLOCK::now()<timePoint){
    //    //
    //}
    diff=std::chrono::steady_clock::now()-timePoint;
    if(diff>std::chrono::milliseconds(1)){
        MLOGE<<"offset2 (overshoot)"<< MyTimeHelper::R(diff);
    }
    //return std::chrono::duration_cast<std::chrono::nanoseconds>(offset).count();
}


void FBRManager::printLog() {
    const auto now=steady_clock::now();
    if(now-lastLog>std::chrono::seconds(5)){//every 5 seconds
        lastLog=now;
        auto& leChrono=eyeChrono[0];
        auto& reChrono=eyeChrono[1];
        double leGPUTimeNotMeasurablePerc=0;
        double reGPUTimeNotMeasurablePerc=0;
        double leAreGPUTimeNotMeasurablePerc=0;
        if(leChrono.nEyes > 0){
            leGPUTimeNotMeasurablePerc= (leChrono.nEyesNotMeasurable / leChrono.nEyes) * 100.0;
        }
        if(reChrono.nEyes > 0){
            reGPUTimeNotMeasurablePerc= (reChrono.nEyesNotMeasurable / reChrono.nEyes) * 100.0;
        }
        leAreGPUTimeNotMeasurablePerc=(leGPUTimeNotMeasurablePerc+reGPUTimeNotMeasurablePerc)*0.5;
        std::ostringstream avgLog;
        avgLog<<"------------------------FBRManager Averages------------------------";
        avgLog << "\nCPU Time: "<<"leftEye: " << leChrono.avgCPUTime.getAvgReadable() << " | rightEye:" << reChrono.avgCPUTime.getAvgReadable();
        avgLog << "\nGPU time: "<<"leftEye: " << leChrono.avgGPUTime.getAvgReadable() << " | rightEye:" << reChrono.avgGPUTime.getAvgReadable()
        <<" | left&right:" <<AvgCalculator::median(leChrono.avgGPUTime,reChrono.avgGPUTime).getAvgReadable();
        avgLog<<"\nGPU % not measurable:"<<": leftEye:"<<leGPUTimeNotMeasurablePerc<<" | rightEye:"<<reGPUTimeNotMeasurablePerc
        <<" | left&right:"<<leAreGPUTimeNotMeasurablePerc;
        avgLog<<"\nVsync waitT:"<<" start: "<< vsyncWaitTime[0].getAvgReadable()<<" | middle: "<<vsyncWaitTime[1].getAvgReadable()
        <<" | start&middle "<<AvgCalculator::median(vsyncWaitTime[0],vsyncWaitTime[1]).getAvgReadable();
        //avgLog<<"\nDisplay refresh time ms:"<<DISPLAY_REFRESH_TIME/1000.0/1000.0;
        avgLog<<"\n----  -----  ----  ----  ----  ----  ----  ----  --- ---";
        MLOGD<<avgLog.str();
        resetTS();
    }
}

void FBRManager::resetTS() {
    for(int eye=0;eye<2;eye++){
        vsyncWaitTime[0].reset();
    }
    for(int i=0;i<2;i++){
        eyeChrono[i].avgCPUTime.reset();
        eyeChrono[i].avgGPUTime.reset();
        eyeChrono[i].nEyes=0;
        eyeChrono[i].nEyesNotMeasurable=0;
    }
}



