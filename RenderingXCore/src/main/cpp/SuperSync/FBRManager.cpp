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


void FBRManager::enterDirectRenderingLoop(JNIEnv* env) {
    shouldRender= true;
    cNanoseconds before,diff=0;
    while(shouldRender){
        for(int eye=0;eye<2;eye++){
            if(!shouldRender){
                break;
            }
            vsyncWaitTime[eye].start();
            if(eye==0){
                waitUntilVsyncStart();
            }else{
                waitUntilVsyncMiddle();
            }
            vsyncWaitTime[eye].stop();
            before=getSystemTimeNS();
            //render new eye
            onRenderNewEyeCallback(env,eye,0);
            diff=getSystemTimeNS()-before;
            if(diff>=getDisplayRefreshTime()){
                MLOGE<<"WARNING: rendering a eye took longer than displayRefreshTime ! Error. Time: "<<(diff/1000/1000);
            }
        }
        printLog();
    }
}

void FBRManager::requestExitSuperSyncLoop() {
    shouldRender= false;
}

int64_t FBRManager::waitUntilVsyncStart() {
    leGPUChrono.nEyes++;
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
    }
}

int64_t FBRManager::waitUntilVsyncMiddle() {
    reGPUChrono.nEyes++;
    const auto timeLeft=getEyeRefreshTime()-getVsyncRasterizerPosition();
    MLOGD<<"time left1: "<< MyTimeHelper::ReadableNS(timeLeft);
    if(timeLeft<0 || timeLeft>=getDisplayRefreshTime()){
        MLOGE<<"Time left ?"<<timeLeft;
    }
    const bool satisfied=reGPUChrono.fenceSync->wait(timeLeft);
    if(satisfied){
        reGPUChrono.lastDelta=reGPUChrono.fenceSync->getDeltaCreationSatisfiedNS();
        reGPUChrono.avgDelta.add(std::chrono::nanoseconds(reGPUChrono.lastDelta));
        //LOGV("rightEye GL: %f",(reGPUChrono.lastDelta/1000)/1000.0);
    }else{
        MLOGE<<"fence sync timed out";
        reGPUChrono.nEyesNotMeasurable++;
    }
    reGPUChrono.fenceSync.reset(nullptr);
    const auto timeLeft2=getEyeRefreshTime()-getVsyncRasterizerPosition();
    MLOGD<<"time left 2: "<< MyTimeHelper::ReadableNS(timeLeft2);
    if(timeLeft2>1000){
        std::this_thread::sleep_for(std::chrono::nanoseconds(timeLeft2));
    }
    const auto offset=getEyeRefreshTime()-getVsyncRasterizerPosition();
    MLOGD<<"offset (overshoot)"<< MyTimeHelper::ReadableNS(offset);
    return offset;
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

void FBRManager::startDirectRendering(bool leftEye, int viewPortW, int viewPortH) {
    directRender.begin(leftEye,viewPortW,viewPortH);
}

void FBRManager::stopDirectRendering(bool whichEye) {
    directRender.end(whichEye);
    if(EGL_KHR_Reusable_Sync_Available){
        if(whichEye){
            leGPUChrono.fenceSync=std::make_unique<FenceSync>();
        }else{
            reGPUChrono.fenceSync=std::make_unique<FenceSync>();
        }
    }
    glFlush();
}

void FBRManager::printLog() {
    const auto now=steady_clock::now();
    if(duration_cast<std::chrono::milliseconds>(now-lastLog).count()>5*1000){//every 5 seconds
        lastLog=now;
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
    leGPUChrono.avgDelta.reset();
    leGPUChrono.nEyes=0;
    leGPUChrono.nEyesNotMeasurable=0;
    //
    reGPUChrono.avgDelta.reset();
    reGPUChrono.nEyes=0;
    reGPUChrono.nEyesNotMeasurable=0;
}


