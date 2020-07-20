//
// Created by Constantin on 30.10.2017.
//

#ifndef OSDTESTER_EXTENSIONS_H
#define OSDTESTER_EXTENSIONS_H

#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/errno.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include <AndroidLogger.hpp>
#include <array>

// https://www.khronos.org/registry/OpenGL/extensions/QCOM/QCOM_tiled_rendering.txt
namespace QCOM_tiled_rendering{
    static PFNGLSTARTTILINGQCOMPROC	glStartTilingQCOM=nullptr;
    static PFNGLENDTILINGQCOMPROC glEndTilingQCOM=nullptr;
    static void init(){
        glStartTilingQCOM= reinterpret_cast<PFNGLSTARTTILINGQCOMPROC>(eglGetProcAddress("glStartTilingQCOM"));
        glEndTilingQCOM = reinterpret_cast<PFNGLENDTILINGQCOMPROC>(eglGetProcAddress("glEndTilingQCOM"));
        if(glStartTilingQCOM== nullptr || glEndTilingQCOM==nullptr){
            MLOGE<<"Cannot initialize QCOM_tiled_rendering";
        }
    }
    static void StartTilingQCOM(int x,int y,int width,int height){
        glStartTilingQCOM(x,y,width,height,0);
    }
    static void StartTilingQCOM(std::array<int,4> rect){
        glStartTilingQCOM(rect[0],rect[1],rect[2],rect[3],0);
    }
    static void EndTilingQCOM() {
        glEndTilingQCOM(GL_COLOR_BUFFER_BIT0_QCOM);
    }
}

// https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt
namespace KHR_debug{
    static PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR=nullptr;
    static PFNGLGETDEBUGMESSAGELOGKHRPROC glGetDebugMessageLogKHR= nullptr;
    static void on_gl_error(unsigned int source,unsigned int type, uint id,unsigned int severity,
                            int length, const char* message,const void *userParam){
        MLOGE2("GL_DEBUG")<<std::string(message);
    }
    static void enable(){
        glDebugMessageCallbackKHR =(PFNGLDEBUGMESSAGECALLBACKKHRPROC)eglGetProcAddress(
                "glDebugMessageCallbackKHR");
        glGetDebugMessageLogKHR =(PFNGLGETDEBUGMESSAGELOGKHRPROC)eglGetProcAddress(
                "glGetDebugMessageLogKHR");
        if(glDebugMessageCallbackKHR==nullptr){
            MLOGE<<"GL_KHR_DEBUG functions not found";
            return;
        }
        // callback is thread safe
        glEnable(GL_DEBUG_OUTPUT_KHR);
        if(!glIsEnabled(GL_DEBUG_OUTPUT_KHR)){
            MLOGE<<"Cannot enable GL_DEBUG_OUTPUT_KHR";
            return;
        }
        glDebugMessageCallbackKHR(on_gl_error, NULL);
    }
}

// https://www.khronos.org/registry/EGL/extensions/ANDROID/EGL_ANDROID_presentation_time.txt
namespace ANDROID_presentation_time{
    static PFNEGLPRESENTATIONTIMEANDROIDPROC eglPresentationTimeANDROID=nullptr;
    static void init(){
        eglPresentationTimeANDROID=reinterpret_cast<PFNEGLPRESENTATIONTIMEANDROIDPROC>(eglGetProcAddress("eglPresentationTimeANDROID"));
        if(eglPresentationTimeANDROID== nullptr){
            MLOGE<<"Cannot initialize ANDROID_presentation_time";
        }
    }
}

// https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_fence_sync.txt
namespace  KHR_fence_sync{
    static PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR_=nullptr;
    static PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR_=nullptr;
    static PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR_=nullptr;
    static PFNEGLGETSYNCATTRIBKHRPROC eglGetSyncAttribKHR=nullptr;
    static void init(){
        eglCreateSyncKHR_ = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(eglGetProcAddress("eglCreateSyncKHR" ));
        eglDestroySyncKHR_ = reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(eglGetProcAddress("eglDestroySyncKHR" ));
        eglClientWaitSyncKHR_ = reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(eglGetProcAddress("eglClientWaitSyncKHR"));
        eglGetSyncAttribKHR = reinterpret_cast<PFNEGLGETSYNCATTRIBKHRPROC>(eglGetProcAddress( "eglGetSyncAttribKHR" ));
        if(eglCreateSyncKHR_==nullptr || eglDestroySyncKHR_==nullptr||eglClientWaitSyncKHR_==nullptr||eglGetSyncAttribKHR==nullptr){
            MLOGE<<"Cannot init KHR_fence_sync";
        }
    }
    static void log(){
        MLOGD<<"eglCreateSyncKHR_"<<(int)(KHR_fence_sync::eglCreateSyncKHR_==nullptr)<<" eglDestroySyncKHR_"<<(int)(KHR_fence_sync::eglDestroySyncKHR_==nullptr)<<"  eglClientWaitSyncKHR_"<<(int)(KHR_fence_sync::eglClientWaitSyncKHR_==nullptr);
    }
}

