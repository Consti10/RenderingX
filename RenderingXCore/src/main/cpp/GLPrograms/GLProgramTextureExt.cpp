
#include "GLProgramTextureExt.h"

#define TAG "GLRenderTextureExternal"

#define GL_TEXTURE_EXTERNAL_OES 0x00008d65

GLProgramTextureExt::GLProgramTextureExt(const GLuint videoTexture,const bool enableDist,const std::array<float,7> *optionalCoeficients)
        :distortionCorrection(enableDist) {

    mProgram = GLHelper::createProgram(VS(enableDist, optionalCoeficients),FS());
    mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");

    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");
    mSamplerHandle = glGetUniformLocation (mProgram, "sTextureExt" );
    //glGenTextures(1, mTexture);
    mTexture=videoTexture;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
    GLHelper::checkGlError(TAG);
}

void GLProgramTextureExt::beforeDraw(const GLuint buffer) const{
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture);
    glUniform1i(mSamplerHandle,1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,u));
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
    glDrawElements(GL_TRIANGLES,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
#else
    glLineWidth(4.0f);
    glDrawElements(GL_LINES,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
    glDrawElements(GL_POINTS,numberVertices,GL_UNSIGNED_SHORT, (void*)(verticesOffset*sizeof(GLushort)));
#endif
}

void GLProgramTextureExt::afterDraw() const{
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
}
