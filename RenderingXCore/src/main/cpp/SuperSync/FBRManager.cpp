//
// Created by Constantin on 23.11.2016.
//
#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sstream>
#include "FBRManager.h"
#include "Extensions.hpp"

constexpr auto TAG="FBRManager";
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)

//#define RELEASE
constexpr auto MS_TO_NS=1000*1000;

using namespace std::chrono;

FBRManager::FBRManager(bool qcomTiledRenderingAvailable,bool reusableSyncAvailable,bool useVSYNC_CALLBACK_ADVANCE_NS,const RENDER_NEW_EYE_CALLBACK onRenderNewEyeCallback,const ERROR_CALLBACK onErrorCallback):
        EGL_KHR_Reusable_Sync_Available(reusableSyncAvailable),
        directRenderingMode(qcomTiledRenderingAvailable ? QCOM_TILED_RENDERING : MALI_SoylentGraham),
        useVSYNC_CALLBACK_ADVANCE_NS(useVSYNC_CALLBACK_ADVANCE_NS),
        onRenderNewEyeCallback(onRenderNewEyeCallback),
        onErrorCallback(onErrorCallback),
        vsyncStartWT("VSYNC start wait time"),
        vsyncMiddleWT("VSYNC middle wait time")
{
    lastRegisteredVSYNC=0;
    previousVSYNC=0;
    displayRefreshTimeSum=0;
    displayRefreshTimeC=0;
    initOtherExtensions();
    switch(directRenderingMode){
        case QCOM_TILED_RENDERING:initQCOMTiling();break;
        default:
            break;
    }
    lastLog=steady_clock::now();
    resetTS();
}


void FBRManager::enterDirectRenderingLoop(JNIEnv* env) {
    shouldRender= true;
    cNanoseconds before,diff=0;
    while(shouldRender){
#ifndef MY_RELEASE
        if(diff>=DISPLAY_REFRESH_TIME){
            LOGD("WARNING: rendering a eye took longer than displayRefreshTime ! Error. Time: %d",(int)(diff/1000/1000));
        }
#endif
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
#ifndef MY_RELEASE
        if(diff>=DISPLAY_REFRESH_TIME){
            LOGD("WARNING: rendering a eye took longer than displayRefreshTime ! Error. Time: %d",(int)(diff/1000/1000));
        }
#endif
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
        case QCOM_TILED_RENDERING:glEndTilingQCOM();break;
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
            const EGLint wait = eglClientWaitSyncKHR_( eglGetCurrentDisplay(), leGPUChrono.eglSyncKHR,
                                                       EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0 );
            if(wait==EGL_CONDITION_SATISFIED_KHR){
                //great ! We can measure the GPU time
                leGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
                eglDestroySyncKHR_( eglGetCurrentDisplay(), leGPUChrono.eglSyncKHR );
                leGPUChrono.eglSyncKHR= nullptr;
                leGPUChrono.lastDelta=leGPUChrono.eglSyncSatisfiedT-leGPUChrono.eglSyncCreationT;
                leGPUChrono.deltaSumUS+=leGPUChrono.lastDelta/1000;
                leGPUChrono.deltaSumUsC++;
                //LOGV("leftEye GL: %f",(leGPUChrono.lastDelta/1000)/1000.0);
            }
        }
        int64_t pos=getVsyncRasterizerPosition();
        if(pos<EYE_REFRESH_TIME){
            //don't forget to delete the sync object if it was not jet signaled. We can't measure the completion time because of the Asynchronousity of glFlush()
            if(leGPUChrono.eglSyncKHR!= nullptr){
                eglDestroySyncKHR_( eglGetCurrentDisplay(), leGPUChrono.eglSyncKHR );
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
            const EGLint wait = eglClientWaitSyncKHR_( eglGetCurrentDisplay(), reGPUChrono.eglSyncKHR,
                                                       EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0 );
            if(wait==EGL_CONDITION_SATISFIED_KHR){
                //great ! We can measure the GPU time
                reGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
                eglDestroySyncKHR_( eglGetCurrentDisplay(), reGPUChrono.eglSyncKHR );
                reGPUChrono.eglSyncKHR= nullptr;
                reGPUChrono.lastDelta=reGPUChrono.eglSyncSatisfiedT-reGPUChrono.eglSyncCreationT;
                reGPUChrono.deltaSumUS+=reGPUChrono.lastDelta/1000;
                reGPUChrono.deltaSumUsC++;
                //LOGV("rightEye GL: %f",(reGPUChrono.lastDelta/1000)/1000.0);
            }
        }
        int64_t pos=getVsyncRasterizerPosition();
        if(pos>EYE_REFRESH_TIME){
            //don't forget to delete the sync object if it was not jet signaled. We can't measure the completion time because of the Asynchronousity of glFlush()
            if(reGPUChrono.eglSyncKHR!= nullptr){
                eglDestroySyncKHR_( eglGetCurrentDisplay(), reGPUChrono.eglSyncKHR );
                reGPUChrono.eglSyncKHR= nullptr;
                reGPUChrono.eglSyncSatisfiedT=getSystemTimeNS();
                reGPUChrono.nEyesNotMeasurable++;
                //LOGV("Couldn't measure rightEye GPU time");
            }
            //the vsync is currently between 0.5 and 1 (scanning the right eye).
            //The left eye framebuffer part can be safely manipulated for eyeRefreshTime-offset ns
            int64_t offset=pos-EYE_REFRESH_TIME;
            return offset;
        }
    }
}

