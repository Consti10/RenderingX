#include "GLProgramText.h"
#include "TextAssetsHelper.hpp"
#include "Color/Color.hpp"
#include "Helper/NDKHelper.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <vector>

constexpr auto TAG="GLRenderText";
#define LOGD1(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

constexpr const int CHAR_START = 0;           // First Character (ASCII Code)
constexpr const int CHAR_END = 256;            // Last Character (ASCII Code)
constexpr const int CHAR_CNT = CHAR_END-CHAR_START;

static float CELL_WIDTH_U=1.0f/16.0f;//32.0f/512.0f;
static float CELL_HEIGHT_V=1.0f/16.0f;//32.0f/512.0f;
static float FONTS_WIDTHS_U[CHAR_CNT];

//#define WIREFRAME

GLProgramText::GLProgramText(const DistortionManager* distortionManager):
    distortionManager(distortionManager)
    {
    mProgram = GLHelper::createProgram(VS(distortionManager),FS2());
    GLHelper::checkGlError("GLProgramText() create");
    mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    GLHelper::checkGlError("GLProgramText() uniforms0");
    mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    GLHelper::checkGlError("GLProgramText() uniforms1");
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");
    GLHelper::checkGlError("GLProgramText() uniforms2");
    mColorHandle= (GLuint)glGetAttribLocation((GLuint)mProgram, "aVertexColor");
    mSamplerHandle = glGetUniformLocation (mProgram, "sTexture" );
    mOutlineColorHandle=(GLuint)glGetUniformLocation(mProgram,"uOutlineColor");
    mOutlineStrengthHandle=(GLuint)glGetUniformLocation(mProgram,"uOutlineStrength");
    uEdge=(GLuint)glGetUniformLocation(mProgram,"uEdge");
    uBorderEdge=(GLuint)glGetUniformLocation(mProgram,"uBorderEdge");
    if(distortionManager!=nullptr){
        mUndistortionHandles=distortionManager->getUndistortionUniformHandles(mProgram);
    }
    GLHelper::checkGlError("GLProgramText() uniforms3");
#ifdef WIREFRAME
    mOverrideColorHandle=(GLuint)glGetUniformLocation(mProgram,"uOverrideColor");
#endif
    // = {0,1,2, // first triangle (bottom left - top left - top right)
    //  0,2,3}; // second triangle (bottom left - top right - bottom right)
    INDEX_DATA indices[INDEX_BUFFER_SIZE];
    INDEX_DATA offset=0;
    for(INDEX_DATA i=0;i<INDEX_BUFFER_SIZE-5;i+=6){
        indices[i+0]=((GLushort)0)+offset;
        indices[i+1]=((GLushort)1)+offset;
        indices[i+2]=((GLushort)2)+offset;
        indices[i+3]=((GLushort)0)+offset;
        indices[i+4]=((GLushort)2)+offset;
        indices[i+5]=((GLushort)3)+offset;
        offset+=4;
    }
    glGenBuffers(1,&mGLIndicesB);
    glBindBuffer(GL_ARRAY_BUFFER,mGLIndicesB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices),
                 indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //
    glGenTextures(1, &mTexture);
    glUseProgram(mProgram);
    updateOutline();
    setOtherUniforms();
    glUseProgram(0);
    GLHelper::checkGlError("GLProgramText()");
}

void GLProgramText::beforeDraw(const GLuint buffer) const{
    glUseProgram((GLuint)mProgram);
    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D,mTexture);
    glUniform1i(mSamplerHandle,MY_SAMPLER_UNIT);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, sizeof(Vertex),nullptr);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,u));
    glEnableVertexAttribArray((GLuint)mColorHandle);
    glVertexAttribPointer((GLuint)mColorHandle,4/*r,g,b,a*/,GL_UNSIGNED_BYTE, GL_TRUE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,color));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndicesB);
    if(distortionManager!= nullptr){
        distortionManager->beforeDraw(mUndistortionHandles);
    }
}

void GLProgramText::setOtherUniforms(float edge, float borderEdge)const {
    glUniform1f(uEdge,edge);
    glUniform1f(uBorderEdge,borderEdge);
}

void GLProgramText::updateOutline(const glm::vec3 &outlineColor, const float outlineStrength) const{
    glUniform3f(mOutlineColorHandle,outlineColor.r,outlineColor.g,outlineColor.b);
    glUniform1f(mOutlineStrengthHandle,outlineStrength);
}

