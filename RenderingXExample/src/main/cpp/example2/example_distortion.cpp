//
// Created by Consti10 on 15/05/2019.
//

#include <DistortionCorrection/FileHelper.h>
#include "example_distortion.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include "Helper/GLBufferHelper.hpp"
#include "../HelperX.h"

constexpr auto TAG="DistortionExample";

ExampleRenderer2::ExampleRenderer2(JNIEnv *env, jobject androidContext,gvr_context *gvr_context,jfloatArray undistData) {
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    //gvr_api_->InitializeGl();
    //distortionManager=new DistortionManager(env,undistData);
    //distortionManager=new DistortionManager(gvr_api_->GetContext());
    distortionManager=DistortionManager::createFromFileIfAlreadyExisting("/storage/emulated/0/",gvr_api_->GetContext());
}


void ExampleRenderer2::onSurfaceCreated(JNIEnv *env, jobject context) {
//Instantiate all our OpenGL rendering 'Programs'
    //distortionManager=new DistortionManager(gvr_api_->GetContext());
    //distortionManager=new DistortionManager("","");
    distortionManager->generateTexturesIfNeeded();

    glProgramVC=new GLProgramVC();
    glProgramVC2=new GLProgramVC(distortionManager);
    GLuint texture;
    glGenTextures(1,&texture);
    glProgramTexture=new GLProgramTexture(texture,false,nullptr,true);
    glProgramTexture->loadTexture(env,context,"c_gimp1.png");
    //create all the gl Buffer for later use
    glGenBuffers(1,&glBufferVC);
    glGenBuffers(1,&glBufferVCDistorted1);
    glGenBuffers(1,&glBufferVCDistorted2);
    glGenBuffers(1,&glBufferCoordinateSystemLines);
    //create the geometry for our simple test scene
    float size=1.0f;
    const auto tmp=ColoredGeometry::makeTesselatedColoredRectLines(LINE_MESH_TESSELATION_FACTOR,{-size/2.0f,-size/2.0f,0},size,size,Color::WHITE);
    nColoredVertices=tmp.size();
    GLBufferHelper::allocateGLBufferStatic(glBufferVC,tmp);
    //make the line going trough (0,0)
    const auto coordinateSystemLines=ColoredGeometry::makeDebugCoordinateSystemLines(100);
    GLBufferHelper::allocateGLBufferStatic(glBufferCoordinateSystemLines,coordinateSystemLines);
    nCoordinateSystemLinesVertices=coordinateSystemLines.size();
    //Textured stuff
    //const float fov=90.0f;
    const float sizeX=1.0f;
    const float sizeY=1.0f;
    glGenBuffers(1,&glBufferTextured);
    glGenBuffers(1,&glBufferTexturedLeftEye);
    glGenBuffers(1,&glBufferTexturedRightEye);
    const auto tesselatedVideoCanvas=TexturedGeometry::makeTesselatedVideoCanvas2(glm::vec3(-sizeX/2.0f,-sizeY/2.0f,0),
            sizeX,sizeY, TEXTURE_TESSELATION_FACTOR, 0.0f,1.0f);
    nTexturedVertices=tesselatedVideoCanvas.size();
    GLBufferHelper::allocateGLBufferStatic(glBufferTextured,tesselatedVideoCanvas);

    HelperX::generateDistortionMeshBuffersTEST(tesselatedVideoCanvas,gvr_api_->GetContext(),
                                               glBufferTexturedLeftEye,glBufferTexturedRightEye,glBufferTexturedLeftEye_rgb,glBufferTexturedRightEye_rgb);

    HelperX::debugColorChannelDifferences(gvr_api_->GetContext());

    GLHelper::checkGlError("example_renderer::onSurfaceCreated");
}

