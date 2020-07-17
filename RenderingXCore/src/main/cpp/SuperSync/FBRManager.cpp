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
        directRenderingMode(qcomTiledRenderingAvailable ? QCOM_TILED_RENDERING : MALI_SoylentGraham),
        onRenderNewEyeCallback(onRenderNewEyeCallback),
        onErrorCallback(onErrorCallback),
        vsyncStartWT("VSYNC start wait time"),
        vsyncMiddleWT("VSYNC middle wait time")
{
    KHR_fence_sync::init();
    Extensions::initOtherExtensions();
    lastLog=steady_clock::now();
    resetTS();
}


void FBRManager::enterDirectRenderingLoop(JNIEnv* env) {
    shouldRender= true;
    cNanoseconds before,diff=0;
    while(shouldRender){
        if(diff>=getDisplayRefreshTime()){
            MLOGE<<"WARNING: rendering a eye took longer than displayRefreshTime ! Error. Time: "<<(diff/1000/1000);
        }
        vsyncStartWT.start();
        int64_t leOffset=waitUntilVsyncStart();
        vsyncStartWT.stop();
        before=getSystemTimeNS();
        //render new eye
        onRenderNewEyeCallback(env,RIGHT_EYE,leOffset);
        diff=getSystemTimeNS()-before;
        if(!shouldRender){
            break;
        }
        if(diff>=getDisplayRefreshTime()){
            MLOGE<<"WARNING: rendering a eye took longer than displayRefreshTime ! Error. Time: "<<(diff/1000/1000);
        }
        vsyncMiddleWT.start();
        int64_t reOffset=waitUntilVsyncMiddle();
        vsyncMiddleWT.stop();
        before=getSystemTimeNS();
        //render new eye
        onRenderNewEyeCallback(env,LEFT_EYE,reOffset);
        diff=getSystemTimeNS()-before;
        printLog();
    }
    //do not forget to clean up for a more pleasant view
    switch(directRenderingMode){
        case QCOM_TILED_RENDERING:QCOM_tiled_rendering::EndTilingQCOM();break;
        default:
            break;
    }
}

void FBRManager::requestExitSuperSyncLoop() {
    shouldRender= false;
}

