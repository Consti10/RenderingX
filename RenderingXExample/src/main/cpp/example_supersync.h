//
// Created by Consti10 on 30/08/2019.
//

#ifndef RENDERINGX_EXAMPLE_SUPERSYNC_H
#define RENDERINGX_EXAMPLE_SUPERSYNC_H

#include "../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/jni.h"
#include "../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/GLES2/gl2.h"
#include "../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/vector"
#include "../../../../RenderingXCore/libs/glm/glm/glm.hpp"
#include "../../../../RenderingXCore/libs/glm/glm/gtc/matrix_transform.hpp"
#include "../../../../RenderingXCore/src/main/cpp/GLPrograms/GLProgramTexture.h"
#include "../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/EGL/egl.h"
#include "../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/cinttypes"
#include "../../../../RenderingXCore/src/main/cpp/SuperSync/Extensions.hpp"
#include "../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/sys/resource.h"
#include "../../../../RenderingXCore/src/main/cpp/SuperSync/FBRManager.h"
#include "../../../../RenderingXCore/src/main/cpp/Time/VRFrameCPUChronometer.h"

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
    void drawEye(JNIEnv* env,bool whichEye);
private:
    VRFrameTimeAccumulator mFrameTimeAcc;
    //std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    GLProgramVC* glProgramVC;
    std::unique_ptr<FBRManager> mFBRManager= nullptr;
    int ViewPortW=0,ViewPortH=0;
    int swapColor=0;
    //
    //Camera/Projection matrix constants
    static constexpr float CAMERA_DISTANCE=-5.0f;
    //the projection matrix. Should be re calculated in each onSurfaceChanged with new width and height
    glm::mat4 projection;
    //the view matrix, with ipd
    glm::mat4 eyeView;
    glm::mat4 leftEyeView,rightEyeView;
    //holds colored geometry vertices
    GLuint glBufferVC;
    static constexpr int N_TRIANGLES=5;
    static constexpr int N_COLOR_VERTICES=3*N_TRIANGLES;
};

#endif //RENDERINGX_EXAMPLE_SUPERSYNC_H