void ExampleRenderer2::onSurfaceChanged(int width, int height) {
    ViewPortW=width/2;
    ViewPortH=height;
    projection=glm::perspective(glm::radians(80.0F),((float) ViewPortW)/((float)ViewPortH), MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    //projection=make_frustum(45,45,45,45,MIN_Z_DISTANCE,MAX_Z_DISTANCE);

    glm::vec3 cameraPos   = glm::vec3(0,0,CAMERA_POSITION);
    glm::vec3 cameraFront = glm::vec3(0.0F,0.0F,-1.0F);
    eyeView=glm::lookAt(cameraPos,cameraPos+cameraFront,glm::vec3(0,1,0));
    //eyeView=glm::mat4();
    leftEyeView=glm::translate(eyeView,glm::vec3(-VR_InterpupilaryDistance/2.0f,0,0));
    rightEyeView=glm::translate(eyeView,glm::vec3(VR_InterpupilaryDistance/2.0f,0,0));
}

void ExampleRenderer2::onDrawFrame() {
    glClearColor(0,0,0.0,0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    cpuFrameTime.start();
    drawEye(false);
    drawEye(true);
    GLHelper::checkGlError("example_renderer::onDrawFrame");
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(5000);
    fpsCalculator.tick();
}

void ExampleRenderer2::drawEye(bool leftEye) {
    if(leftEye){
        glViewport(0,0,ViewPortW,ViewPortH);
        distortionManager->leftEye=true;
    }else{
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
        distortionManager->leftEye=false;
    }
    glm::mat4 tmp=leftEye ? leftEyeView : rightEyeView;

    //GLuint buff=whichEye ? glBufferVCDistorted1 : glBufferVCDistorted2;
    /*GLuint buff=glBufferVC;
    glProgramVC->beforeDraw(buff);
    glProgramVC->draw(glm::value_ptr(tmp),glm::value_ptr(projection),0,N_COLORED_VERTICES,GL_TRIANGLES);
    glProgramVC->afterDraw();*/

    /*glProgramTexture->beforeDraw(leftEye ? glBufferTexturedLeftEye : glBufferTexturedRightEye);
    glProgramTexture->draw(eyeView,projection,0,nTexturedVertices);
    glProgramTexture->afterDraw();*/

    const auto* glBufferTextured_rgb=leftEye ? glBufferTexturedLeftEye_rgb : glBufferTexturedRightEye_rgb;
    for(int rgb=0;rgb<3;rgb++){
        glProgramTexture->beforeDraw(glBufferTextured_rgb[rgb]);
        glProgramTexture->setUColorChannel(rgb+1);
        glProgramTexture->draw(eyeView,projection,0,nTexturedVertices);
        glProgramTexture->afterDraw();
    }

    /*distortionManager->leftEye=leftEye;
    glProgramVC2->beforeDraw(glBufferVC);
    glProgramVC2->draw(glm::value_ptr(eyeView),glm::value_ptr(projection),0,nColoredVertices,GL_LINES);
    glProgramVC2->afterDraw();*/

    /*glProgramVC->beforeDraw(glBufferVCDistorted1);
    glProgramVC->draw(glm::value_ptr(eyeView),glm::value_ptr(projection),0,N_COLORED_VERTICES,GL_LINES);
    glProgramVC->afterDraw();*/

    /*glProgramVC->beforeDraw(glBufferVC);
    glProgramVC->draw(glm::value_ptr(eyeView),glm::value_ptr(projection),0,N_COLORED_VERTICES,GL_LINES);
    glProgramVC->afterDraw();*/

    /*glProgramVC2->beforeDraw(glBufferCoordinateSystemLines);
    glProgramVC2->draw(glm::value_ptr(eyeView),glm::value_ptr(projection),0,nCoordinateSystemLinesVertices,GL_LINES);
    glProgramVC2->draw(glm::value_ptr(eyeView),glm::value_ptr(projection),0,nCoordinateSystemLinesVertices,GL_POINTS);
    glProgramVC2->afterDraw();*/
}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_renderer2_GLRTest_##method_name

inline jlong jptr(ExampleRenderer2 *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline ExampleRenderer2 *native(jlong ptr) {
    return reinterpret_cast<ExampleRenderer2*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jfloatArray undistortionData,jlong native_gvr_api) {
    return jptr(new ExampleRenderer2(env,androidContext,reinterpret_cast<gvr_context *>(native_gvr_api),undistortionData));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
    delete native(p);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong p,jobject androidContext) {
    native(p)->onSurfaceCreated(env,androidContext);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj,jlong p,jint width,jint height) {
    native(p)->onSurfaceChanged((int)width,(int)height);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onDrawFrame();
}

}
