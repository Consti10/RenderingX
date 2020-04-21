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
#include <GLBufferHelper.hpp>
#include <GLProgramLine.h>
#include <GLProgramTexture.h>
#include <Chronometer.h>
#include <FPSCalculator.h>
#include <GLProgramTextureProj.h>

//Render a simple scene consisting of a colored triangle, smooth lines and smooth text

//Camera/Projection matrix constants
constexpr float MAX_Z_DISTANCE=20.0f;
constexpr float MIN_Z_DISTANCE=0.05f;
constexpr float CAMERA_DISTANCE=5.0f;
constexpr float TEXT_Y_OFFSET=-1.0f;

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
GLuint mExampleTexture;
GLProgramTextureProj* glProgramTextureProj;

//holds colored geometry vertices
VertexBuffer glBufferVC;
//holds text vertices
VertexBuffer glBufferText;
//holds icon vertices (also interpreted as text)
VertexBuffer glBufferIcons;
//holds smooth line vertices
VertexBuffer glBufferLine;
//holds textured vertices
VertexBuffer glBufferTextured;
VertexBuffer glBufferPyramid;
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
    //Instantiate all our OpenGL rendering 'Programs'
    glProgramVC=new GLProgramVC();
    glProgramLine=new GLProgramLine();
    glProgramText=new GLProgramText();
    glProgramText->loadTextRenderingData(env,context,TextAssetsHelper::ARIAL_PLAIN);
    glProgramTexture=new GLProgramTexture(false);
    glProgramTextureProj=new GLProgramTextureProj();
    glGenTextures(1,&mExampleTexture);
    GLProgramTexture::loadTexture(mExampleTexture,env,context,"ExampleTexture/brick_wall_simple.png");


    //create all the gl Buffer for later use
    glBufferVC.initializeGL();
    glBufferText.initializeGL();
    glBufferLine.initializeGL();
    glBufferIcons.initializeGL();
    glBufferTextured.initializeGL();
    //create the geometry for our simple test scene
    //some colored geometry
    const float rectangleWidth=10.0F;
    glBufferVC.uploadGL(ColoredGeometry::makeTessellatedColoredRect(12,glm::vec3(-rectangleWidth/2,-rectangleWidth/2,0),rectangleWidth,rectangleWidth,Color::RED)
            ,GL_TRIANGLES);
    //some smooth lines
    //create 5 lines of increasing stroke width
    std::vector<GLProgramLine::Vertex> lines(N_LINES*GLProgramLine::VERTICES_PER_LINE);
    const float lineLength=4;
    float yOffset=TEXT_Y_OFFSET;
    for(int i=0;i<N_LINES;i++){
        const float strokeWidth=(i%10)*0.01F;
        TrueColor baseColor=i>=N_LINES/2 ? Color::YELLOW : Color::BLUE;
        TrueColor outlineColor=i>=N_LINES/2 ? Color::RED : Color::WHITE;
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
        GLProgramText::convertStringToRenderingData(-textLength/2.0f,yOffset,0,textHeight,{EXAMPLE_TEXT},Color::YELLOW,charactersAsVertices.data(),i*EXAMPLE_TEXT_LENGTH);
        yOffset+=textHeight;
        textHeight+=0.3;
    }
    glBufferText.uploadGL(charactersAsVertices);
    //some icons
    std::vector<GLProgramText::Character> iconsAsVertices(N_ICONS);
    yOffset=TEXT_Y_OFFSET;
    for(int i=0;i<N_ICONS;i++){
        const float textHeight=0.8F;
        GLProgramText::convertStringToRenderingData(0,yOffset,0,textHeight,{(wchar_t)GLProgramText::ICONS_OFFSET+i},
                Color::YELLOW,iconsAsVertices.data(),i);
        yOffset+=textHeight;
    }
    glBufferIcons.uploadGL(iconsAsVertices);
    //Textured geometry
    const float wh=5.0f;
    glBufferTextured.uploadGL(TexturedGeometry::makeTesselatedVideoCanvas2(glm::vec3(-wh*0.5f,-wh*0.5f,0),wh,wh,10,0.0f,1.0f));

    glBufferPyramid.initializeAndUploadGL(TexturedGeometry::makePyramid(),GL_TRIANGLES);
    //modelM=glm::mat4(1.0);
    const float scale=4.0f;
    modelM=glm::scale(glm::mat4(1.0f), glm::vec3(scale,scale,scale));

    //const auto lol=TexturedGeometry::makeTesselatedVideoCanvas(glm::vec3(-wh*0.5f,-wh*0.5f,0),wh,wh,10,0.0f,1.0f);
    //glBufferTextured2.initializeAndUploadGL(lol.first,lol.second);
    GLHelper::checkGlError("example_renderer::onSurfaceCreated");
}

