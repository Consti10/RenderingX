
#include <NDKHelper.hpp>
#include "GLProgramTexture.h"

constexpr auto TAG="GLRenderTexture(-External)";
constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;


GLProgramTexture::GLProgramTexture(const bool USE_EXTERNAL_TEXTURE,const bool ENABLE_VDDC, const bool use2dCoordinates, const bool mapEquirectangularToInsta360)
        : USE_EXTERNAL_TEXTURE(USE_EXTERNAL_TEXTURE), ENABLE_VDDC(ENABLE_VDDC), MAP_EQUIRECTANGULAR_TO_INSTA360(mapEquirectangularToInsta360) {
    std::string flags;
    if(ENABLE_VDDC){
        flags="#define ENABLE_VDDC\n";
    }else if(use2dCoordinates){
        flags="#define USE_2D_COORDINATES\n";
    }
    if(USE_EXTERNAL_TEXTURE)flags+="#define USE_EXTERNAL_TEXTURE\n";
    mProgram = GLHelper::createProgram(VS(), FS(mapEquirectangularToInsta360), flags);
    mMVMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle = GLHelper::GlGetAttribLocation(mProgram, "aPosition");
    mTextureHandle = GLHelper::GlGetAttribLocation(mProgram, "aTexCoord");
    mSamplerHandle = GLHelper::GlGetUniformLocation (mProgram, "sTexture" );
    if(ENABLE_VDDC){
        mUndistortionHandles=VDDC::getUndistortionUniformHandles(mProgram);
    }
    GLHelper::checkGlError("GLProgramTexture()");
}

void GLProgramTexture::beforeDraw(const GLuint buffer,GLuint texture) const{
    glUseProgram((GLuint)mProgram);

    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,texture);
    glUniform1i(mSamplerHandle,MY_SAMPLER_UNIT);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,u));
}

void GLProgramTexture::draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const int verticesOffset, const int numberVertices,GLenum mode) const{
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
#ifndef WIREFRAME
    glDrawArrays(mode, verticesOffset, numberVertices);
#else
    glLineWidth(4.0f);
    glDrawArrays(GL_LINES, verticesOffset, numberVertices);
    glDrawArrays(GL_POINTS, verticesOffset, numberVertices);
#endif
}

void GLProgramTexture::drawIndexed(GLuint indexBuffer, const glm::mat4x4 &ViewM,
                                   const glm::mat4x4 &ProjM, int indicesOffset, int numberIndices,
                                   GLenum mode) const {
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
#ifndef WIREFRAME
    glDrawElements(mode,numberIndices,GL_UNSIGNED_INT, (void*)(indicesOffset*sizeof(INDEX_DATA)));
#else
    glLineWidth(4.0f);
    glDrawElements(GL_LINES,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
    glDrawElements(GL_POINTS,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
#endif
}

void GLProgramTexture::afterDraw() const{
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,0);
}

void GLProgramTexture::loadTexture(GLuint texture,JNIEnv *env, jobject androidContext, const char *name) {
    //Load texture, generate mipmaps, set sampling parameters
    glBindTexture(GL_TEXTURE_2D,texture);
    NDKHelper::LoadPngFromAssetManager2(env,androidContext,GL_TEXTURE_2D,name);
    glHint(GL_GENERATE_MIPMAP_HINT,GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLProgramTexture::drawX(GLuint texture, const glm::mat4x4 &ViewM, const glm::mat4x4 &ProjM,
                             const GLProgramTexture::TexturedMesh &mesh) {
    mesh.logWarningWhenDrawnUninitialized();
    //MLOGD<<mesh.getCount()<<" "<<mesh.glBufferVertices.count<<" "<<mesh.glBufferIndices->count;
    beforeDraw(mesh.glBufferVertices.glBufferId, texture);
    if(mesh.hasIndices()){
        drawIndexed(mesh.glBufferIndices->glBufferId, ViewM, ProjM, 0, mesh.getCount(), mesh.mode);
    }else{
        draw(ViewM,ProjM,0,mesh.getCount(),mesh.mode);
    }
    afterDraw();
}

void GLProgramTexture::updateUnDistortionUniforms(bool leftEye, const VDDC::DataUnDistortion &dataUnDistortion) const {
    if(!ENABLE_VDDC){
        MLOGE<<"called GLProgramTexture::updateUnDistortion with VDDC disabled";
        return;
    }
    glUseProgram(mProgram);
    VDDC::updateUnDistortionUniforms(leftEye, *mUndistortionHandles, dataUnDistortion);
}
