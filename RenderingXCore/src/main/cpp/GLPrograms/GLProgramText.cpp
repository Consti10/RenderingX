#include "GLProgramText.h"
#include "TextAssetsHelper.hpp"
#include <TrueColor.hpp>
#include <NDKHelper.hpp>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <vector>
#include <GLHelper.hpp>

constexpr auto TAG="GLProgramText";

constexpr const int CHAR_START = 0;           // First Character (ASCII Code)
constexpr const int CHAR_END = 256;            // Last Character (ASCII Code)
constexpr const int CHAR_CNT = CHAR_END-CHAR_START;

static float CELL_WIDTH_U=1.0f/16.0f;//32.0f/512.0f;
static float CELL_HEIGHT_V=1.0f/16.0f;//32.0f/512.0f;
static float FONTS_WIDTHS_U[CHAR_CNT];

//#define WIREFRAME

GLProgramText::GLProgramText(){
    mProgram = GLHelper::createProgram(VS(),FS2());
    uProjectionMatrix=GLHelper::GlGetUniformLocation(mProgram,"uProjectionMatrix");
    mPositionHandle = GLHelper::GlGetAttribLocation(mProgram, "aPosition");
    mTextureHandle = GLHelper::GlGetAttribLocation(mProgram, "aTexCoord");
    mColorHandle= GLHelper::GlGetAttribLocation(mProgram, "aVertexColor");
    mSamplerHandle = GLHelper::GlGetUniformLocation (mProgram, "sTexture" );
    mOutlineColorHandle=GLHelper::GlGetUniformLocation(mProgram,"uOutlineColor");
    mOutlineStrengthHandle=GLHelper::GlGetUniformLocation(mProgram,"uOutlineStrength");
    uEdge=GLHelper::GlGetUniformLocation(mProgram,"uEdge");
    uBorderEdge=GLHelper::GlGetUniformLocation(mProgram,"uBorderEdge");
#ifdef WIREFRAME
    mOverrideColorHandle=GLHelper::GlGetUniformLocation(mProgram,"uOverrideColor");
#endif
    // = {0,1,2, // first triangle (bottom left - top left - top right)
    //  0,2,3}; // second triangle (bottom left - top right - bottom right)
    std::vector<INDEX_DATA> indices(INDEX_BUFFER_SIZE);
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
    mGLIndicesB.uploadGL(indices,GL_STATIC_DRAW);
    //
    glGenTextures(1, &mTexture);
    glUseProgram(mProgram);
    updateOutline();
    setOtherUniforms();
    glUseProgram(0);
    GLHelper::checkGlError(TAG);
}

void GLProgramText::beforeDraw(const GLuint buffer) const{
    glUseProgram(mProgram);
    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D,mTexture);
    glUniform1i(mSamplerHandle,MY_SAMPLER_UNIT);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(mPositionHandle);
    // 2 vertices (x and y)
    glVertexAttribPointer(mPositionHandle, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),nullptr);
    glEnableVertexAttribArray(mTextureHandle);
    // 2 u,v values
    glVertexAttribPointer(mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,u));
    glEnableVertexAttribArray(mColorHandle);
    // 4 rgba values (each of them 1 byte wide)
    glVertexAttribPointer(mColorHandle,4/*r,g,b,a*/,GL_UNSIGNED_BYTE, GL_TRUE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,color));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndicesB.getGLBufferId());
}

void GLProgramText::setOtherUniforms(float edge, float borderEdge)const {
    glUniform1f(uEdge,edge);
    glUniform1f(uBorderEdge,borderEdge);
}

void GLProgramText::updateOutline(const glm::vec3 &outlineColor, const float outlineStrength) const{
    glUniform3f(mOutlineColorHandle,outlineColor.r,outlineColor.g,outlineColor.b);
    glUniform1f(mOutlineStrengthHandle,outlineStrength);
}

