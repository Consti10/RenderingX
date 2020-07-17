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
#include "Chronometer.h"
#include "FPSCalculator.h"
#include "VRFrameCPUChronometer.h"
#include "example_supersync.h"

//TODO: use better geometry

GLRSuperSyncExample::GLRSuperSyncExample(JNIEnv *env, jobject androidContext,
                                       bool qcomTiledRenderingAvailable,
                                       bool reusableSyncAvailable):
        mFrameTimeAcc(std::vector<std::string>{"startDR","drawEye","stopDR"}){
    std::function<void(JNIEnv *env2, bool whichEye, int64_t offsetNS)> f = [this](JNIEnv *env2, bool whichEye, int64_t offsetNS) {
        this->renderNewEyeCallback(env2,whichEye,offsetNS);
    };
    mFBRManager=std::make_unique<FBRManager>(qcomTiledRenderingAvailable,reusableSyncAvailable,f, nullptr);
}

void GLRSuperSyncExample::onSurfaceCreated(JNIEnv *env,jobject androidContext) {
    //mBasicGLPrograms=std::make_unique<BasicGLPrograms>(false,nullptr);
    //mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,TextAssetsHelper::ARIAL_BOLD);
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
}


void GLRSuperSyncExample::onSurfaceChanged(int width, int height) {
    ViewPortW=width/2;
    ViewPortH=height;
    projection=glm::perspective(glm::radians(45.0F),((float) ViewPortW)/((float)ViewPortH), 0.05f, 20.0f);
}

void GLRSuperSyncExample::enterSuperSyncLoop(JNIEnv *env, jobject obj,int exclusiveVRCore) {
    //Extensions::setAffinity(exclusiveVRCore);
    MLOGD<<"entering superSync loop. GLThread will be blocked";
    mFBRManager->enterDirectRenderingLoop(env);
    MLOGD<<"exited superSync loop. GLThread unblocked";
}

void GLRSuperSyncExample::exitSuperSyncLoop() {
    mFBRManager->requestExitSuperSyncLoop();
}

void GLRSuperSyncExample::setLastVSYNC(int64_t lastVSYNC) {
    mFBRManager->setLastVSYNC(lastVSYNC);
}

void GLRSuperSyncExample::renderNewEyeCallback(JNIEnv *env,const bool whichEye,const int64_t offsetNS) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5F,0.5F,0.5F,0.0F);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    VREyeDurations vrEyeTimeStamps{whichEye};
    mFBRManager->startDirectRendering(whichEye,ViewPortW,ViewPortH);
    if(mFBRManager->directRenderingMode==FBRManager::QCOM_TILED_RENDERING){
        //so we have to call glClear() before any OpenGL calls that affect framebuffer contents (e.g. draw())
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    vrEyeTimeStamps.setTimestamp("startDR");
    drawEye(env,whichEye);
    vrEyeTimeStamps.setTimestamp("drawEye");
    mFBRManager->stopDirectRendering(whichEye);
    vrEyeTimeStamps.setTimestamp("stopDR");
    //
    //vrEyeTimeStamps.print();
    mFrameTimeAcc.add(vrEyeTimeStamps);
    mFrameTimeAcc.printEveryXSeconds(5);
}

void GLRSuperSyncExample::drawEye(JNIEnv *env, bool whichEye) {
    //Draw the background, which alternates between black and yellow to make tearing observable
    const int idx=whichEye==0 ? 0 : 1;
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
    for(int i=0;i<20;i++){
        const glm::mat4 leftOrRightEyeView= whichEye==0 ? leftEyeView : rightEyeView;
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
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject androidContext) {
    native(glRendererStereo)->onSurfaceCreated(env,androidContext);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->onSurfaceChanged(w, h);
}
JNI_METHOD(void, nativeEnterSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint exclusiveVRCore) {
    MLOGD<<"nativeEnterSuperSyncLoop()";
    native(glRendererStereo)->enterSuperSyncLoop(env,obj,(int)exclusiveVRCore);
}
JNI_METHOD(void, nativeExitSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    MLOGD<<"nativeExitSuperSyncLoop()";
    native(glRendererStereo)->exitSuperSyncLoop();
}
JNI_METHOD(void, nativeDoFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jlong lastVSYNC) {
    native(glRendererStereo)->setLastVSYNC((int64_t) lastVSYNC);
    //LOGD("nativeDoFrame");
}

}