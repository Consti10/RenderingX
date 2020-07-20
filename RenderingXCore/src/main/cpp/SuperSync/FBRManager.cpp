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
    cNanoseconds before,diff=0;

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
            directRender.begin(getViewportForEye(LEFT_EYE,SCREEN_W,SCREEN_H));
            onRenderNewEyeCallback(env,eye!=0);
            directRender.end();
            std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
            glFlush();
            vsyncWaitTime[eye].start();
            if(eye==0){
                waitUntilTimePoint(nextVSYNCMiddle,*fenceSync);
            }else{
                waitUntilTimePoint(nextVSYNC,*fenceSync);
            }
            gpuChrono[eye].nEyes++;
            if(fenceSync->hasAlreadyBeenSatisfied()){
                gpuChrono[eye].lastDelta= fenceSync->getDeltaCreationSatisfiedNS();
                gpuChrono[eye].avgDelta.add(std::chrono::nanoseconds( gpuChrono[eye].lastDelta));
            }else{
                MLOGE<<"Couldnt measure GPU time";
                gpuChrono[eye].nEyesNotMeasurable++;
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

int64_t FBRManager::waitUntilVsyncStart() {
    /*leGPUChrono.nEyes++;
    while(true){
        if(leGPUChrono.fenceSync!= nullptr){
            const bool satisfied=leGPUChrono.fenceSync->wait(0);
            if(satisfied){
                //great ! We can measure the GPU time
                leGPUChrono.lastDelta=leGPUChrono.fenceSync->getDeltaCreationSatisfiedNS();
                leGPUChrono.avgDelta.add(std::chrono::nanoseconds(leGPUChrono.lastDelta));
                leGPUChrono.fenceSync.reset(nullptr);
                //LOGV("leftEye GL: %f",(leGPUChrono.lastDelta/1000)/1000.0);
            }
        }
        int64_t pos=getVsyncRasterizerPosition();
        if(pos<getEyeRefreshTime()){
            //don't forget to delete the sync object if it was not jet signaled. We can't measure the completion time because of the Asynchronousity of glFlush()
            if(leGPUChrono.fenceSync!= nullptr){
                leGPUChrono.fenceSync.reset(nullptr);
                leGPUChrono.nEyesNotMeasurable++;
                //LOGV("Couldn't measure leftEye GPU time");
            }
            //the vsync is currently between 0 and 0.5 (scanning the left eye).
            //The right eye framebuffer part can be safely manipulated for eyeRefreshTime-offset ns
            int64_t offset=pos;
            return offset;
        }
    }*/
}

int64_t FBRManager::waitUntilVsyncMiddle(const std::chrono::steady_clock::time_point& nextVSYNCMiddle) {
    /*reGPUChrono.nEyes++;
    const auto timeLeft1=std::chrono::steady_clock::now()-nextVSYNCMiddle;
    MLOGD<<"time left1: "<<MyTimeHelper::R(timeLeft1);
    const bool satisfied=reGPUChrono.fenceSync->wait(timeLeft1);
    if(satisfied){
        reGPUChrono.lastDelta=reGPUChrono.fenceSync->getDeltaCreationSatisfiedNS();
        reGPUChrono.avgDelta.add(std::chrono::nanoseconds(reGPUChrono.lastDelta));
        //LOGV("rightEye GL: %f",(reGPUChrono.lastDelta/1000)/1000.0);
    }else{
        MLOGE<<"fence sync timed out";
        reGPUChrono.nEyesNotMeasurable++;
    }
    reGPUChrono.fenceSync.reset(nullptr);
    const auto timeLeft2=std::chrono::steady_clock::now()-nextVSYNCMiddle;
    MLOGD<<"time left 2: "<< MyTimeHelper::R(timeLeft2);
    std::this_thread::sleep_for(timeLeft2);
    const auto offset=std::chrono::steady_clock::now()-nextVSYNCMiddle;
    MLOGD<<"offset (overshoot)"<< MyTimeHelper::R(offset);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(offset).count();*/
    /*while(true){
        if(reGPUChrono.fenceSync!= nullptr){
            const bool satisfied=reGPUChrono.fenceSync->wait(0);
            if(satisfied){
                reGPUChrono.lastDelta=reGPUChrono.fenceSync->getDeltaCreationSatisfiedNS();
                reGPUChrono.avgDelta.add(std::chrono::nanoseconds(reGPUChrono.lastDelta));
                reGPUChrono.fenceSync.reset(nullptr);
                //LOGV("rightEye GL: %f",(reGPUChrono.lastDelta/1000)/1000.0);
            }
        }
        int64_t pos=getVsyncRasterizerPosition();
        if(pos>getEyeRefreshTime()){
            //don't forget to delete the sync object if it was not jet signaled. We can't measure the completion time because of the Asynchronousity of glFlush()
            if(reGPUChrono.fenceSync!= nullptr){
                reGPUChrono.fenceSync.reset(nullptr);
                reGPUChrono.nEyesNotMeasurable++;
                //LOGV("Couldn't measure rightEye GPU time");
            }
            //the vsync is currently between 0.5 and 1 (scanning the right eye).
            //The left eye framebuffer part can be safely manipulated for eyeRefreshTime-offset ns
            int64_t offset=pos-getEyeRefreshTime();
            return offset;
        }
    }*/
}


void FBRManager::printLog() {
    const auto now=steady_clock::now();
    if(duration_cast<std::chrono::milliseconds>(now-lastLog).count()>5*1000){//every 5 seconds
        lastLog=now;
        auto& leGPUChrono=gpuChrono[0];
        auto& reGPUChrono=gpuChrono[1];
        const double leGPUTimeAvg=leGPUChrono.avgDelta.getAvg_ms();
        const double reGPUTimeAvg=reGPUChrono.avgDelta.getAvg_ms();
        const double leAreGPUTimeAvg=(leGPUTimeAvg+reGPUTimeAvg)*0.5;
        double leGPUTimeNotMeasurablePerc=0;
        double reGPUTimeNotMeasurablePerc=0;
        double leAreGPUTimeNotMeasurablePerc=0;
        if(leGPUChrono.nEyes>0){
            leGPUTimeNotMeasurablePerc=(leGPUChrono.nEyesNotMeasurable/leGPUChrono.nEyes)*100.0;
        }
        if(reGPUChrono.nEyes>0){
            reGPUTimeNotMeasurablePerc=(reGPUChrono.nEyesNotMeasurable/reGPUChrono.nEyes)*100.0;
        }
        leAreGPUTimeNotMeasurablePerc=(leGPUTimeNotMeasurablePerc+reGPUTimeNotMeasurablePerc)*0.5;
        std::ostringstream avgLog;
        avgLog<<"------------------------FBRManager Averages------------------------";
        avgLog<<"\nGPU time:"<<": leftEye:"<<leGPUTimeAvg<<" | rightEye:"<<reGPUTimeAvg<<" | left&right:"<<leAreGPUTimeAvg;
        avgLog<<"\nGPU % not measurable:"<<": leftEye:"<<leGPUTimeNotMeasurablePerc<<" | rightEye:"<<reGPUTimeNotMeasurablePerc<<" | left&right:"<<leAreGPUTimeNotMeasurablePerc;
        avgLog<<"\nVsync waitT:"<<" start:"<< vsyncWaitTime[0].getAvgUS()/1000.0<<" | middle:"<<vsyncWaitTime[1].getAvgMS()
        <<" | start&middle"<<(vsyncWaitTime[0].getAvgMS()+vsyncWaitTime[1].getAvgMS())/2.0;
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
        gpuChrono[i].avgDelta.reset();
        gpuChrono[i].nEyes=0;
        gpuChrono[i].nEyesNotMeasurable=0;
    }
}