int64_t FBRManager::waitUntilVsyncStart() {
    leGPUChrono.nEyes++;
    while(true){
        if(leGPUChrono.eglSyncKHR!= nullptr){
            const EGLint wait = KHR_fence_sync::eglClientWaitSyncKHR_(eglGetCurrentDisplay(), leGPUChrono.eglSyncKHR,
                                                                      EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0 );
            if(wait==EGL_CONDITION_SATISFIED_KHR){
                //great ! We can measure the GPU time
                leGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
                KHR_fence_sync::eglDestroySyncKHR_(eglGetCurrentDisplay(), leGPUChrono.eglSyncKHR );
                leGPUChrono.eglSyncKHR= nullptr;
                leGPUChrono.lastDelta=leGPUChrono.eglSyncSatisfiedT-leGPUChrono.eglSyncCreationT;
                leGPUChrono.deltaSumUS+=leGPUChrono.lastDelta/1000;
                leGPUChrono.deltaSumUsC++;
                //LOGV("leftEye GL: %f",(leGPUChrono.lastDelta/1000)/1000.0);
            }
        }
        int64_t pos=getVsyncRasterizerPosition();
        if(pos<getEyeRefreshTime()){
            //don't forget to delete the sync object if it was not jet signaled. We can't measure the completion time because of the Asynchronousity of glFlush()
            if(leGPUChrono.eglSyncKHR!= nullptr){
                KHR_fence_sync::eglDestroySyncKHR_(eglGetCurrentDisplay(), leGPUChrono.eglSyncKHR );
                leGPUChrono.eglSyncKHR= nullptr;
                leGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
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
    while(true){
        if(reGPUChrono.eglSyncKHR!= nullptr){
            const EGLint wait = KHR_fence_sync::eglClientWaitSyncKHR_(eglGetCurrentDisplay(), reGPUChrono.eglSyncKHR,
                                                                      EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0 );
            if(wait==EGL_CONDITION_SATISFIED_KHR){
                //great ! We can measure the GPU time
                reGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
                KHR_fence_sync::eglDestroySyncKHR_(eglGetCurrentDisplay(), reGPUChrono.eglSyncKHR );
                reGPUChrono.eglSyncKHR= nullptr;
                reGPUChrono.lastDelta=reGPUChrono.eglSyncSatisfiedT-reGPUChrono.eglSyncCreationT;
                reGPUChrono.deltaSumUS+=reGPUChrono.lastDelta/1000;
                reGPUChrono.deltaSumUsC++;
                //LOGV("rightEye GL: %f",(reGPUChrono.lastDelta/1000)/1000.0);
            }
        }
        int64_t pos=getVsyncRasterizerPosition();
        if(pos>getEyeRefreshTime()){
            //don't forget to delete the sync object if it was not jet signaled. We can't measure the completion time because of the Asynchronousity of glFlush()
            if(reGPUChrono.eglSyncKHR!= nullptr){
                KHR_fence_sync::eglDestroySyncKHR_(eglGetCurrentDisplay(), reGPUChrono.eglSyncKHR );
                reGPUChrono.eglSyncKHR= nullptr;
                reGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
                reGPUChrono.nEyesNotMeasurable++;
                //LOGV("Couldn't measure rightEye GPU time");
            }
            //the vsync is currently between 0.5 and 1 (scanning the right eye).
            //The left eye framebuffer part can be safely manipulated for eyeRefreshTime-offset ns
            int64_t offset=pos-getEyeRefreshTime();
            return offset;
        }
    }
}

void FBRManager::startDirectRendering(bool leftEye, int viewPortW, int viewPortH) {
    directRender.begin(leftEye,viewPortW,viewPortH);
}

void FBRManager::stopDirectRendering(bool whichEye) {
    directRender.end(whichEye);
    if(EGL_KHR_Reusable_Sync_Available){
        if(whichEye){
            leGPUChrono.eglSyncCreationT=getSystemTimeNS();
            leGPUChrono.eglSyncKHR=KHR_fence_sync::eglCreateSyncKHR_(eglGetCurrentDisplay(), EGL_SYNC_FENCE_KHR,
                                                                     nullptr );
        }else{
            reGPUChrono.eglSyncCreationT=getSystemTimeNS();
            reGPUChrono.eglSyncKHR=KHR_fence_sync::eglCreateSyncKHR_(eglGetCurrentDisplay(), EGL_SYNC_FENCE_KHR,
                                                                     nullptr );
        }
    }
    glFlush();
}

void FBRManager::printLog() {
    const auto now=steady_clock::now();
    if(duration_cast<std::chrono::milliseconds>(now-lastLog).count()>5*1000){//every 5 seconds
        lastLog=now;
        double leGPUTimeAvg=0;
        double reGPUTimeAvg=0;
        double leAreGPUTimeAvg;
        if(leGPUChrono.deltaSumUsC>0){
            leGPUTimeAvg=(leGPUChrono.deltaSumUS/leGPUChrono.deltaSumUsC)/1000.0;
        }
        if(reGPUChrono.deltaSumUsC>0){
            reGPUTimeAvg=(reGPUChrono.deltaSumUS/reGPUChrono.deltaSumUsC)/1000.0;
        }
        leAreGPUTimeAvg=(leGPUTimeAvg+reGPUTimeAvg)*0.5;
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
        double advanceMS=0;
        if(leAreGPUTimeNotMeasurablePerc>50){
            //more than half of all frames took too long to render, increase latency by 2
            advanceMS=2;
        }
        if(leAreGPUTimeNotMeasurablePerc<1){
            //(almost) no frames failed
            advanceMS=((vsyncStartWT.getAvgUS()+ vsyncMiddleWT.getAvgUS())/2.0/1000.0)-leAreGPUTimeAvg;
            //4ms for "safety" (because front and back porch usw) DAFUQ ?!
            advanceMS-=4;
            if(advanceMS>0){
                advanceMS*=-1;
            }else{
                advanceMS=0;
            }
        }
        std::ostringstream avgLog;
        avgLog<<"------------------------FBRManager Averages------------------------";
        avgLog<<"\nGPU time:"<<": leftEye:"<<leGPUTimeAvg<<" | rightEye:"<<reGPUTimeAvg<<" | left&right:"<<leAreGPUTimeAvg;
        avgLog<<"\nGPU % not measurable:"<<": leftEye:"<<leGPUTimeNotMeasurablePerc<<" | rightEye:"<<reGPUTimeNotMeasurablePerc<<" | left&right:"<<leAreGPUTimeNotMeasurablePerc;
        avgLog<<"\nVsync waitT:"<<" start:"<< vsyncStartWT.getAvgUS()/1000.0<<" | middle:"<<
                                                                                       vsyncMiddleWT.getAvgUS()/1000.0<<" | start&middle"<<(vsyncStartWT.getAvgUS()+
                                                                                                                                            vsyncMiddleWT.getAvgUS())/2.0/1000.0;
        avgLog<<"\nVsync advance ms:"<< advanceMS;
        //avgLog<<"\nDisplay refresh time ms:"<<DISPLAY_REFRESH_TIME/1000.0/1000.0;
        avgLog<<"\n----  -----  ----  ----  ----  ----  ----  ----  --- ---";
        MLOGD<<avgLog.str();
        resetTS();
    }
}

void FBRManager::resetTS() {
    vsyncStartWT.reset();
    vsyncMiddleWT.reset();
    leGPUChrono.deltaSumUsC=0;
    leGPUChrono.deltaSumUS=0;
    leGPUChrono.nEyes=0;
    leGPUChrono.nEyesNotMeasurable=0;
    //
    reGPUChrono.deltaSumUsC=0;
    reGPUChrono.deltaSumUS=0;
    reGPUChrono.nEyes=0;
    reGPUChrono.nEyesNotMeasurable=0;
}


