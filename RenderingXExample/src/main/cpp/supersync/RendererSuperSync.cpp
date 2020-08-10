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
#include "FPSCalculator.h"
#include "VRFrameCPUChronometer.h"
#include "RendererSuperSync.h"

RendererSuperSync::RendererSuperSync(JNIEnv *env, jobject androidContext, gvr_context *gvr_context,jlong vsync):
        mSurfaceTextureUpdate(env),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context))
        ,vrCompositorRenderer(env,androidContext,gvr_api_.get(),true,false,false)
        {
    std::function<void(JNIEnv *env2, bool leftEye)> f = [this](JNIEnv *env2, bool leftEye) {
        this->renderNewEyeCallback(env2,leftEye);
    };
    mFBRManager=std::make_unique<FBRManager>(VSYNC::native(vsync),f,mSurfaceTextureUpdate);
}

void RendererSuperSync::onSurfaceCreated(JNIEnv *env, jobject androidContext,jobject surfaceTextureHolder,int width, int height) {
    Extensions::initializeGL();
    vrCompositorRenderer.initializeGL();
    gvr_api_->InitializeGl();
    mSurfaceTextureUpdate.updateFromSurfaceTextureHolder(env,surfaceTextureHolder);
    glGenTextures(1,&mBlueTexture);
    glGenTextures(1,&mGreenTexture);
    glGenTextures(1,&mExampleUITexture);
    GLProgramTexture::loadTexture(mBlueTexture,env,androidContext,"ExampleTexture/blue.png");
    GLProgramTexture::loadTexture(mGreenTexture,env,androidContext,"ExampleTexture/green.png");
    GLProgramTexture::loadTexture(mExampleUITexture,env,androidContext,"ExampleTexture/ui.png");
    vrCompositorRenderer.removeLayers();

    float uiElementWidth=3.0;
    vrCompositorRenderer.addLayer2DCanvas(-3, uiElementWidth,uiElementWidth*1080.0f/2160.0f,mSurfaceTextureUpdate.getTextureId(), true, VrCompositorRenderer::NONE);
    uiElementWidth=1.5;
    vrCompositorRenderer.addLayer2DCanvas(-3, uiElementWidth,uiElementWidth*1080.0f/2160.0f,mExampleUITexture, false, VrCompositorRenderer::NONE);

    glProgramVC2D=new GLProgramVC(true);
    solidRectangleBlack.setData(
            ColoredGeometry::makeTessellatedColoredRect(10, {0,0,0}, {2,2}, TrueColor2::BLACK));
    solidRectangleYellow.setData(
            ColoredGeometry::makeTessellatedColoredRect(10, {0,0,0}, {2,2}, TrueColor2::YELLOW));
}

void RendererSuperSync::enterSuperSyncLoop(JNIEnv *env, jobject obj) {
    mFBRManager->warpEyesToFrontBufferSynchronized(env, vrCompositorRenderer.SCREEN_WIDTH_PX,
                                                   vrCompositorRenderer.SCREEN_HEIGHT_PX);
    //mFBRManager->drawLeftAndRightEye(env,vrCompositorRenderer.SCREEN_WIDTH_PX,vrCompositorRenderer.SCREEN_HEIGHT_PX);
}

void RendererSuperSync::renderNewEyeCallback(JNIEnv *env, const bool leftEye) {
    //if(const auto delay=mSurfaceTextureUpdate.updateAndCheck(env)){
    //    //MLOGD<<"avg Latency until opengl is "<<MyTimeHelper::R(*delay);
    //}
    glClearColor(0.5F,0.5F,0.5F,0.0F);
    drawEye(env,leftEye);
}

void RendererSuperSync::drawEye(JNIEnv *env, bool leftEye) {
    ATrace_beginSection("drawEye()");
    //Draw the background, which alternates between black and yellow to make tearing observable
    const int idx=leftEye==0 ? 0 : 1;
    whichColor[idx]++;
    if(whichColor[idx]>1){
        whichColor[idx]=0;
    }
    if(whichColor[idx]==0){
        glProgramVC2D->drawX(glm::mat4(),glm::mat4(), solidRectangleBlack);
    }else{
        glProgramVC2D->drawX(glm::mat4(),glm::mat4(), solidRectangleYellow);
    }
    vrCompositorRenderer.drawLayers(leftEye ? GVR_LEFT_EYE : GVR_RIGHT_EYE);
    ATrace_endSection();
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