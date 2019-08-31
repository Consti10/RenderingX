//
// Created by Consti10 on 15/05/2019.
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
#include <Helper/ColoredGeometry.hpp>
#include <TextAssetsHelper.hpp>

#include "GLProgramLine.h"
#include "Time/Chronometer.h"
#include "Time/FPSCalculator.h"

//Render a simple scene consisting of a colored triangle, smooth lines and smooth text

//Camera/Projection matrix constants
constexpr float MAX_Z_DISTANCE=20.0f;
constexpr float MIN_Z_DISTANCE=0.05f;
constexpr float CAMERA_DISTANCE=5.0f;

//the projection matrix. Should be re calculated in each onSurfaceChanged with new width and height
glm::mat4x4 projection;
//the view matrix, without any IPD
glm::mat4x4 eyeView;
//used to render vertex-color geometry
GLProgramVC* glProgramVC;
//used to render smooth lines
GLProgramLine* glProgramLine;
//used to render smooth text
GLProgramText* glProgramText;

//holds colored geometry vertices
GLuint glBufferVC;
//holds text vertices
GLuint glBufferText;
//holds smooth line vertices
GLuint glBufferLine;

//simplifies debugging/benchmarking
Chronometer cpuFrameTime{"CPU frame time"};
FPSCalculator fpsCalculator{"OpenGL FPS",2000}; //print every 2 seconds

constexpr auto EXAMPLE_TEXT=L"HELLO,World";
constexpr int EXAMPLE_TEXT_LENGTH=5+1+5;
constexpr int N_LINES=10;
constexpr auto LOREM_IPSUM=L"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                           "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                           "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                           "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";


static void onSurfaceCreated(JNIEnv* env,jobject context){
    //Instantiate all our OpenGL rendering 'Programs'
    glProgramVC=new GLProgramVC();
    glProgramLine=new GLProgramLine();
    glProgramText=new GLProgramText();
    glProgramText->loadTextRenderingData(env,context,TextAssetsHelper::ARIAL_PLAIN);
    //create all the gl Buffer for later use
    glGenBuffers(1,&glBufferVC);
    glGenBuffers(1,&glBufferText);
    glGenBuffers(1,&glBufferLine);
    //create the geometry for our simple test scene
    //some colored geometry
    GLProgramVC::Vertex coloredVertices[3];
    ColoredGeometry::makeColoredTriangle1(coloredVertices,glm::vec3(0,0,0),5.0f,5.0f,Color::RED);
    GLHelper::allocateGLBufferStatic(glBufferVC,coloredVertices,3*sizeof(GLProgramVC::Vertex));
    //some smooth lines
    //create 5 lines of increasing stroke width
    GLProgramLine::Vertex lines[N_LINES*GLProgramLine::VERTICES_PER_LINE];
    const float lineLength=1;
    float yOffset=0;
    for(int i=0;i<N_LINES;i++){
        const float strokeWidth=i*0.01F;
        yOffset+=strokeWidth*3;
        GLProgramLine::convertLineToRenderingData({-lineLength,yOffset,0},{0,yOffset,0},strokeWidth,lines,i*GLProgramLine::VERTICES_PER_LINE,Color::GREEN,Color::WHITE);
    }
    GLHelper::allocateGLBufferStatic(glBufferLine,lines,sizeof(lines));
    //some smooth characters
    GLProgramText::Character charactersAsVertices[EXAMPLE_TEXT_LENGTH];
    GLProgramText::convertStringToRenderingData(0,0,0,1,{EXAMPLE_TEXT},Color::YELLOW,charactersAsVertices,0);
    GLHelper::allocateGLBufferStatic(glBufferText,charactersAsVertices,sizeof(charactersAsVertices));
    GLHelper::checkGlError("example_renderer::onSurfaceCreated");
}

//recalculate matrices
static void onSurfaceChanged(int width, int height){
    projection=glm::perspective(glm::radians(45.0F), (float)width/height, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    eyeView=glm::lookAt(
            glm::vec3(0,0,CAMERA_DISTANCE),
            glm::vec3(0,0,-1),
            glm::vec3(0,1,0)
    );
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,width,height);
}

//draw a frame
static void onDrawFrame(){
    glClearColor(0,0,0.2,0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    //Drawing with the OpenGL Programs is easy - call beforeDraw() with the right OpenGL Buffer and then draw until done
    glProgramVC->beforeDraw(glBufferVC);
    glProgramVC->draw(glm::value_ptr(eyeView),glm::value_ptr(projection),0,3,GL_TRIANGLES);
    glProgramVC->afterDraw();
    glProgramText->beforeDraw(glBufferText);
    glProgramText->draw(eyeView,projection,0,EXAMPLE_TEXT_LENGTH*GLProgramText::VERTICES_PER_CHARACTER);
    glProgramText->afterDraw();
    glProgramLine->beforeDraw(glBufferLine);
    glProgramLine->draw(eyeView,projection,0,N_LINES*GLProgramLine::VERTICES_PER_LINE);
    GLHelper::checkGlError("example_renderer::onDrawFrame");
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(5000);
    fpsCalculator.tick();
}

static void moveCamera(float distance,float x,float y){
    glm::vec3 cameraPos   = glm::vec3(x,y,MAX_Z_DISTANCE-distance);
    glm::vec3 cameraFront = glm::vec3(0.0F,0.0F,-1.0F);
    eyeView=glm::lookAt(cameraPos,cameraPos+cameraFront,glm::vec3(0,1,0));
    //LOGD("move %f %f %f",distance,x,y);
    //eyeView=glm::translate(eyeView,glm::vec3(x,y,distance));
}

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_GLRExample_##method_name


extern "C" {


JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jobject androidContext) {
    onSurfaceCreated(env,androidContext);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj,jint width,jint height) {
    onSurfaceChanged((int)width,(int)height);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj) {
    onDrawFrame();
}

JNI_METHOD(void, nativeMoveCamera)
(JNIEnv *env, jobject obj,jfloat scale,jfloat x,jfloat y) {
   moveCamera((float)scale,(float)x,(float)y);
}


}
