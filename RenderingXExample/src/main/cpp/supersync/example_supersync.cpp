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
#include <Extensions.hpp>

#include "GLProgramLine.h"
#include "FPSCalculator.h"
#include "VRFrameCPUChronometer.h"
#include "example_supersync.h"

//TODO: use better geometry

GLRSuperSyncExample::GLRSuperSyncExample(JNIEnv *env, jobject androidContext,
                                       bool qcomTiledRenderingAvailable,
                                       bool reusableSyncAvailable):
        mFrameTimeAcc(std::vector<std::string>{"startDR","drawEye","stopDR"}){
    std::function<void(JNIEnv *env2, bool leftEye)> f = [this](JNIEnv *env2, bool leftEye) {
        this->renderNewEyeCallback(env2,leftEye,0);
    };
    mFBRManager=std::make_unique<FBRManager>(qcomTiledRenderingAvailable,reusableSyncAvailable,f);
}

void GLRSuperSyncExample::onSurfaceCreated(JNIEnv *env,jobject androidContext,int width, int height) {
    Extensions::initialize();
    glProgramVC=new GLProgramVC();
    mFrameTimeAcc.reset();
    eyeView=glm::lookAt(glm::vec3(0,0,20),glm::vec3(0.0F,0.0F,-1.0F),glm::vec3(0,1,0));
    const float IPD=0.2f;
    leftEyeView=glm::translate(eyeView,glm::vec3(-IPD/2.0f,0,0));
    rightEyeView=glm::translate(eyeView,glm::vec3(IPD/2.0f,0,0));
    //some colored geometry
    std::vector<ColoredVertex> coloredVertices(N_COLOR_VERTICES);
    const float triangleWidth=3.0F;
    for(int i=0;i<N_TRIANGLES;i++){
        ColoredGeometry::makeColoredTriangle1(&coloredVertices[i*3], glm::vec3(-triangleWidth/2,0,0), triangleWidth, triangleWidth, TrueColor2::BLUE);
    }
    mVertexBufferVC.setData({coloredVertices, GL_TRIANGLES});
    const float cbs=20.0f;
    solidRectangleBlack.setData(
            ColoredGeometry::makeTessellatedColoredRect(10, {0,0,0}, {cbs,cbs}, TrueColor2::BLACK));
    solidRectangleYellow.setData(
            ColoredGeometry::makeTessellatedColoredRect(10, {0,0,0}, {cbs,cbs}, TrueColor2::YELLOW));

    // XX
    SCREEN_W=width;
    SCREEN_H=height;
    ViewPortW=width/2;
    ViewPortH=height;
    projection=glm::perspective(glm::radians(45.0F),((float) ViewPortW)/((float)ViewPortH), 0.05f, 20.0f);
}


void GLRSuperSyncExample::enterSuperSyncLoop(JNIEnv *env, jobject obj,int exclusiveVRCore) {
    LOLX::setAffinity(exclusiveVRCore);
    mFBRManager->enterDirectRenderingLoop(env,SCREEN_W,SCREEN_H);
    /*glScissor(0,0,SCREEN_W,SCREEN_H);
    glViewport(0,0,SCREEN_W,SCREEN_H);
    glClearColor(0,1,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();*/
}

void GLRSuperSyncExample::setLastVSYNC(int64_t lastVSYNC) {
    mFBRManager->setVSYNCSentByChoreographer(lastVSYNC);
}

void GLRSuperSyncExample::renderNewEyeCallback(JNIEnv *env,const bool leftEye,const int64_t offsetNS) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5F,0.5F,0.5F,0.0F);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    drawEye(env,leftEye);
}

void GLRSuperSyncExample::drawEye(JNIEnv *env, bool leftEye) {
    //Draw the background, which alternates between black and yellow to make tearing observable
    const int idx=leftEye==0 ? 0 : 1;
    whichColor[idx]++;
    if(whichColor[idx]>1){
        whichColor[idx]=0;
    }
    if(whichColor[idx]==0){
        glProgramVC->drawX(leftEyeView,projection,solidRectangleBlack);
    }else{
        glProgramVC->drawX(leftEyeView,projection,solidRectangleYellow);
    }
    //A typical application has way more than 1 draw call only
    for(int i=0;i<N_DRAW_CALLS;i++){
        const glm::mat4 leftOrRightEyeView= leftEye==0 ? leftEyeView : rightEyeView;
        glProgramVC->drawX(leftOrRightEyeView,projection,mVertexBufferVC);
    }
}

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_supersync_GLRExampleSuperSync_##method_name

inline jlong jptr(GLRSuperSyncExample *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline GLRSuperSyncExample *native(jlong ptr) {
    return reinterpret_cast<GLRSuperSyncExample *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jboolean qcomTiledRenderingAvailable,jboolean reusableSyncAvailable) {
    return jptr(new GLRSuperSyncExample(env,androidContext,(bool)qcomTiledRenderingAvailable,(bool)reusableSyncAvailable));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject androidContext,jint width, jint height) {
    native(glRendererStereo)->onSurfaceCreated(env,androidContext,width,height);
}
JNI_METHOD(void, nativeEnterSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint exclusiveVRCore) {
    native(glRendererStereo)->enterSuperSyncLoop(env,obj,(int)exclusiveVRCore);
}
JNI_METHOD(void, nativeDoFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jlong lastVSYNC) {
    native(glRendererStereo)->setLastVSYNC((int64_t) lastVSYNC);
}

}