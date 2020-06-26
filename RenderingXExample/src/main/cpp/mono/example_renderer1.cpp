#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by Consti10 on 15/05/2019.
//

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <GLProgramVC.h>
#include <GLProgramText.h>
#include <ColoredGeometry.hpp>
#include <TexturedGeometry.hpp>
#include <TextAssetsHelper.hpp>
#include <GLBuffer.hpp>
#include <GLProgramLine.h>
#include <GLProgramTexture.h>
#include <Chronometer.h>
#include <FPSCalculator.h>
#include <ProjTex/GLPTextureProj.h>
#include "../../../../../RenderingXCore/src/main/cpp/GLPrograms/GLProgramVC.h"

//Render a simple scene consisting of a colored triangle, smooth lines and smooth text

//Camera/Projection matrix constants
constexpr float MAX_Z_DISTANCE=20.0f;
constexpr float MIN_Z_DISTANCE=0.05f;
constexpr float CAMERA_DISTANCE=5.0f;
constexpr float TEXT_Y_OFFSET=-1.0f;

constexpr float DEFAULT_CAMERA_Z=10.0f;
constexpr auto DEFAULT_CAMERA_POSITION=glm::vec3(0,0,DEFAULT_CAMERA_Z);
constexpr auto DEFAULT_CAMERA_LOOK_AT=glm::vec3(0,0,0);
constexpr auto DEFAULT_CAMERA_UP=glm::vec3(0,1,0);
const auto DEFAULT_EYE_VIEW=glm::lookAt(DEFAULT_CAMERA_POSITION,DEFAULT_CAMERA_LOOK_AT,DEFAULT_CAMERA_UP);
float currentCameraScale=1.0f;
glm::vec2 currentCameraMovement=glm::vec2(0.0f,0.0f);
//
int currentRenderingMode;

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
//Used to render textured geometry
GLProgramTexture* glProgramTexture;
GLuint mTextureBrick;
GLuint mTextureMonaLisa;
GLPTextureProj* glProgramTextureProj;

//holds colored geometry vertices
GLProgramVC::Mesh mMeshColoredGeometry;
//holds text vertices
VertexBuffer glBufferText;
//holds icon vertices (also interpreted as text)
VertexBuffer glBufferIcons;
//holds smooth line vertices
VertexBuffer glBufferLine;
//holds textured vertices
VertexBuffer glBufferTextured;
VertexBuffer glBufferPyramid;
const glm::mat4 DEFAULT_MODEL_MATRIX=glm::scale(glm::mat4(1.0f), glm::vec3(1.0,1.0,1.0));
glm::mat4 modelM;

//simplifies debugging/benchmarking
Chronometer cpuFrameTime{"CPU frame time"};
FPSCalculator fpsCalculator{"OpenGL FPS",2000}; //print every 2 seconds

constexpr auto EXAMPLE_TEXT=L"HELLO,World";
constexpr int EXAMPLE_TEXT_LENGTH=11;
constexpr int EXAMPLE_TEXT_N_LINES=4;
constexpr int N_LINES=10*2;
constexpr int N_ICONS=8;
constexpr auto LOREM_IPSUM=L"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                           "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                           "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                           "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
float seekBarValue1=20.0f;
float seekBarValue2=10.0f;
float seekBarValue3=10.0f;


