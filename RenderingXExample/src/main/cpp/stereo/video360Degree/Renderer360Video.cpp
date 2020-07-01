//
// Created by Consti10 on 15/05/2019.
//
#include "Renderer360Video.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include <GLBuffer.hpp>
#include <MatrixHelper.h>
#include <array>
#include <Sphere/DualFisheyeSphere.hpp>
#include <Sphere/UvSphere.hpp>
#include <CardboardViewportOcclusion.hpp>
#include <Sphere/SphereBuilder.hpp>

Renderer360Video::Renderer360Video(JNIEnv *env, jobject androidContext, gvr_context *gvr_context, bool RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                       bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT, const int vSPHERE_MODE):
        M_SPHERE_MODE(static_cast<SPHERE_MODE>(vSPHERE_MODE)),
        vrCompositorRenderer(true,TrueColor(glm::vec4{1.0f, 0.1, 0.1, 1.0})),
        mFPSCalculator("OpenGL FPS", 2000){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrCompositorRenderer.distortionEngine.setGvrApi(gvr_api_.get());
}


void Renderer360Video::onSurfaceCreated(JNIEnv *env, jobject context, int videoTexture) {
    vrCompositorRenderer.initializeGL();
    mVideoTexture=(GLuint)videoTexture;
    glGenTextures(1,&mExampleUiTexture);
    GLProgramTexture::loadTexture(mExampleUiTexture,env,context,"ExampleTexture/ui.png");
    /*glGenTextures(1,&mTexture360Image);
    glGenTextures(1,&mTexture360ImageInsta360);
    GLProgramTexture::loadTexture(mTexture360Image,env,context,"360DegreeImages/gvr_testroom_mono.png");
    GLProgramTexture::loadTexture(mTexture360ImageInsta360,env,context,"360DegreeImages/insta_360_equirectangular.png");*/
    GLProgramTexture::TexturedMesh videoSphere360;
    switch(M_SPHERE_MODE){
        case SPHERE_MODE_EQUIRECTANGULAR_TEST:{
            videoSphere360=SphereBuilder::createSphereEquirectangularMonoscopic(1.0, 72, 36);
        }break;
        case SPHERE_MODE_INSTA360_TEST2:{
            videoSphere360=DualFisheyeSphere::createSphereGL(2560, 1280);
        }break;
        default:
            MLOGE<<"Unknown sphere mode";
    }
    vrCompositorRenderer.removeLayers();
    vrCompositorRenderer.addLayer(std::move(videoSphere360), mVideoTexture, true, VrCompositorRenderer::HEAD_TRACKING::FULL);
    const float uiElementWidth=2.0;
    vrCompositorRenderer.addLayer2DCanvas(-2,uiElementWidth,uiElementWidth*1080.0f/2160.0f,mExampleUiTexture,false);
}

void Renderer360Video::onDrawFrame() {
    mFPSCalculator.tick();
    //LOGD("FPS: %f",mFPSCalculator.getCurrentFPS());

    //Update the head position (rotation) then leave it untouched during the frame
    vrCompositorRenderer.distortionEngine.updateLatestHeadSpaceFromStartSpaceRotation();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    for(int eye=0;eye<2;eye++){
        vrCompositorRenderer.drawLayers(static_cast<gvr::Eye>(eye));
    }
    GLHelper::checkGlError("Renderer360Video::onDrawFrame");
}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_stereo_video360degree_Renderer360Video_##method_name

inline jlong jptr(Renderer360Video *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline Renderer360Video *native(jlong ptr) {
    return reinterpret_cast<Renderer360Video*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong native_gvr_api,jboolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
 jboolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,jint SPHERE_MODE) {
    return jptr(new Renderer360Video(env, androidContext, reinterpret_cast<gvr_context *>(native_gvr_api), RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                       RENDER_SCENE_USING_VERTEX_DISPLACEMENT,SPHERE_MODE));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
    delete native(p);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong p,jobject androidContext,jint videoTexture) {
    native(p)->onSurfaceCreated(env,androidContext,videoTexture);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onDrawFrame();
}

JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instancePointer,jobject instanceMyVrHeadsetParams) {
    const MVrHeadsetParams deviceParams=createFromJava(env, instanceMyVrHeadsetParams);
    native(instancePointer)->vrCompositorRenderer.updateHeadsetParams(deviceParams);
}

}
