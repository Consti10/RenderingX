//
// Created by Consti10 on 30/08/2019.
//

#ifndef RENDERINGX_EXAMPLE_SUPERSYNC_H
#define RENDERINGX_EXAMPLE_SUPERSYNC_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <GLProgramVC.h>
#include <GLProgramText.h>
#include <GLProgramTexture.h>
#include <ColoredGeometry.hpp>
#include <TexturedGeometry.hpp>
#include <TextAssetsHelper.hpp>

#include <GLProgramLine.h>
#include <Chronometer.h>
#include <FPSCalculator.h>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

//in contrast to example_renderer, here we are using an object-oriented native binding approach - the lifecycle of the cpp object  is tied to the lifecycle
//of the same named java object
//See implementation for more details on SuperSync

//Super sync (synchronous front buffer rendering) is a technique for reducing latency in VR
//It renders both the left and right eye alternating in sync with the VSYNC.
//Since it only halves the frame buffer, it can be both called scan line racing and scan line chasing

//This is just an example for showing the basics, see FPV-VR for a useful implementation
//It only clears the screen and renders a triangle from the left and right eye view


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
    //
    //Camera/Projection matrix constants
    static constexpr float CAMERA_DISTANCE=-5.0f;
    //the projection matrix. Should be re calculated in each onSurfaceChanged with new width and height
    glm::mat4 projection;
    //the view matrix, with ipd
    glm::mat4 eyeView;
    glm::mat4 leftEyeView,rightEyeView;
    //holds colored geometry vertices
    GLProgramVC::Mesh mVertexBufferVC;
    //
    GLProgramVC::Mesh solidRectangleYellow;
    GLProgramVC::Mesh solidRectangleBlack;
    std::array<int,2> whichColor;
    static constexpr int N_TRIANGLES=5;
    static constexpr int N_COLOR_VERTICES=3*N_TRIANGLES;
    //Create buffer(s) for
    std::array<VertexBuffer,10> triangleBuffersVCRed;
    std::array<VertexBuffer,10> triangleBuffersVCYellow;
};

#endif //RENDERINGX_EXAMPLE_SUPERSYNC_H