static void onSurfaceCreated(JNIEnv* env,jobject context){
    TEST_LOGGING_ON_ANDROID::test2();
    //Instantiate all our OpenGL rendering 'Programs'
    glProgramVC=new GLProgramVC();
    glProgramLine=new GLProgramLine();
    glProgramText=new GLProgramText();
    glProgramText->loadTextRenderingData(env,context,TextAssetsHelper::ARIAL_PLAIN);
    glProgramTexture=new GLProgramTexture(false);
    glProgramTextureProj=new GLPTextureProj();
    glGenTextures(1,&mTextureBrick);
    GLProgramTexture::loadTexture(mTextureBrick,env,context,"ExampleTexture/brick_wall_simple.png");
    GLProgramTexture::loadTexture(mTextureMonaLisa, env, context, "ExampleTexture/mona_lisa.jpg");

    //create all the gl Buffer for later use
    //create the geometry for our simple test scene
    //some colored geometry
    mMeshColoredGeometry=GLProgramVC::Mesh(ColoredGeometry::makeTessellatedColoredRect(12, glm::vec3(0, 0, 0), {5.0, 5.0}, TrueColor2::RED)
            , GL_TRIANGLES);
    mMeshColoredGeometry.uploadGL();
    //glBufferVC.uploadGL(ColoredGeometry::makeTessellatedColoredRect(12, glm::vec3(0,0,0), {5.0,5.0}, TrueColor2::RED)
    //        ,GL_TRIANGLES);
    //some smooth lines
    //create 5 lines of increasing stroke width
    std::vector<GLProgramLine::Vertex> lines(N_LINES*GLProgramLine::VERTICES_PER_LINE);
    const float lineLength=4;
    float yOffset=TEXT_Y_OFFSET;
    for(int i=0;i<N_LINES;i++){
        const float strokeWidth=(i%10)*0.01F;
        auto baseColor= i >= N_LINES / 2 ? TrueColor2::YELLOW : TrueColor2::BLUE;
        auto outlineColor= i >= N_LINES / 2 ? TrueColor2::RED : TrueColor2::WHITE;
        yOffset+=strokeWidth*3;
        GLProgramLine::convertLineToRenderingData({-lineLength/2,yOffset,0},{lineLength/2,yOffset,0},strokeWidth,lines.data(),i*GLProgramLine::VERTICES_PER_LINE,
                baseColor,outlineColor);
    }
    glBufferLine.uploadGL(lines);
    //some smooth characters
    std::vector<GLProgramText::Character> charactersAsVertices(EXAMPLE_TEXT_LENGTH*EXAMPLE_TEXT_N_LINES);
    yOffset=TEXT_Y_OFFSET;
    float textHeight=0.5f;
    for(int i=0;i<EXAMPLE_TEXT_N_LINES;i++){
        float textLength=GLProgramText::getStringLength({EXAMPLE_TEXT},textHeight);
        GLProgramText::convertStringToRenderingData(-textLength/2.0f, yOffset, 0, textHeight, {EXAMPLE_TEXT}, TrueColor2::YELLOW, charactersAsVertices.data(), i * EXAMPLE_TEXT_LENGTH);
        yOffset+=textHeight;
        textHeight+=0.3;
    }
    glBufferText.uploadGL(charactersAsVertices);
    //some icons
    std::vector<GLProgramText::Character> iconsAsVertices(N_ICONS);
    yOffset=TEXT_Y_OFFSET;
    for(int i=0;i<N_ICONS;i++){
        const float textHeight=0.8F;
        GLProgramText::convertStringToRenderingData(0, yOffset, 0, textHeight, {(wchar_t)GLProgramText::ICONS_OFFSET+i},
                                                    TrueColor2::YELLOW, iconsAsVertices.data(), i);
        yOffset+=textHeight;
    }
    glBufferIcons.uploadGL(iconsAsVertices);
    //Textured geometry
    const float wh=5.0f;
    glBufferTextured.uploadGL(TexturedGeometry::makeTesselatedVideoCanvas2(10,glm::vec3(0,0,0),{wh,wh},0.0f,1.0f));
    {
        const auto pyramid=TexturedGeometry::makePyramid();
        const auto modelMatrix= glm::translate(glm::vec3(0,-1,0))*
                glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
                glm::scale(glm::vec3(wh,wh,1));
        const auto plane=TexturedGeometry::makeTesselatedVideoCanvas2(10,modelMatrix);
        auto pyramidAndPlane=pyramid;
        for(const auto v:plane){
            pyramidAndPlane.push_back(v);
        }
        glBufferPyramid.uploadGL(pyramidAndPlane,GL_TRIANGLES);
    }
    modelM=DEFAULT_MODEL_MATRIX;

    //const auto lol=TexturedGeometry::makeTesselatedVideoCanvas(glm::vec3(-wh*0.5f,-wh*0.5f,0),wh,wh,10,0.0f,1.0f);
    //glBufferTextured2.initializeAndUploadGL(lol.first,lol.second);
    GLHelper::checkGlError("example_renderer::onSurfaceCreated");
}

static void updateCamera(){
    eyeView=DEFAULT_EYE_VIEW;
    //eyeView=glm::rotate(eyeView,glm::radians(60.0f),glm::vec3(1,0,0));
    eyeView=glm::scale(eyeView,glm::vec3(currentCameraScale,currentCameraScale,currentCameraScale));
    eyeView=glm::translate(eyeView,{currentCameraMovement.x,currentCameraMovement.y,0.0f});

    //modelM=glm::rotate(modelM,glm::radians(1.0f),glm::vec3(1,0,0));
}

//recalculate matrices
static void onSurfaceChanged(int width, int height){
    projection=glm::perspective(glm::radians(45.0F), (float)width/height, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    currentCameraScale=1.0f;
    currentCameraMovement=glm::vec2(0,0);
    updateCamera();
    glViewport(0,0,width,height);
}

glm::mat4 buildProjectorMatrices() {

    // Construct a view-projection matrix as if the camera is placed at the projector.
    // If we use this view-projection matrix to transform the object, the object will
    // be projected from the projector to a 2D space, which is the texture space
    // for the projector view. The texture coordinates in this 2D space are the
    // texture coordinates for the projective texture.

    glm::vec3 projectorPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 projectorLookAtPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 projectorUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
    float projectorFOV = 40.0f;

    // The view matrix from the projector's viewpoint.
    glm::mat4 projectorViewMatrix = glm::lookAt(projectorPosition,
                                      projectorLookAtPosition, projectorUpVector);
    projectorViewMatrix=glm::rotate(projectorViewMatrix,glm::radians(30.0f),glm::vec3(1,0,0));

    // The projection matrix for the projector.
    glm::mat4 projectorProjectionMatrix = glm::perspective(glm::radians(projectorFOV), 1.0f, 0.1f, 10.0f);
    // mat4 projectorProjectionMatrix = ortho(0.3f, 0.3f, 0.3f, 0.0f, 0.5f, 10.0f);

    // After the initial projection, the origin is at the center of the window.
    // However, the origin of a texture image is at the lower left corner.
    // The scale and bias matrix is used to transform the origin from the center
    // to the corner.
    glm::mat4 scaleBiasMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(0.5f));

    // Construct the combined matrix to transform the vertex position to its projective
    // texture coordiantes.
    glm::mat4 projectorTransformMatrix = scaleBiasMatrix * projectorProjectionMatrix *
                                    projectorViewMatrix;

    return projectorTransformMatrix;
}

