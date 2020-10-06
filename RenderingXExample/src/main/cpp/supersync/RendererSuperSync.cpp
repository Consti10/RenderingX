//
// Created by Consti10 on 28/08/2019.
//
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
#include <ColoredGeometry.hpp>
#include <TextAssetsHelper.hpp>
#include <BasicGLPrograms.hpp>
#include <GLBuffer.hpp>
#include <FBRManager.h>
#include <Extensions.h>

#include "GLProgramLine.h"
#include "FPSCalculator.hpp"
#include "RendererSuperSync.h"

RendererSuperSync::RendererSuperSync(JNIEnv *env, jobject androidContext, gvr_context *gvr_context,jlong vsync):
        mSurfaceTextureUpdate(env),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context))
        ,vrCompositorRenderer(env,androidContext,gvr_api_.get(),true,false,false),
        mFBRManager(VSYNC::native(vsync),true){
}

void RendererSuperSync::onSurfaceCreated(JNIEnv *env, jobject androidContext,jobject surfaceTextureHolder,int width, int height) {
    Extensions::initializeGL();
    vrCompositorRenderer.initializeGL();
    gvr_api_->InitializeGl();
    mSurfaceTextureUpdate.updateFromSurfaceTextureHolder(env,surfaceTextureHolder);
    vrRenderBufferExampleTexture.loadDefaultTexture(env,androidContext);
    vrCompositorRenderer.removeLayers();

    float uiElementWidth=3.0;
    vrCompositorRenderer.addLayer2DCanvas(-3, uiElementWidth,uiElementWidth*1080.0f/2160.0f,&mSurfaceTextureUpdate, VrCompositorRenderer::NONE);
    uiElementWidth=1.5;
    vrCompositorRenderer.addLayer2DCanvas(-3, uiElementWidth,uiElementWidth*1080.0f/2160.0f,&vrRenderBufferExampleTexture, VrCompositorRenderer::NONE);
}

void RendererSuperSync::enterSuperSyncLoop(JNIEnv *env, jobject obj) {
    mFBRManager.enterWarping(env,vrCompositorRenderer);
    //mFBRManager.drawEyesToFrontBufferUnsynchronized(env,vrCompositorRenderer);
}

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_supersync_GLRExampleSuperSync_##method_name

inline jlong jptr(RendererSuperSync *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline RendererSuperSync *native(jlong ptr) {
    return reinterpret_cast<RendererSuperSync *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong native_gvr_api,jlong vsync) {
    return jptr(new RendererSuperSync(env, androidContext, reinterpret_cast<gvr_context *>(native_gvr_api),vsync));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject androidContext,jobject surfaceTextureHolder,jint width, jint height) {
    native(glRendererStereo)->onSurfaceCreated(env,androidContext,surfaceTextureHolder,width,height);
}
JNI_METHOD(void, nativeEnterSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    native(glRendererStereo)->enterSuperSyncLoop(env,obj);
}

}