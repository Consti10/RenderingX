//
// Created by Consti10 on 30/08/2019.
//

#ifndef RENDERINGX_EXAMPLE_SUPERSYNC_H
#define RENDERINGX_EXAMPLE_SUPERSYNC_H

#include "jni.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTextureExt.h>
#include <EGL/egl.h>
#include <cinttypes>
#include "Extensions.hpp"
#include <sys/resource.h>
#include <FBRManager.h>
#include "../Time/VRFrameCPUChronometer.h"
#include <vector>

//in contrast to example_renderer, here we are using an object-oriented native binding approach - the lifecycle of the cpp object  is tied to the lifecycle of the same named java object
//See implementation for more details on SuperSnyc

class GLRSuperSyncExample{
public:
    GLRSuperSyncExample(JNIEnv* env,jobject androidContext,bool qcomTiledRenderingAvailable,bool reusableSyncAvailable);
    /**
     * WARNING: does not return until exitSuperSyncLoop is called. Basically it blocks the GL thread.
    */
    void enterSuperSyncLoop(JNIEnv * env, jobject obj,int exclusiveVRCore);
    /**
     * Exit the SuperSync loop. Since the super sync loop blocks the GLThread, this has to be called from another thread, e.g the UI thread
     * called by the UI's onPause().
     */
    void exitSuperSyncLoop();
    /**
     * Pass trough the last VSYNC from java
     * @param lastVSYNC last reported vsync, in ns
     */
    void setLastVSYNC(int64_t lastVSYNC);
    void onSurfaceCreated(JNIEnv * env,jobject obj);
    void onSurfaceChanged(int width, int height);
private:
    /**
     * Called by the SuperSync manager
     * @param env
     * @param whichEye left/right eye
     * @param offsetNS  time since eye event
     */
    void renderNewEyeCallback(JNIEnv* env,bool whichEye,int64_t offsetNS);
private:
    VRFrameCPUChronometer mFrameCPUChronometer;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<FBRManager> mFBRManager= nullptr;
    int ViewPortW=0,ViewPortH=0;
    int swapColor=0;
    const float MAX_FOV_USABLE_FOR_VDDC=70;
};

#endif //RENDERINGX_EXAMPLE_SUPERSYNC_H