//draw a frame
//mode selects which elements to draw
static void onDrawFrame(){
    glClearColor(0,0,0.2,0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    //
    updateCamera();
    if(currentRenderingMode==5){
        glEnable(GL_DEPTH_TEST);
    }else{
        GLProgramText::setupDepthTestAndBlending();
    }
    //Drawing with the OpenGL Programs is easy - call beforeDraw() with the right OpenGL Buffer and then draw until done
    if(currentRenderingMode==0){ //Smooth text
        glProgramText->beforeDraw(glBufferText.vertexB);
        glProgramText->updateOutline(TrueColor2::RED, seekBarValue1 / 100.0f);
        glProgramText->setOtherUniforms(seekBarValue2/100.0f,seekBarValue3/100.0f);
        glProgramText->draw(projection*eyeView,0,glBufferText.nVertices*GLProgramText::INDICES_PER_CHARACTER);
        glProgramText->afterDraw();
    } else if(currentRenderingMode==1){
        glProgramText->beforeDraw(glBufferIcons.vertexB);
        glProgramText->updateOutline(TrueColor2::RED, seekBarValue1 / 100.0f);
        glProgramText->setOtherUniforms(seekBarValue2/100.0f,seekBarValue3/100.0f);
        glProgramText->draw(projection*eyeView,0,glBufferIcons.nVertices*GLProgramText::INDICES_PER_CHARACTER);
        glProgramText->afterDraw();
    }else if(currentRenderingMode==2){
        glProgramLine->beforeDraw(glBufferLine.vertexB);
        glProgramLine->setOtherUniforms(seekBarValue1/100.0F,seekBarValue2/100.0F,seekBarValue3/100.0F);
        glProgramLine->draw(eyeView,projection,0,glBufferLine.nVertices);
        glProgramLine->afterDraw();
    }else if(currentRenderingMode==3){
        glProgramVC->drawX(eyeView, projection, mMeshColoredGeometry);
    }else if(currentRenderingMode==4){
         glProgramTexture->drawX(mTextureBrick, eyeView, projection, glBufferTextured);
    }else if(currentRenderingMode==5){
        /*glProgramTextureProj->beforeDraw(glBufferPyramid.vertexB, mTextureMonaLisa);
        glProgramTextureProj->draw(modelM,eyeView,projection,0,glBufferPyramid.nVertices,glBufferPyramid.mMode);
        glProgramTextureProj->updateTexMatrix(buildProjectorMatrices());
        glProgramTextureProj->afterDraw();*/
        glProgramTextureProj->beforeDraw(glBufferTextured.vertexB, mTextureMonaLisa);
        glProgramTextureProj->draw(modelM,eyeView,projection,0,glBufferTextured.nVertices,glBufferTextured.mMode);
        glProgramTextureProj->updateTexMatrix(buildProjectorMatrices());
        glProgramTextureProj->afterDraw();
    }
    GLHelper::checkGlError("example_renderer::onDrawFrame");
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(std::chrono::seconds(5));
    fpsCalculator.tick();
}



#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_mono_GLRExample_##method_name


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

// Scale is value between 0..1 for example scale=0.9f
JNI_METHOD(void, nativeScale)
(JNIEnv *env, jobject obj,jfloat scale) {
    currentCameraScale*=scale;
}
JNI_METHOD(void, nativeMove)
(JNIEnv *env, jobject obj,jfloat moveX,float moveY) {
    currentCameraMovement.x+=moveX*3;
    currentCameraMovement.y+=-moveY*3;
}
JNI_METHOD(void, nativeSetRenderingMode)
(JNIEnv *env, jobject obj,int renderingMode) {
    currentRenderingMode=renderingMode;
}

JNI_METHOD(void, nativeSetSeekBarValues)
(JNIEnv *env, jobject obj,jfloat val1,jfloat val2,jfloat val3) {
    seekBarValue1=val1;
    seekBarValue2=val2;
    seekBarValue3=val3;
}

}

#pragma clang diagnostic pop