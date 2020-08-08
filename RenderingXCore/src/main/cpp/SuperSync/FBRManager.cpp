//
// Created by Constantin on 23.11.2016.
//
#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sstream>
#include "FBRManager.h"
#include "Extensions.h"
#include <AndroidLogger.hpp>
#include <utility>

//#define RELEASE
constexpr auto MS_TO_NS=1000*1000;

using namespace std::chrono;

static std::chrono::steady_clock::duration ThisThreadSleepUntil(std::chrono::steady_clock::time_point tp){
    std::this_thread::sleep_until(tp);
    const auto now=std::chrono::steady_clock::now();
    return now-tp;
}

FBRManager::FBRManager(VSYNC* vsync,RENDER_NEW_EYE_CALLBACK onRenderNewEyeCallback,SurfaceTextureUpdate& surfaceTextureUpdate):
vsync(*vsync),
surfaceTextureUpdate(surfaceTextureUpdate),
        onRenderNewEyeCallback(std::move(onRenderNewEyeCallback))
{
    lastLog=steady_clock::now();
    resetTS();
}

void FBRManager::drawLeftAndRightEye(JNIEnv* env,int SCREEN_W,int SCREEN_H) {
    ATrace_beginSection("SurfaceTexture::update");
    if(const auto delay=surfaceTextureUpdate.waitUntilFrameAvailable(env,std::chrono::steady_clock::now()+std::chrono::seconds(1))){
        MLOGD<<"Delay until opengl is "<<MyTimeHelper::R(*delay);
    }
    const auto VSYNCPositionNormalized=vsync.getVsyncRasterizerPositionNormalized();
    MLOGD<<"Got new video Frame. Current VSYNC position is: "<<VSYNCPositionNormalized;
    const auto lastVSYNC=vsync.getLatestVSYNC().base;
    /*CLOCK::time_point startRenderingLeftEye;
    CLOCK::time_point startRenderingRightEye;
    if(VSYNCPositionNormalized>0.1f && VSYNCPositionNormalized<0.5f){
        // Not enough time left to render the
        startRenderingLeftEye=lastVSYNC+getEyeRefreshTime();
        startRenderingRightEye=lastVSYNC+getDisplayRefreshTime();
    }else if(VSYNCPositionNormalized){

    }*/

    if(VSYNCPositionNormalized<0.5){
        // Render right eye, We are scan line racing
        bool isLeftEye=false;
        const CLOCK::time_point startRenderingLeftEye=lastVSYNC+vsync.getEyeRefreshTime();
        directRender.begin(getViewportForEye(isLeftEye,SCREEN_W,SCREEN_H));
        onRenderNewEyeCallback(env,isLeftEye);
        directRender.end();
        //
        std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
        waitUntilTimePoint(startRenderingLeftEye,*fenceSync);
        isLeftEye=true;
        directRender.begin(getViewportForEye(isLeftEye,SCREEN_W,SCREEN_H));
        onRenderNewEyeCallback(env,isLeftEye);
        directRender.end();
        glFlush();
    }else{ // VSYNC is between [0.5,1.0]
        // Render left eye
       bool isLeftEye=true;
        CLOCK::time_point startRenderingRightEye=lastVSYNC+vsync.getDisplayRefreshTime();
        directRender.begin(getViewportForEye(isLeftEye,SCREEN_W,SCREEN_H));
        onRenderNewEyeCallback(env,isLeftEye);
        directRender.end();
        //
        std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
        waitUntilTimePoint(startRenderingRightEye,*fenceSync);
        isLeftEye=false;
        directRender.begin(getViewportForEye(isLeftEye,SCREEN_W,SCREEN_H));
        onRenderNewEyeCallback(env,isLeftEye);
        directRender.end();
        glFlush();
    }
    ATrace_endSection();
}