class FenceSync{
private:
    const EGLDisplay eglDisplay=eglGetCurrentDisplay();
    EGLSyncKHR sync;
    bool hasBeenSatisfied=false;
    std::chrono::steady_clock::time_point satisfiedTime;
public:
    const std::chrono::steady_clock::time_point creationTime=std::chrono::steady_clock::now();
    FenceSync(){
        //KHR_fence_sync::log();
        sync=KHR_fence_sync::eglCreateSyncKHR_(eglDisplay, EGL_SYNC_FENCE_KHR, nullptr);
        if(sync==EGL_NO_SYNC_KHR)MLOGE<<"Cannot create sync";
    }
    ~FenceSync(){
        KHR_fence_sync::init();
        //KHR_fence_sync::log();
        KHR_fence_sync::eglDestroySyncKHR_(eglDisplay,sync);
    }
    // true if condition was satisfied, false otherwise
    bool wait(EGLTimeKHR timeoutNS=0){
        //KHR_fence_sync::init();
        //KHR_fence_sync::log();
        if(hasBeenSatisfied)return true;
        const auto ret=KHR_fence_sync::eglClientWaitSyncKHR_(eglDisplay,sync,EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,timeoutNS);
        if(ret==EGL_CONDITION_SATISFIED_KHR){
            hasBeenSatisfied=true;
            satisfiedTime=std::chrono::steady_clock::now();
            return true;
        }
        return false;
    }
    bool wait(const std::chrono::steady_clock::duration& timeout){
        return wait(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
    }
    std::chrono::steady_clock::duration getDeltaCreationSatisfied(){
        if(!hasBeenSatisfied)return 0ns;
        return satisfiedTime-creationTime;
    }
    bool hasAlreadyBeenSatisfied()const{
        return hasBeenSatisfied;
    }
};


// other extensions
namespace Extensions{
    typedef void (GL_APIENTRYP PFNGLINVALIDATEFRAMEBUFFER_) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
    static PFNGLINVALIDATEFRAMEBUFFER_	glInvalidateFramebuffer_;

