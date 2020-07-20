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
    while(shouldRender){
        const auto latestVSYNC=getLatestVSYNC();
        const auto nextVSYNCMiddle=latestVSYNC+getEyeRefreshTime();
        const auto nextVSYNC=latestVSYNC+getDisplayRefreshTime();
        //MLOGD<<"latestVSYNC"<<MyTimeHelper::R(CLOCK::now()-latestVSYNC)<<" nextVSYNCMiddle "<<MyTimeHelper::R(CLOCK::now()-nextVSYNCMiddle)<<" nextVSYNC "<<MyTimeHelper::R(CLOCK::now()-nextVSYNC);
        //
        // wait until nextVSYNCMiddle
        // Render left eye
        // wait until nextVSYNC
        // Render right eye
        // -> latestVSYNC becomes nextVSYNC
        //MLOGD<<"VSYNC rasterizer position "<<getVsyncRasterizerPositionNormalized();

        for(int eye=0;eye<2;eye++){
            if(!shouldRender){
                break;
            }
            const bool LEFT_EYE=eye!=0;
            //render new eye
            eyeChrono[eye].avgCPUTime.start();
            directRender.begin(getViewportForEye(LEFT_EYE,SCREEN_W,SCREEN_H));
            onRenderNewEyeCallback(env,eye!=0);
            directRender.end();
            eyeChrono[eye].avgCPUTime.stop();
            std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
            glFlush();
            vsyncWaitTime[eye].start();
            if(eye==0){
                waitUntilTimePoint(nextVSYNCMiddle,*fenceSync);
            }else{
                waitUntilTimePoint(nextVSYNC,*fenceSync);
            }
            eyeChrono[eye].nEyes++;
            if(fenceSync->hasAlreadyBeenSatisfied()){
                eyeChrono[eye].avgGPUTime.add(fenceSync->getDeltaCreationSatisfied());
            }else{
                MLOGE<<"Couldnt measure GPU time";
                eyeChrono[eye].nEyesNotMeasurable++;
            }
            fenceSync.reset(nullptr);
            vsyncWaitTime[eye].stop();
        }
        printLog();
    }
}

void FBRManager::requestExitSuperSyncLoop() {
    shouldRender= false;
}

void FBRManager::waitUntilTimePoint(const std::chrono::steady_clock::time_point& timePoint,FenceSync& fenceSync) {
    const auto timeLeft1=std::chrono::steady_clock::now()-timePoint;
    //MLOGD<<"time left1: "<<MyTimeHelper::R(timeLeft1);
    const bool satisfied= fenceSync.wait(timeLeft1);
    const auto timeLeft2=std::chrono::steady_clock::now()-timePoint;
    //MLOGD<<"time left 2: "<< MyTimeHelper::R(timeLeft2);
    std::this_thread::sleep_until(timePoint);
    const auto offset=std::chrono::steady_clock::now()-timePoint;
    //MLOGD<<"offset (overshoot)"<< MyTimeHelper::R(offset);
    //return std::chrono::duration_cast<std::chrono::nanoseconds>(offset).count();
}


void FBRManager::printLog() {
    const auto now=steady_clock::now();
    if(duration_cast<std::chrono::milliseconds>(now-lastLog).count()>5*1000){//every 5 seconds
        lastLog=now;
        auto& leChrono=eyeChrono[0];
        auto& reChrono=eyeChrono[1];
        const auto leAreGPUTimeAvg= (leChrono.avgGPUTime.getAvg() + reChrono.avgGPUTime.getAvg()) / 2;
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
        avgLog << "\nCPU Time  : " << "leftEye:" << leChrono.avgCPUTime.getAvg_ms() << " | rightEye:" << reChrono.avgCPUTime.getAvg_ms();
        avgLog << "\nGPU time: " << "leftEye:" << leChrono.avgGPUTime.getAvgReadable() << " | rightEye:" << reChrono.avgGPUTime.getAvgReadable() << " | left&right:" << MyTimeHelper::R(leAreGPUTimeAvg);
        avgLog<<"\nGPU % not measurable:"<<": leftEye:"<<leGPUTimeNotMeasurablePerc<<" | rightEye:"<<reGPUTimeNotMeasurablePerc<<" | left&right:"<<leAreGPUTimeNotMeasurablePerc;
        avgLog<<"\nVsync waitT:"<<" start:"<< vsyncWaitTime[0].getAvg_ms()<<" | middle:"<<vsyncWaitTime[1].getAvg_ms()
        <<" | start&middle"<<(vsyncWaitTime[0].getAvg_ms()+vsyncWaitTime[1].getAvg_ms())/2.0;
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
        eyeChrono[i].avgGPUTime.reset();
        eyeChrono[i].nEyes=0;
        eyeChrono[i].nEyesNotMeasurable=0;
    }
}



