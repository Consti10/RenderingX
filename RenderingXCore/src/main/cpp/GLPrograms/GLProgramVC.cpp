
#include "GLProgramVC.h"

#define TAG "GLProgramVC"
#define LOGDP(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

GLProgramVC::GLProgramVC(const bool enableDist,const std::array<float,7> *optionalCoeficients):
    distortionCorrection(enableDist) {
    mProgram = GLHelper::createProgram(VS(enableDist, optionalCoeficients),FS());
    mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mColorHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aColor");
    GLHelper::checkGlError("glGetAttribLocation OGProgramColor");
}

void GLProgramVC::beforeDraw(const GLuint buffer) const {
#ifdef WIREFRAME
    glLineWidth(4.0f);
#endif
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray((GLuint)mColorHandle);
    glVertexAttribPointer((GLuint)mColorHandle, 4/*rgba*/,GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),(GLvoid*)offsetof(Vertex,colorRGBA));
}

void GLProgramVC::draw(const Mat4x4 ViewM, const Mat4x4 ProjM,
                       const int verticesOffset, const int numberVertices,const GLenum mode) const{
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, ViewM);
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, ProjM);
#ifdef WIREFRAME
    glDrawArrays(GL_LINES, verticesOffset, numberVertices);
    glDrawArrays(GL_POINTS, verticesOffset, numberVertices);
#else
    glDrawArrays(mode, verticesOffset, numberVertices);
#endif
}

void GLProgramVC::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mColorHandle);
}
