//
// Created by Consti10 on 30/08/2019.
//

#ifndef RENDERINGX_RENDERERSUPERSYNC_H
#define RENDERINGX_RENDERERSUPERSYNC_H

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
#include <FPSCalculator.h>
#include <VrCompositorRenderer.h>
#include <SurfaceTextureUpdate.hpp>

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


class RendererSuperSync{
public:
    RendererSuperSync(JNIEnv* env, jobject androidContext, gvr_context *gvr_context,jlong VSYNC);
    /**
     * WARNING: does not return until exitSuperSyncLoop is called. Basically it blocks the GL thread.
    */
    void enterSuperSyncLoop(JNIEnv * env, jobject obj);
    void onSurfaceCreated(JNIEnv * env,jobject obj,jobject surfaceTextureHolder,int width, int height);
private:
    /**
     * Called by the SuperSync manager
     * @param env
     * @param whichEye left/right eye
     */
    void renderNewEyeCallback(JNIEnv* env,bool whichEye);
    void drawEye(JNIEnv* env,bool whichEye);
private:
    GLProgramVC* glProgramVC2D;
    std::unique_ptr<FBRManager> mFBRManager= nullptr;
    ColoredGLMeshBuffer solidRectangleYellow;
    ColoredGLMeshBuffer solidRectangleBlack;
    std::array<int,2> whichColor;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    SurfaceTextureUpdate mSurfaceTextureUpdate;
public:
    VrCompositorRenderer vrCompositorRenderer;
    VrRenderBuffer2 vrRenderBufferExampleTexture{"ExampleTexture/ui.png"};
};

#endif //RENDERINGX_RENDERERSUPERSYNC_H