    static void initOtherExtensions(){
        glInvalidateFramebuffer_  = (PFNGLINVALIDATEFRAMEBUFFER_)eglGetProcAddress("glInvalidateFramebuffer");
    }
    static void glInvalidateFramebuffer(){
        int count=3;
        const GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
        glInvalidateFramebuffer_( GL_FRAMEBUFFER, count, attachments);
    }
}

#include <optional>
#include <TimeHelper.hpp>
#include <array>

/*namespace EGLExtensionsHelper{
    template<typename T>
    static void eglGetProcAddress(const char* name){
        T f= eglGetProcAddress(name);
    }
}*/

// https://www.khronos.org/registry/EGL/extensions/ANDROID/EGL_ANDROID_get_frame_timestamps.txt
namespace Extensions2{
    static PFNEGLGETNEXTFRAMEIDANDROIDPROC eglGetNextFrameIdANDROID = nullptr;
    static PFNEGLGETFRAMETIMESTAMPSANDROIDPROC eglGetFrameTimestampsANDROID = nullptr;
    static PFNEGLGETCOMPOSITORTIMINGANDROIDPROC eglGetCompositorTimingANDROID=nullptr;
    static PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC eglGetFrameTimestampSupportedANDROID=nullptr;
    static void init(){
        eglGetNextFrameIdANDROID = reinterpret_cast<PFNEGLGETNEXTFRAMEIDANDROIDPROC>(
                eglGetProcAddress("eglGetNextFrameIdANDROID"));
        if (eglGetNextFrameIdANDROID == nullptr) {
            MLOGE<<"Failed to load eglGetNextFrameIdANDROID";
        }
        eglGetFrameTimestampsANDROID = reinterpret_cast<PFNEGLGETFRAMETIMESTAMPSANDROIDPROC>(
                eglGetProcAddress("eglGetFrameTimestampsANDROID"));
        if (eglGetFrameTimestampsANDROID == nullptr) {
            MLOGE<<"Failed to load eglGetFrameTimestampsANDROID";
        }
        eglGetCompositorTimingANDROID= reinterpret_cast<PFNEGLGETCOMPOSITORTIMINGANDROIDPROC >(
                eglGetProcAddress("eglGetCompositorTimingANDROID"));
        if (eglGetFrameTimestampsANDROID == nullptr) {
            MLOGE<<"Failed to load eglGetCompositorTimingANDROID";
        }
        eglGetFrameTimestampSupportedANDROID=reinterpret_cast<PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC>(
                eglGetProcAddress("eglGetFrameTimestampSupportedANDROID"));
        if (eglGetFrameTimestampsANDROID == nullptr) {
            MLOGE<<"Failed to load eglGetFrameTimestampSupportedANDROID";
        }
    }
    static std::optional<EGLuint64KHR> getNextFrameId(EGLDisplay dpy, EGLSurface surface){
        if (eglGetNextFrameIdANDROID == nullptr) {
            MLOGE<<"stats are not supported on this platform";
            return std::nullopt;
        }
        EGLuint64KHR frameId;
        EGLBoolean result = eglGetNextFrameIdANDROID(dpy, surface, &frameId);
        if (result == EGL_FALSE) {
            MLOGE<<"Failed to get next frame ID";
            return std::nullopt;
        }
        return frameId;
    }
    static_assert(sizeof(long long)==8);
    static_assert(sizeof(EGLnsecsANDROID)==8);
    struct FrameTimestamps {
        EGLnsecsANDROID REQUESTED_PRESENT_TIME_ANDROID;
        EGLnsecsANDROID RENDERING_COMPLETE_TIME_ANDROID;
        EGLnsecsANDROID COMPOSITION_LATCH_TIME_ANDROID;
        EGLnsecsANDROID FIRST_COMPOSITION_START_TIME_ANDROID;
        EGLnsecsANDROID LAST_COMPOSITION_START_TIME_ANDROID;
        EGLnsecsANDROID FIRST_COMPOSITION_GPU_FINISHED_TIME_ANDROID;
        EGLnsecsANDROID DISPLAY_PRESENT_TIME_ANDROID;
        EGLnsecsANDROID DEQUEUE_READY_TIME_ANDROID;
        EGLnsecsANDROID READS_DONE_TIME_ANDROID;
    } __attribute__ ((packed));
    static std::optional<FrameTimestamps> getFrameTimestamps(EGLDisplay dpy,EGLSurface surface,EGLuint64KHR frameId){
        const std::array<EGLint,9> timestamps={
                EGL_REQUESTED_PRESENT_TIME_ANDROID,
                EGL_RENDERING_COMPLETE_TIME_ANDROID,
                EGL_COMPOSITION_LATCH_TIME_ANDROID,
                EGL_FIRST_COMPOSITION_START_TIME_ANDROID ,
                EGL_LAST_COMPOSITION_START_TIME_ANDROID ,
                EGL_FIRST_COMPOSITION_GPU_FINISHED_TIME_ANDROID ,
                EGL_DISPLAY_PRESENT_TIME_ANDROID ,
                EGL_DEQUEUE_READY_TIME_ANDROID ,
                EGL_READS_DONE_TIME_ANDROID
        };
        std::vector<EGLnsecsANDROID> values(timestamps.size());
        EGLBoolean result = eglGetFrameTimestampsANDROID(dpy, surface, frameId,
                timestamps.size(), timestamps.data(),values.data());
        if (result == EGL_FALSE) {
            EGLint reason = eglGetError();
            if (reason == EGL_BAD_SURFACE) {
                MLOGD<<"eglSurfaceAttrib EGL_TIMESTAMPS_ANDROID";
                eglSurfaceAttrib(dpy, surface, EGL_TIMESTAMPS_ANDROID, EGL_TRUE);
            }else{
                MLOGE<<"Failed to get timestamps for frame "<<frameId<<" error"<<reason;
            }
            return std::nullopt;
        }
        // try again if we got some pending stats
        for (auto i : values) {
            if (i == EGL_TIMESTAMP_PENDING_ANDROID) return std::nullopt;
        }
        assert(sizeof(FrameTimestamps)==values.size()*sizeof(EGLnsecsANDROID));
        FrameTimestamps frameTimestamps;
        std::memcpy(&frameTimestamps,values.data(),sizeof(FrameTimestamps));
        return frameTimestamps;
    }
    struct CompositorTiming{
        EGLnsecsANDROID COMPOSITE_DEADLINE_ANDROID;
        EGLnsecsANDROID COMPOSITE_INTERVAL_ANDROID;
        EGLnsecsANDROID COMPOSITE_TO_PRESENT_LATENCY_ANDROID;
    }__attribute__ ((packed));
    static CompositorTiming GetCompositorTimingANDROID(EGLDisplay dpy,EGLSurface surface){
        const std::array<EGLint,3> names = {
                EGL_COMPOSITE_DEADLINE_ANDROID,
                EGL_COMPOSITE_INTERVAL_ANDROID,
                EGL_COMPOSITE_TO_PRESENT_LATENCY_ANDROID
        };
        CompositorTiming compositorTiming;
        EGLBoolean result = eglGetCompositorTimingANDROID(dpy, surface,names.size(),names.data(),(EGLnsecsANDROID*)&compositorTiming);
        MLOGD<<"eglGetCompositorTimingANDROID returned "<<result;
        MLOGD2("CompositorTiming")
        <<"COMPOSITE_DEADLINE_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(compositorTiming.COMPOSITE_DEADLINE_ANDROID)-std::chrono::steady_clock::now().time_since_epoch())
        <<"COMPOSITE_INTERVAL_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(compositorTiming.COMPOSITE_INTERVAL_ANDROID))
        <<"COMPOSITE_TO_PRESENT_LATENCY_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(compositorTiming.COMPOSITE_TO_PRESENT_LATENCY_ANDROID));
        return compositorTiming;
    }
    struct SubmittedFrame{
        std::chrono::steady_clock::time_point creationTime;
        EGLuint64KHR frameId;
    };
    static void logStats(const std::chrono::steady_clock::time_point& creationTime,const FrameTimestamps& timestamps){
        AndroidLogger logger(ANDROID_LOG_DEBUG,"FrameTimestamps");
        const auto t=creationTime.time_since_epoch().count();
        logger<<"REQUESTED_PRESENT_TIME "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.REQUESTED_PRESENT_TIME_ANDROID-t));
        logger<<"RENDERING_COMPLETE_TIME "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.RENDERING_COMPLETE_TIME_ANDROID-t));
        logger<<"COMPOSITION_LATCH_TIME "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.COMPOSITION_LATCH_TIME_ANDROID-timestamps.RENDERING_COMPLETE_TIME_ANDROID));
        logger<<"FIRST_COMPOSITION_START_TIME_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.FIRST_COMPOSITION_START_TIME_ANDROID-timestamps.COMPOSITION_LATCH_TIME_ANDROID));
        logger<<"LAST_COMPOSITION_START_TIME_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.LAST_COMPOSITION_START_TIME_ANDROID-timestamps.FIRST_COMPOSITION_START_TIME_ANDROID));
        // value of  EGL_TIMESTAMP_INVALID_ANDROID - never occured
        //logger<<"FIRST_COMPOSITION_GPU_FINISHED_TIME_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.FIRST_COMPOSITION_GPU_FINISHED_TIME_ANDROID-t));
        logger<<"DISPLAY_PRESENT_TIME "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.DISPLAY_PRESENT_TIME_ANDROID-timestamps.LAST_COMPOSITION_START_TIME_ANDROID));
        // these 2 are only usefull to find out when the buffer was usable again, not latency
        //logger<<"DEQUEUE_READY_TIME_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.DEQUEUE_READY_TIME_ANDROID-timestamps.DISPLAY_PRESENT_TIME_ANDROID));
        //logger<<"READS_DONE_TIME_ANDROID "<<MyTimeHelper::R(std::chrono::nanoseconds(timestamps.READS_DONE_TIME_ANDROID-timestamps.DISPLAY_PRESENT_TIME_ANDROID));
    }
}


namespace LOLX{
    static void setAffinity(int core){
        cpu_set_t  cpuset;
        CPU_ZERO(&cpuset);       //clears the cpuset
        CPU_SET( core, &cpuset); //set CPU x on cpuset*/
        long err,syscallres;
        pid_t pid=gettid();
        syscallres=syscall(__NR_sched_setaffinity,pid, sizeof(cpuset),&cpuset);
        if(syscallres) {
            //PrivLOG("Error sched_setaffinity");
        }
    }
}
#endif //OSDTESTER_EXTENSIONS_H