void FBRManager::warpEyesToFrontBufferSynchronized(JNIEnv* env, int SCREEN_W, int SCREEN_H) {
    if(endLastFunctionCall!=CLOCK::time_point{}){
        const auto deltaBetweenFunctionCalls=CLOCK::now()-endLastFunctionCall;
        if(deltaBetweenFunctionCalls>300us){ //1/10 of a ms
            //MLOGE<<"Stayed too long:"<<MyTimeHelper::R(deltaBetweenFunctionCalls);
        }
    }
    const auto latestVSYNC=vsync.getLatestVSYNC();
    const auto nextVSYNCMiddle=latestVSYNC.base+vsync.getEyeRefreshTime()+std::chrono::milliseconds(0);
    const auto nextVSYNC=latestVSYNC.base+vsync.getDisplayRefreshTime()+std::chrono::milliseconds(0);
    if(lastRenderedFrame.count+1!=latestVSYNC.count){
        MLOGE<<"Probably missed VSYNC "<<lastRenderedFrame.count<<" "<<latestVSYNC.count<<" "<<vsync.getVsyncRasterizerPositionNormalized()<<" "<<MyTimeHelper::R(CLOCK::now()-latestVSYNC.base);
    }
    lastRenderedFrame=latestVSYNC;
    //
    // wait until nextVSYNCMiddle
    // Render left eye
    // wait until nextVSYNC
    // Render right eye
    // -> latestVSYNC becomes nextVSYNC
    //MLOGD<<"VSYNC rasterizer position "<<getVsyncRasterizerPositionNormalized();

    // Render right eye (1) first, then left eye(0)
    for(int eye=1;eye>=0;eye--){
        const bool isLeftEye=eye==0;
        const auto nextEvent=eye==1 ? nextVSYNCMiddle : nextVSYNC;
        if(CLOCK::now()>nextEvent){
            MLOGE<<"Event already passed";
            continue;
        }
        //render new eye (right eye first)
        ATrace_beginSection(eye==0 ? "FBRManager::renderLeftEye" : "FBRManager::renderRightEye");
        eyeChrono[eye].avgCPUTime.start();
        TimerQuery timerQuery;
        timerQuery.begin();
        ATrace_beginSection("SurfaceTexture::update");
        avgCPUTimeUpdateSurfaceTexture.start();
        surfaceTextureUpdate.updateAndCheck(env);
        avgCPUTimeUpdateSurfaceTexture.stop();
        ATrace_endSection();
        ATrace_beginSection("DirectRendering::begin");
        directRender.begin(getViewportForEye(isLeftEye,SCREEN_W,SCREEN_H));
        ATrace_endSection();
        ATrace_beginSection("renderNewEyeCallback");
        onRenderNewEyeCallback(env,isLeftEye);
        ATrace_endSection();
        ATrace_beginSection("DirectRendering::end");
        directRender.end();
        eyeChrono[eye].avgCPUTime.stop();
        ATrace_endSection();
        ATrace_beginSection("Wait for GPU completion");
        std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
        glFlush();
        timerQuery.end();
        vsyncWaitTime[eye].start();
        waitUntilTimePoint(nextEvent,*fenceSync);
        ATrace_endSection();
        //timerQuery.print();
        //MLOGD<<"Time from fence "<<MyTimeHelper::R(fenceSync->getDeltaCreationSatisfied());

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
        //MLOGD<<"VSYNC pos "<<getVsyncRasterizerPositionNormalized();
    }
    printLog();
    endLastFunctionCall=CLOCK::now();
}


VSYNC::CLOCK::duration  FBRManager::waitUntilTimePoint(const std::chrono::steady_clock::time_point& timePoint,FenceSync& fenceSync) {
    const auto timeLeft=timePoint-CLOCK::now();
    if(timeLeft<=0ns){
        MLOGE<<"Time point already elapsed(wait)";
        const auto overshoot=CLOCK::now()-timePoint;
        ATrace_endSection();
        return overshoot;
    }
    fenceSync.wait(timeLeft);
    ATrace_endSection();
    ATrace_beginSection("Sleep");
    std::this_thread::sleep_until(timePoint);
    const auto overshoot=CLOCK::now()-timePoint;
    ATrace_endSection();
    return overshoot;
}


void FBRManager::printLog() {
    const auto now=steady_clock::now();
    if(now-lastLog>std::chrono::seconds(3)){//every 5 seconds
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
        avgLog<<"\n SurfaceTexture update "<<avgCPUTimeUpdateSurfaceTexture.getAvgReadable();
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