void GLProgramText::draw(const glm::mat4x4& MVPMatrix, const int verticesOffset, const int numberIndices) const {
    if(verticesOffset+numberIndices>INDEX_BUFFER_SIZE){
        MLOGE<<"n vert:"<<numberIndices<<" n Indices:"<<verticesOffset;
    }
    glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, glm::value_ptr(MVPMatrix));
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
    glDisableVertexAttribArray(mPositionHandle);
    glDisableVertexAttribArray(mTextureHandle);
    glBindTexture(GL_TEXTURE_2D,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    //distortionManager.afterDraw();
}

int GLProgramText::convertStringToRenderingData(const float X, const float Y, const float charHeight,
                                                const std::wstring &text, const TrueColor color, Character *array,
                                                const int arrayOffset) {

    const float BOX_SIZE=charHeight*9.0f/8.0f;
    const float PADDING_X_Y=BOX_SIZE/9.0f/2.0f;

    float x=X - PADDING_X_Y;
    const float y=Y - PADDING_X_Y;
    //const float z=Z;

    int nChars=0;
    for (const auto character1 : text) {
        const int index=(int)character1;

        // Calculate the uv parts
        const int row = index / 16;
        const int col = index % 16;
        //LOGD("Char: %c | index: %d | row: %d | col: %d",c,index,row,col);

        const float u = col * CELL_WIDTH_U;
        const float u2 = u + CELL_WIDTH_U;
        const float v = row * CELL_HEIGHT_V;
        const float v2 = v + CELL_HEIGHT_V;

        //
        Character* character=&array[arrayOffset+nChars];
        //0
        character->upperLeft.x = x;
        character->upperLeft.y = y + BOX_SIZE;
        //character->upperLeft.z = z;
        character->upperLeft.u = u;
        character->upperLeft.v = v;
        character->upperLeft.color = color;
        //1
        character->lowerLeft.x = x;
        character->lowerLeft.y = y;
        //character->lowerLeft.z = z;
        character->lowerLeft.u = u;
        character->lowerLeft.v = v2;
        character->lowerLeft.color = color;
        //2
        character->lowerRight.x = x + BOX_SIZE;
        character->lowerRight.y = y;
        //character->lowerRight.z = z;
        character->lowerRight.u = u2;
        character->lowerRight.v = v2;
        character->lowerRight.color = color;
        //3
        character->upperRight.x = x + BOX_SIZE;
        character->upperRight.y  = y + BOX_SIZE;
        //character->upperRight.z  = z;
        character->upperRight.u  = u2;
        character->upperRight.v  = v;
        character->upperRight.color  = color;

        x+=getFontWidthSafe(index)*BOX_SIZE;
        nChars++;
    }
    return nChars;
}

std::vector<GLProgramText::Character>
GLProgramText::convertStringToRenderingData(float X, float Y,float charHeight,
                                            const std::wstring &text, TrueColor color) {
    std::vector<Character> ret(text.size());
    convertStringToRenderingData(X,Y,charHeight,text,color,ret.data(),0);
    return ret;
}

void GLProgramText::appendString(std::vector<Character>& buff, float X, float Y,
                                 float charHeight, const std::wstring &text, TrueColor color) {
    const auto offset=buff.size();
    buff.resize(buff.size()+text.length());
    convertStringToRenderingData(X,Y,charHeight,text,color,buff.data(),offset);
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
    NDKHelper::LoadPngFromAssetManager2(env,androidContext,GL_TEXTURE_2D,TextAssetsHelper::getDistanceFieldNameByStyle(textStyle),true);

    //load the text widths into cpu memory, as a float array
    const auto tmp=NDKHelper::getFloatArrayFromAssets2<CHAR_CNT>(env,androidContext,TextAssetsHelper::getOtherDataNameByStyle(textStyle));
    memcpy(FONTS_WIDTHS_U,tmp.data(),CHAR_CNT*sizeof(float));

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
        MLOGE<<"Text: Array out of bounds "<<idx;
        return 0;
    }
    return FONTS_WIDTHS_U[idx];
}

