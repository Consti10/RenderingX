
#include <Helper/NDKHelper.h>
#include "GLProgramTextureExt.h"

constexpr auto TAG="GLRenderTextureExternal";
constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;

GLProgramTextureExt::GLProgramTextureExt(const GLuint texture,const bool USE_EXTERNAL_TEXTURE,const DistortionManager* distortionManager)
        :USE_EXTERNAL_TEXTURE(USE_EXTERNAL_TEXTURE),mTexture(texture),distortionManager(distortionManager) {
    mProgram = GLHelper::createProgram(VS(distortionManager),FS(USE_EXTERNAL_TEXTURE));
    mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    mLOLHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"LOL");
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");
    mSamplerHandle = glGetUniformLocation (mProgram, "sTexture" );
    //glGenTextures(1, mTexture);
    const auto TEX=USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture((GLenum)TEX,mTexture);
    glTexParameterf((GLenum)TEX, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR);
    glTexParameterf((GLenum)TEX,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture((GLenum)TEX,0);
    GLHelper::checkGlError(TAG);
}

void GLProgramTextureExt::beforeDraw(const GLuint buffer) const{
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,mTexture);
    glUniform1i(mSamplerHandle,1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,u));
    if(distortionManager!= nullptr){
        distortionManager->doLOL(mLOLHandle);
    }
}

void GLProgramTextureExt::draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const int verticesOffset, const int numberVertices) const{
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
#ifndef WIREFRAME
    glDrawArrays(GL_TRIANGLES, verticesOffset, numberVertices);
#else
    glLineWidth(4.0f);
    glDrawArrays(GL_LINES, verticesOffset, numberVertices);
    glDrawArrays(GL_POINTS, verticesOffset, numberVertices);
#endif
}

void GLProgramTextureExt::drawIndexed(const glm::mat4x4 &ViewM, const glm::mat4x4 &ProjM,
                                      const int verticesOffset, const int numberVertices,const GLuint indexBuffer) const {
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
#ifndef WIREFRAME
    glDrawElements(GL_TRIANGLES,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(INDEX_DATA)));
#else
    glLineWidth(4.0f);
    glDrawElements(GL_LINES,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
    glDrawElements(GL_POINTS,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
#endif
}

void GLProgramTextureExt::afterDraw() const{
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,0);
}

void GLProgramTextureExt::loadTexture(JNIEnv *env, jobject androidContext, const char *name) {
    if(USE_EXTERNAL_TEXTURE){
        LOGD("ERROR: Should use an external surface texture");
        return;
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,mTexture);

    NDKHelper::uploadAssetImageToGPU(env,androidContext,name,false);

    glHint(GL_GENERATE_MIPMAP_HINT,GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);*/

    glBindTexture(GL_TEXTURE_2D, 0);
}
