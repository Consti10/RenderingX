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
#include <NDKThreadHelper.hpp>
#include <utility>

//#define RELEASE
constexpr auto MS_TO_NS=1000*1000;

using namespace std::chrono;

static std::chrono::steady_clock::duration ThisThreadSleepUntil(std::chrono::steady_clock::time_point tp){
    std::this_thread::sleep_until(tp);
    const auto now=std::chrono::steady_clock::now();
    return now-tp;
}

FBRManager::FBRManager(VSYNC* vsync):
vsync(*vsync){
    lastLog=steady_clock::now();
    resetTS();
}

void FBRManager::enterWarping(JNIEnv* env,VrCompositorRenderer& vrCompositorRenderer){
    JThread jThread(env);
    Chronometer callJavaTime{"Call java isInterrupted()"};
    while (true){
        callJavaTime.start();
        const bool isInterrupted=jThread.isInterrupted();
        callJavaTime.stop();
        callJavaTime.printInIntervalls(std::chrono::seconds(3),false);
        if(isInterrupted)break;
        warpEyesToFrontBufferSynchronized(env,vrCompositorRenderer);
    }
}

void FBRManager::warpEyesToFrontBufferSynchronized(JNIEnv* env,VrCompositorRenderer& vrCompositorRenderer) {
    const auto latestVSYNC=vsync.getLatestVSYNC();
    const auto nextVSYNCMiddle=latestVSYNC.base+vsync.getEyeRefreshTime()+std::chrono::milliseconds(0);
    const auto nextVSYNC=latestVSYNC.base+vsync.getDisplayRefreshTime()+std::chrono::milliseconds(0);
    if(lastRenderedFrame.count+1!=latestVSYNC.count){
        MLOGE<<"Probably missed VSYNC "<<lastRenderedFrame.count<<" "<<latestVSYNC.count<<" "<<vsync.getVsyncRasterizerPositionNormalized()<<" "<<MyTimeHelper::R(CLOCK::now()-latestVSYNC.base);
    }
    lastRenderedFrame=latestVSYNC;
    // For now assume there is max 1 layer that holds a SurfaceTexture
    SurfaceTextureUpdate* surfaceTextureUpdate=std::get<SurfaceTextureUpdate*>(vrCompositorRenderer.getLayers().at(0).contentProvider);
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
        surfaceTextureUpdate->updateAndCheck(env);
        avgCPUTimeUpdateSurfaceTexture.stop();
        ATrace_endSection();
        ATrace_beginSection("DirectRendering::begin");
        DirectRender::begin(vrCompositorRenderer.getViewportForEye(isLeftEye ? GVR_LEFT_EYE : GVR_RIGHT_EYE));
        ATrace_endSection();
        ATrace_beginSection("renderNewEyeCallback");
        drawEye(env,isLeftEye,vrCompositorRenderer);
        ATrace_endSection();
        ATrace_beginSection("DirectRendering::end");
        DirectRender::end();
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

void FBRManager::drawEye(JNIEnv *env, const bool leftEye, VrCompositorRenderer &vrCompositorRenderer) {
    ATrace_beginSection("drawEye()");
    //Draw the background, which alternates between black and yellow to make tearing observable
    if(CHANGE_CLEAR_COLOR_TO_MAKE_TEARING_OBSERVABLE){
        const int idx=leftEye==0 ? 0 : 1;
        whichColor[idx]++;
        if(whichColor[idx]>1){
            whichColor[idx]=0;
        }
        if(whichColor[idx]==0){
            vrCompositorRenderer.clearViewportUsingRenderedMesh(true);
        }else{
            vrCompositorRenderer.clearViewportUsingRenderedMesh(false);
        }
    }
    vrCompositorRenderer.drawLayers(leftEye ? GVR_LEFT_EYE : GVR_RIGHT_EYE);
    ATrace_endSection();
}

void FBRManager::drawEyesToFrontBufferUnsynchronized(JNIEnv *env,VrCompositorRenderer &vrCompositorRenderer) {
    JThread jThread(env);
    while (!jThread.isInterrupted()){
        SurfaceTextureUpdate* surfaceTextureUpdate=std::get<SurfaceTextureUpdate*>(vrCompositorRenderer.getLayers().at(0).contentProvider);
        for(int eye=0;eye<2;eye++){
            surfaceTextureUpdate->updateAndCheck(env);
            //surfaceTextureUpdate->waitUntilFrameAvailable(env,std::chrono::steady_clock::now()+std::chrono::milliseconds(14));
            const bool isLeftEye=eye==0;
            DirectRender::begin(vrCompositorRenderer.getViewportForEye(isLeftEye ? GVR_LEFT_EYE : GVR_RIGHT_EYE));
            drawEye(env,isLeftEye,vrCompositorRenderer);
            DirectRender::end();
            std::unique_ptr<FenceSync> fenceSync=std::make_unique<FenceSync>();
            glFlush();
            // Make sure that I do not submit eyes faster than the GPU is able to render them
            fenceSync->wait(std::chrono::milliseconds(100));
        }
    }
}