static void placeCamera(float distance, float x, float y){
    glm::vec3 cameraPos   = glm::vec3(x,y,MAX_Z_DISTANCE-distance);
    glm::vec3 cameraFront = glm::vec3(0.0F,0.0F,-1.0F);
    eyeView=glm::lookAt(cameraPos,cameraPos+cameraFront,glm::vec3(0,1,0));
    //LOGD("move %f %f %f",distance,x,y);
    //eyeView=glm::translate(eyeView,glm::vec3(x,y,distance));
}

//recalculate matrices
static void onSurfaceChanged(int width, int height){
    projection=glm::perspective(glm::radians(45.0F), (float)width/height, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    placeCamera(0,0,0);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,width,height);
    // P
    glEnable(GL_DEPTH_TEST);
}

glm::mat4 buildProjectorMatrices() {

    // Construct a view-projection matrix as if the camera is placed at the projector.
    // If we use this view-projection matrix to transform the object, the object will
    // be projected from the projector to a 2D space, which is the texture space
    // for the projector view. The texture coordinates in this 2D space are the
    // texture coordinates for the projective texture.

    glm::vec3 projectorPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 projectorLookAtPosition = glm::vec3(0.5f, 0.0f, 0.0f);
    glm::vec3 projectorUpVector = glm::vec3(0.0f, 0.0f, 1.0f);
    float projectorFOV = 60.0f;

    // The view matrix from the projector's viewpoint.
    glm::mat4 projectorViewMatrix = glm::lookAt(projectorPosition,
                                      projectorLookAtPosition, projectorUpVector);

    // The projection matrix for the projector.
    glm::mat4 projectorProjectionMatrix = glm::perspective(projectorFOV, 1.0f, 0.5f, 10.0f);
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
static void onDrawFrame(int mode){
    glClearColor(0,0,0.2,0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    //
    //Drawing with the OpenGL Programs is easy - call beforeDraw() with the right OpenGL Buffer and then draw until done
    if(mode==0){ //Smooth text
        glProgramText->beforeDraw(glBufferText.vertexB);
        glProgramText->updateOutline(Color::toRGBA(Color::RED),seekBarValue1/100.0f);
        glProgramText->setOtherUniforms(seekBarValue2/100.0f,seekBarValue3/100.0f);
        glProgramText->draw(eyeView,projection,0,glBufferText.nVertices*GLProgramText::INDICES_PER_CHARACTER);
        glProgramText->afterDraw();
    } else if(mode==1){
        glProgramText->beforeDraw(glBufferIcons.vertexB);
        glProgramText->updateOutline(Color::toRGBA(Color::RED),seekBarValue1/100.0f);
        glProgramText->setOtherUniforms(seekBarValue2/100.0f,seekBarValue3/100.0f);
        glProgramText->draw(eyeView,projection,0,glBufferIcons.nVertices*GLProgramText::INDICES_PER_CHARACTER);
        glProgramText->afterDraw();
    }else if(mode==2){
        glProgramLine->beforeDraw(glBufferLine.vertexB);
        glProgramLine->setOtherUniforms(seekBarValue1/100.0F,seekBarValue2/100.0F,seekBarValue3/100.0F);
        glProgramLine->draw(eyeView,projection,0,glBufferLine.nVertices);
        glProgramLine->afterDraw();
    }else if(mode==3){
        glProgramVC->drawX(eyeView,projection,glBufferVC);
    }else if(mode==4){
        /*glProgramTextureProj->beforeDraw(glBufferTextured.vertexB,mExampleTexture);
        glProgramTextureProj->draw(glm::mat4(1.0f),eyeView,projection,0,glBufferTextured.nVertices,glBufferTextured.mMode);
        glProgramTextureProj->updateTexMatrix(buildProjectorMatrices());
        glProgramTextureProj->afterDraw();*/
        glProgramTextureProj->beforeDraw(glBufferPyramid.vertexB,mExampleTexture);
        glProgramTextureProj->draw(modelM,eyeView,projection,0,glBufferPyramid.nVertices,glBufferPyramid.mMode);
        glProgramTextureProj->updateTexMatrix(buildProjectorMatrices());
        glProgramTextureProj->afterDraw();
        //modelM=glm::rotate(modelM, glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         //glProgramTexture->drawX(mExampleTexture,eyeView,projection,glBufferTextured);
         //glProgramTexture->drawX(mExampleTexture,eyeView,projection,glBufferTextured2);
    }
    GLHelper::checkGlError("example_renderer::onDrawFrame");
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(5000);
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
(JNIEnv *env, jobject obj,jint mode) {
    onDrawFrame((int)mode);
}

JNI_METHOD(void, nativeMoveCamera)
(JNIEnv *env, jobject obj,jfloat scale,jfloat x,jfloat y) {
    //placeCamera((float) scale, (float) x, (float) y);
    modelM=glm::rotate(modelM,glm::radians(x), glm::vec3(0.0f, 1.0f, 0.0f));
}

JNI_METHOD(void, nativeSetSeekBarValues)
(JNIEnv *env, jobject obj,jfloat val1,jfloat val2,jfloat val3) {
    seekBarValue1=val1;
    seekBarValue2=val2;
    seekBarValue3=val3;
}

}
