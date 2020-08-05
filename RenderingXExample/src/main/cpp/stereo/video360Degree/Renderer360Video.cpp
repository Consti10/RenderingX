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

Renderer360Video::Renderer360Video(JNIEnv *env, jobject androidContext, gvr_context *gvr_context,const int vSPHERE_MODE):
        M_SPHERE_MODE(static_cast<SPHERE_MODE>(vSPHERE_MODE)),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
        vrCompositorRenderer(env,androidContext,gvr_api_.get(),true,true),
        mFPSCalculator("OpenGL FPS", std::chrono::seconds(2)){
}


void Renderer360Video::onSurfaceCreated(JNIEnv *env, jobject context, int videoTexture) {
    Extensions::initializeGL();
    vrCompositorRenderer.initializeGL();
    mVideoTexture=(GLuint)videoTexture;
    //vrRenderBuffer2.createRenderTextures(1280,720);
    //glGenTextures(1,&mExampleUiTexture);
    GLProgramTexture::loadTexture(mExampleUiTexture,env,context,"ExampleTexture/ui.png");
    //GLProgramTexture::loadTexture(mSomethingTexture,env,context,"ExampleTexture/something.png");
    /*glGenTextures(1,&mTexture360Image);
    glGenTextures(1,&mTexture360ImageInsta360);
    GLProgramTexture::loadTexture(mTexture360Image,env,context,"360DegreeImages/gvr_testroom_mono.png");
    GLProgramTexture::loadTexture(mTexture360ImageInsta360,env,context,"360DegreeImages/insta_360_equirectangular.png");*/
    vrCompositorRenderer.removeLayers();
    if(M_SPHERE_MODE==SPHERE_MODE_EQUIRECTANGULAR_TEST){
        vrCompositorRenderer.addLayerSphere360(10.0f,UvSphere::MEDIA_EQUIRECT_MONOSCOPIC,videoTexture,true);
    }else{
        auto sphere=DualFisheyeSphere::createSphereGL(2560, 1280);
        vrCompositorRenderer.addLayer(sphere, mVideoTexture, true, VrCompositorRenderer::HEAD_TRACKING::FULL);
    }
    const float uiElementWidth=2.0;
    vrCompositorRenderer.addLayer2DCanvas(-3, uiElementWidth,uiElementWidth*1080.0f/2160.0f,mExampleUiTexture, false, VrCompositorRenderer::FULL);
    // add a static layer to test the pre-distort feature
    //vrCompositorRenderer.addLayer2DCanvas(-3,0.2f,0.2f,mSomethingTexture,false,VrCompositorRenderer::NONE);
}

void Renderer360Video::onDrawFrame() {
    mFPSCalculator.tick();
    //MLOGD<<"FPS: "<<mFPSCalculator.getCurrentFPS();
    vrCompositorRenderer.setLayerTextureId(1, vrRenderBuffer2.getLatestRenderedTexture());

    //Update the head position (rotation) then leave it untouched during the frame
    vrCompositorRenderer.updateLatestHeadSpaceFromStartSpaceRotation();

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

void Renderer360Video::onSecondaryContextCreated(JNIEnv* env,jobject context) {
    //glGenTextures(1,&mExampleUiTexture);
    //GLProgramTexture::loadTexture(mExampleUiTexture,env,context,"ExampleTexture/ui.png");
    //glFlush();
    vrRenderBuffer.initializeGL(1280, 720);
    vrRenderBuffer2.createRenderTextures(1280,720);
    vrRenderBuffer2.createFrameBuffers();
}

void Renderer360Video::onSecondaryContextDoWork(JNIEnv *env) {
    /*vrRenderBuffer.bind();
    GLHelper::updateSetClearColor(clearColorIndex);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    vrRenderBuffer.unbind();
    MLOGD<<"Do work";*/
    vrRenderBuffer2.bind1();
    GLHelper::updateSetClearColor(clearColorIndex);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    vrRenderBuffer2.unbindAndSwap();
    //MLOGD<<"Do work";
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
(JNIEnv *env, jobject obj,jobject androidContext,jlong native_gvr_api,jint SPHERE_MODE) {
    return jptr(new Renderer360Video(env, androidContext, reinterpret_cast<gvr_context *>(native_gvr_api),SPHERE_MODE));
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

JNI_METHOD(void, nativeOnSecondaryContextCreated)
(JNIEnv *env, jobject obj,jlong p,jobject context) {
    native(p)->onSecondaryContextCreated(env,context);
}
JNI_METHOD(void, nativeOnSecondaryContextDoWork)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onSecondaryContextDoWork(env);
}

}