int64_t FBRManager::getVsyncBaseNS() {
    int64_t lVSYNC=lastRegisteredVSYNC;
    int64_t timestamp=getSystemTimeNS();
    int64_t diff=timestamp-lVSYNC;
    if(diff>DISPLAY_REFRESH_TIME){
        int64_t factor=diff/DISPLAY_REFRESH_TIME;
        lVSYNC+=factor*DISPLAY_REFRESH_TIME;
    }
    return lVSYNC;
}

int64_t FBRManager::getVsyncRasterizerPosition() {
    int64_t lVsync=getVsyncBaseNS();
    int64_t position=getSystemTimeNS()-lVsync;
    while (position>=DISPLAY_REFRESH_TIME){
        position-=DISPLAY_REFRESH_TIME;
    }
    return position;
}

void FBRManager::setLastVSYNC(int64_t lastVsync) {
    int64_t tmp=lastVsync-VSYNC_CALLBACK_ADVANCE_NS;
    //int c=0;
    while(getSystemTimeNS()<tmp){
        //c++;
        //LOGV("HI %d",c);
        tmp-=DISPLAY_REFRESH_TIME;
    }
    lastRegisteredVSYNC=tmp;
    //Stop as soon we have 600 samples (that should be more than enough)
    if(displayRefreshTimeC<600){
        int64_t delta=lastVsync-previousVSYNC;
        if(delta>0){
            //Assumption: There are only displays on the Market with refresh Rates that differ from 16.666ms +-1.0ms
            //This is needed because i am not sure if the vsync callbacks always get executed in the right order
            //so delta might be 32ms. In this case delta is not the display refresh time
            const int64_t minDisplayRR=16666666-1000000;
            const int64_t maxDisplayRR=16666666+1000000;
            if(delta>minDisplayRR&&delta<maxDisplayRR){
                displayRefreshTimeSum+=delta;
                displayRefreshTimeC++;
                if(displayRefreshTimeC>120){
                    //we can start using the average Value for "displayRefreshTime" when we have roughly 120 samples
                    DISPLAY_REFRESH_TIME=displayRefreshTimeSum/displayRefreshTimeC;
                    EYE_REFRESH_TIME=displayRefreshTimeSum/(displayRefreshTimeC*2);
                    //float f1=displayRefreshTime/1000.0f/1000.0f;
                    //LOGV("Time between frames:%f",f1);
                    //LOGD("Display refresh time %d",(int)DISPLAY_REFRESH_TIME);
                }
            }
        }
        previousVSYNC=lastVsync;
    } //else We have at least 600 samples. This is enough.
    //LOGV("Hi VSYNC");
}

void FBRManager::startDirectRendering(bool leftEye, int viewPortW, int viewPortH) {
    int x,y,w,h;
    if(leftEye){
        x=0;
        y=0;
        w=viewPortW;
        h=viewPortH;
    }else{
        x=viewPortW;
        y=0;
        w=viewPortW;
        h=viewPortH;
    }
    //NOTE: glClear has to be called from the application, depending on the GPU time (I had to differentiate because of the updateTexImage2D)
    switch (directRenderingMode){
        case QCOM_TILED_RENDERING:{
            glStartTilingQCOM(x,y,w,h);
            glScissor(x,y,w,h); //glStartTiling should be enough. But just for safety set the scissor rect, too
            break;
        }
        case MALI_SoylentGraham:{
            const GLenum attachmentsSG[3] = { GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT};
            glInvalidateFramebuffer_( GL_FRAMEBUFFER, 3, attachmentsSG );
            glScissor( x, y, w, h);
            break;
        }
        case MALI_Consti1:{
            const GLenum attachments[3] = { GL_COLOR_EXT,GL_DEPTH_EXT,GL_STENCIL_EXT};
            glInvalidateFramebuffer_( GL_FRAMEBUFFER, 3, attachments);
            glScissor(x,y,w,h);
            break;
        }
        default:
            break;
    }
    glViewport(x,y,w,h);
}

void FBRManager::stopDirectRendering(bool whichEye) {
    switch (directRenderingMode){
        case QCOM_TILED_RENDERING:{
            glEndTilingQCOM();
            break;
        }
        case MALI_SoylentGraham:{
            const GLenum attachmentsSG[2] = { GL_DEPTH_EXT, GL_STENCIL_EXT};
            glInvalidateFramebuffer_( GL_FRAMEBUFFER, 2, attachmentsSG );
            break;
        }
        case MALI_Consti1:{
            const GLenum attachmentsSG[2] = { GL_DEPTH_EXT, GL_STENCIL_EXT};
            glInvalidateFramebuffer_( GL_FRAMEBUFFER, 2, attachmentsSG );
            break;
        }
        default:
            break;
    }
    if(EGL_KHR_Reusable_Sync_Available){
        if(whichEye){
            leGPUChrono.eglSyncCreationT=getSystemTimeNS();
            leGPUChrono.eglSyncKHR=eglCreateSyncKHR_( eglGetCurrentDisplay(), EGL_SYNC_FENCE_KHR,
                                                      nullptr );
        }else{
            reGPUChrono.eglSyncCreationT=getSystemTimeNS();
            reGPUChrono.eglSyncKHR=eglCreateSyncKHR_( eglGetCurrentDisplay(), EGL_SYNC_FENCE_KHR,
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
        if(useVSYNC_CALLBACK_ADVANCE_NS){
            VSYNC_CALLBACK_ADVANCE_NS=(int64_t)(MS_TO_NS*advanceMS);
        }
#ifndef MY_RELEASE
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
        LOGD("%s",avgLog.str().c_str());
#endif
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


cNanoseconds FBRManager::getSystemTimeNS() {
    const auto time=system_clock::now().time_since_epoch();
    return (cNanoseconds)duration_cast<nanoseconds>(time).count();
}


