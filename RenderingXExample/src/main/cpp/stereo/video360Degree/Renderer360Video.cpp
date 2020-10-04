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
        vrSettings(env,androidContext),
        surfaceTextureUpdate(env),
        M_SPHERE_MODE(static_cast<SPHERE_MODE>(vSPHERE_MODE)),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
        vrCompositorRenderer(env, androidContext, gvr_api_.get(),
                             vrSettings.isVR_DISTORTION_CORRECTION_ENABLED(), vrSettings.VR_ENABLE_DEBUG),
        mFPSCalculator("OpenGL FPS", std::chrono::seconds(2)){
}


void Renderer360Video::onSurfaceCreated(JNIEnv *env, jobject context,jobject surfaceTextureHolder) {
    Extensions::initializeGL();
    vrCompositorRenderer.initializeGL();
    surfaceTextureUpdate.updateFromSurfaceTextureHolder(env,surfaceTextureHolder);
    vrRenderBufferExampleUi.loadDefaultTexture(env,context);
    //GLProgramTexture::loadTexture(mSomethingTexture,env,context,"ExampleTexture/something.png");
    /*glGenTextures(1,&mTexture360Image);
    glGenTextures(1,&mTexture360ImageInsta360);
    GLProgramTexture::loadTexture(mTexture360Image,env,context,"360DegreeImages/gvr_testroom_mono.png");
    GLProgramTexture::loadTexture(mTexture360ImageInsta360,env,context,"360DegreeImages/insta_360_equirectangular.png");*/
    vrCompositorRenderer.removeLayers();
    if(M_SPHERE_MODE==SPHERE_MODE_EQUIRECTANGULAR_TEST){
        vrCompositorRenderer.addLayerSphere360(10.0f,UvSphere::MEDIA_EQUIRECT_MONOSCOPIC,&surfaceTextureUpdate);
    }else{
        auto sphere=DualFisheyeSphere::createSphereGL(2560, 1280);
        vrCompositorRenderer.addLayer(sphere,&surfaceTextureUpdate, VrCompositorRenderer::HEAD_TRACKING::FULL);
    }
    const float uiElementWidth=2.0;
    vrCompositorRenderer.addLayer2DCanvas(-3, uiElementWidth,uiElementWidth*1080.0f/2160.0f,&vrRenderBuffer2, VrCompositorRenderer::FULL);
    // add a static layer to test the pre-distort feature
    //vrCompositorRenderer.addLayer2DCanvas(-3,0.2f,0.2f,mSomethingTexture,false,VrCompositorRenderer::NONE);
}

void Renderer360Video::onDrawFrame(JNIEnv* env) {
    mFPSCalculator.tick();
    /*const auto timeP=std::chrono::steady_clock::now()+std::chrono::seconds(1);
    if(const auto delay=surfaceTextureUpdate.waitUntilFrameAvailable(env,timeP)){
        videoFrameWaitTime.add(*delay);
        //MLOGD<<"avg Latency until opengl is "<<surfaceTextureDelay.getAvg_ms();
        //MLOGD<<MyTimeHelper::R(*delay)<<" | "<<videoFrameWaitTime.getAvgReadable();
    }else{
        //MLOGD<<"Timeout";
    }*/

    surfaceTextureUpdate.updateAndCheck(env);
    MLOGD<<"FPS: "<<mFPSCalculator.getCurrentFPS();

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
    Extensions::eglPresentationTimeANDROID(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW),std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    GLHelper::checkGlError("Renderer360Video::onDrawFrame");
    //eglSwapBuffers(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
}

void Renderer360Video::onSecondaryContextCreated(JNIEnv* env,jobject context) {
    vrRenderBuffer2.initializeGL();
    vrRenderBuffer2.setSize(1280,720);
}

void Renderer360Video::onSecondaryContextDoWork(JNIEnv *env) {
    //MLOGD<<"Do work";
    vrRenderBuffer2.bind();
    GLHelper::updateSetClearColor(clearColorIndex);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    vrRenderBuffer2.unbindAndSwap();
    //MLOGD<<"Do work";
    mFPSCalculatorRenderbuffer.tick();
    MLOGD<<"OSD FPS: "<<mFPSCalculatorRenderbuffer.getCurrentFPS();
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
(JNIEnv *env, jobject obj,jlong p,jobject androidContext,jobject surfaceTextureHolder) {
    native(p)->onSurfaceCreated(env,androidContext,surfaceTextureHolder);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onDrawFrame(env);
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
