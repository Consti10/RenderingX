/* *********************************************************************************************************************************************
 * Front Buffer rendering
 * 1 Callback Function to register:
 * //called when the VSYNC rasterizer is at Position <0.5 or Position >0.5
 * //@param whichEye: leftEye if VSYNC is at Position 0.5, rightEye if at Position 0
 * //@param offsetNS: how much time has already elapsed since the eye event
 * onRenderNewEye(boolean whichEye,int64_t offsetNS)
 * //If the application did not render the last eye in less than 8.3ms (half a frame),but in less than 16.6ms, the function will still be called.
 * //With offset being the time the last function exceeded its time frame.
 * //   It is up to the developer to decide what to do in this case. E.g. you might say:
 * //   I only have a 6ms time frame instead of the 8.3ms that would normally be available.
 * //   I can skip some heavy-weight calculations and probably render the eye in <6ms
 * //   Or you might say: I only have a 3ms time frame. I won't be able to render anything usable in this time, So it is better to just skip the eye
 * //   It is,however, granted that onNewEye will be called with left and right eye alternating.
 * //   Even If a eye takes more than 16.6ms to render, the app waits until the next other eye can be drawn;
 * //   so the following constellation CAN happen:
 * //       Rendering the left eye takes 19ms. The next callback has to be with rightEye, so the app waits 5ms until the right eye is safe to render.
 * //       Even though it also could call the onRenderNewEye callback with eye==left eye and offset==3ms
 * //   This is because when one eye takes 16.6ms or more there is something seriously wrong
 *
 * Note: In my earlier work i did use a glFinish() after each eye. Per definition, this not only flushes the pipeline, but also blocks
 * the CPU until all commands are executed. This allowed me to also measure the time the GPU needed to render a eye. However, with the move of
 * "updateTexImage()" to the beginning of the frame I assume the execution on the GPU gets slightly more inefficient (more stalls e.g.) for the GPU
 * (while reducing latency, on the other hand). And a execution of the 3 Stages:
 * 1)create a eye Command buffer on CPU & send it to the GPU
 * 2)the GPU updates the surfaceTexture
 * 3)the GPU renders the eye into the frontbuffer
 * In less than 8.3ms is harder than before (and, I would guess, fails ~5% of the time on my Nexus5X)
 * But it is still possible to do a synchronous rendering into the FB, as long as the 1. stage doesn't take more than 8.3ms, and the 2. and 3. stages
 * also don't take more than 8.3ms together.
 * To archive this, we mustn't block the CPU while GPU is executing commands -> just submit the command buffer fast enough and hope the GPU finishes its
 * work before the rasterizer begins scanning the display.
 * This is achievable by calling glFlush() btw. eglClientWaitSync(...EGL_FLUSH...). But it becomes significant harder to measure the GPU time,
 * since this requires some active "check" by the OpenGL thread. This is only possible during the waitUntilEyeEvent phase.
 * So when the GPU finishes its work during the wait event we can measure the GPU time really accurate- but if it doesn't we can't.
 * So the FBRManager class measures the following:
 * leftEye/rightEyeGPU time: time between "endDirectRendering" (where the sync is created) and the time the sync is signaled (Condition_satisfied).
 * But this value only gets written if the time was actually measurable. If not,
 * leftEye/rightEyeNotMeasurableEyes is incremented, and the % of left/right eyes that couldn't be measured is calculated.
 * If there is no VSYNC_CALLBACK_ADVANCE this % is also a indication of how many frames "failed" btw. did tear
 * *********************************************************************************************************************************************/

#ifndef FPV_VR_FBRMANAGER2_H
#define FPV_VR_FBRMANAGER2_H


#include <sys/types.h>
#include <functional>
#include <jni.h>
#include <vector>
#include <atomic>
#include <EGL/eglext.h>
#include "VSYNC.h"
#include "DirectRender.hpp"
#include <SurfaceTextureUpdate.hpp>
#include <VrCompositorRenderer.h>

//using RENDER_NEW_EYE_CALLBACK=std::function<void(JNIEnv*,bool)>;

class FBRManager{
public:
    FBRManager(VSYNC* vsync,bool CHANGE_CLEAR_COLOR_TO_MAKE_TEARING_OBSERVABLE);
    // Runs until the current thread is interrupted (java thread)
    // You can do optional processing in the optional callback that is called once per frame
    void enterWarping(JNIEnv* env,VrCompositorRenderer& vrCompositorRenderer,std::function<void(JNIEnv*)> optionalCallback=nullptr);
    // warp eyes at the right time into front buffer to avoid tearing
    void warpEyesToFrontBufferSynchronized(JNIEnv* env,VrCompositorRenderer& vrCompositorRenderer);
    //
    void drawEyesToFrontBufferUnsynchronized(JNIEnv* env,VrCompositorRenderer& vrCompositorRenderer);
    //
    void drawFramesToFrontBufferUnsynchronized(JNIEnv* env, VrCompositorRenderer& vrCompositorRenderer);
private:
    const bool CHANGE_CLEAR_COLOR_TO_MAKE_TEARING_OBSERVABLE=false;
    const VSYNC& vsync;
    struct EyeChrono{
        Chronometer avgCPUTime{};
        AvgCalculator avgGPUTime;
        double nEyes=0;
        double nEyesNotMeasurable=0;
    };
    Chronometer avgCPUTimeUpdateSurfaceTexture;
    // return the overshoot
    static CLOCK::duration waitUntilTimePoint(const std::chrono::steady_clock::time_point& timePoint,FenceSync& fenceSync);
    std::array<EyeChrono,2> eyeChrono={};
    //const RENDER_NEW_EYE_CALLBACK onRenderNewEyeCallback;
    std::array<Chronometer,2> vsyncWaitTime={Chronometer{"VSYNC start wait time"},Chronometer{"VSYNC middle wait time"}};
    void printLog();
    std::chrono::steady_clock::time_point lastLog;
    void resetTS();
    VSYNC::VSYNCState lastRenderedFrame;
    void drawEye(JNIEnv* env,const bool isLeftEye,VrCompositorRenderer& vrCompositorRenderer);
    std::array<int,2> whichColor;
};

//While the CPU creates the command buffer it is guaranteed that the Frame Buffer won't be affected. (only as soon as glFinish()/glFlush() is called)
//Creating the command buffer and updating the external video texture takes at least 2ms (in average much more, about 4-5ms in total.But if there
//is no surfaceTexture to update it might take significant less time than 4-5ms)
//When VSYNC_CALLBACK_ADVANCE>0, the callback gets invoked earlier, so the GPU has more time to finish rendering before the pixels are read by the rasterizer.
//However, VSYNC_CALLBACK_ADVANCE also creates as much latency.
//But on my Nexus 5X, with the new change of moving surfaceTextureUpdateImage2D to the beginning of the rendering loop, The renderer can't
//create all command buffer data, update surfaceTextureExternal AND render the frame in <8.3ms with 4xMSAA enabled. I don't use it, however,
//since this small tearing is barely noticeable
//##change 28.12.2017:## Since on daydream-ready phones (e.g. my ZTE axon 7) calc&rendering is consistent way below 8.3ms, i change vsync callback advance
//dynamically. If the GPU fails too often, i set callback advance to 2ms (fixed value). If the gpu does not fail, I set the callback advance to
//-(VsyncWaitTime-1)
//poitive values mean the callback fires earlier
#endif //FPV_VR_FBRMANAGER2_H