void GLProgramText::draw(const glm::mat4x4& ViewM, const  glm::mat4x4& ProjM, const int verticesOffset, const int numberIndices) const {
    if(verticesOffset+numberIndices>INDEX_BUFFER_SIZE){
        LOGD1("Error n vert:%d n Indices:%d",numberIndices,verticesOffset);
    }
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
#ifdef WIREFRAME
    glUniform1f(mOverrideColorHandle,1.0f);
    glLineWidth(1);
    glDrawElements(GL_LINES,numberIndices,GL_UNSIGNED_SHORT, (void*)(sizeof(INDEX_DATA)*verticesOffset));
    glDrawElements(GL_POINTS,numberIndices,GL_UNSIGNED_SHORT, (void*)(sizeof(INDEX_DATA)*verticesOffset));
#else
    glDrawElements(GL_TRIANGLES,numberIndices,GL_UNSIGNED_SHORT, (void*)(sizeof(INDEX_DATA)*verticesOffset));
#endif
}


void GLProgramText::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_2D,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    if(distortionManager!= nullptr){
        distortionManager->afterDraw();
    }
}

int GLProgramText::convertStringToRenderingData(const float X, const float Y, const float Z, const float charHeight,
                                                const std::wstring &text,const TrueColor color, Character *array,
                                                const int arrayOffset) {

    const float BOX_SIZE=charHeight*9.0f/8.0f;
    const float PADDING_X_Y=BOX_SIZE/9.0f/2.0f;

    float x=X - PADDING_X_Y;
    const float y=Y - PADDING_X_Y;
    const float z=Z;

    int nChars=0;
    for (const auto character1 : text) {
        const int index=(int)character1;

        // Calculate the uv parts
        const int row = index / 16;
        const int col = index % 16;
        //LOGD1("Char: %c | index: %d | row: %d | col: %d",c,index,row,col);

        const float u = col * CELL_WIDTH_U;
        const float u2 = u + CELL_WIDTH_U;
        const float v = row * CELL_HEIGHT_V;
        const float v2 = v + CELL_HEIGHT_V;

        //
        Character* character=&array[arrayOffset+nChars];
        //0
        character->upperLeft.x = x;
        character->upperLeft.y = y + BOX_SIZE;
        character->upperLeft.z = z;
        character->upperLeft.u = u;
        character->upperLeft.v = v;
        character->upperLeft.color = color;
        //1
        character->lowerLeft.x = x;
        character->lowerLeft.y = y;
        character->lowerLeft.z = z;
        character->lowerLeft.u = u;
        character->lowerLeft.v = v2;
        character->lowerLeft.color = color;
        //2
        character->lowerRight.x = x + BOX_SIZE;
        character->lowerRight.y = y;
        character->lowerRight.z = z;
        character->lowerRight.u = u2;
        character->lowerRight.v = v2;
        character->lowerRight.color = color;
        //3
        character->upperRight.x = x + BOX_SIZE;
        character->upperRight.y  = y + BOX_SIZE;
        character->upperRight.z  = z;
        character->upperRight.u  = u2;
        character->upperRight.v  = v;
        character->upperRight.color  = color;

        x+=getFontWidthSafe(index)*BOX_SIZE;
        nChars++;
    }
    return nChars;
}

float GLProgramText::getStringLength(const std::wstring s, const float charHeight) {
    const float BOX_SIZE=charHeight*9.0f/8.0f;
    float l=0;
    for (int index : s) {
        l+=getFontWidthSafe(index)*BOX_SIZE;
    }
    return l;
}


void  GLProgramText::loadTextRenderingData(JNIEnv *env, jobject androidContext,
                                           const TextAssetsHelper::TEXT_STYLE& textStyle)const {
    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D,mTexture);
    int maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTextureSize);

    //upload the right signed distance field texture atlas into gpu memory
    NDKHelper::uploadAssetImageToGPU(env,androidContext,TextAssetsHelper::getDistanceFieldNameByStyle(textStyle).c_str(),true);
    //load the text widths into cpu memory, as a float array
    NDKHelper::getFloatArrayFromAssets(env,androidContext,TextAssetsHelper::getOtherDataNameByStyle(textStyle).c_str(),FONTS_WIDTHS_U,CHAR_CNT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glHint(GL_GENERATE_MIPMAP_HINT,GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);*/
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLHelper::checkGlError("loadTexture");
}

float GLProgramText::getFontWidthSafe(int idx) {
    if(idx >= CHAR_CNT || idx<0){
        LOGD1("Error Text: Array out of bounds %d", idx);
        return 0;
    }
    return FONTS_WIDTHS_U[idx];
}


