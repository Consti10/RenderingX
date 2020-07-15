//
// Created by Consti10 on 08/10/2019.
//

#ifndef RENDERINGX_EXAMPLE_DISTORTION_H
#define RENDERINGX_EXAMPLE_DISTORTION_H

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
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
#include <BasicGLPrograms.hpp>
#include <gvr_util/util.h>
#include <GLBuffer.hpp>
#include <VrCompositorRenderer.h>

// Example that renders 360° video with the Vr compositor renderer using VDDC
class Renderer360Video{
public:
    enum SPHERE_MODE{SPHERE_MODE_EQUIRECTANGULAR_TEST,SPHERE_MODE_INSTA360_TEST2};
    const SPHERE_MODE M_SPHERE_MODE;
public:
    Renderer360Video(JNIEnv* env, jobject androidContext, gvr_context *gvr_context,
                       bool RENDER_SCENE_USING_GVR_RENDERBUFFER=true, bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT=true,const int SPHERE_MODE=0);
    void onSurfaceCreated(JNIEnv* env,jobject context,int videoTexture);
    void onDrawFrame();
private:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    FPSCalculator mFPSCalculator;
    GLuint mVideoTexture;
    GLuint mExampleUiTexture;
    GLuint mSomethingTexture;
public:
    VrCompositorRenderer vrCompositorRenderer;
};

#endif //RENDERINGX_EXAMPLE_DISTORTION_H
