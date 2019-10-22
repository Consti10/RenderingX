
#include <Helper/ColoredGeometry.hpp>
#include "GLProgramVC.h"


GLProgramVC::GLProgramVC(const DistortionManager* distortionManager):
    distortionManager(distortionManager) {
    mProgram = GLHelper::createProgram(VS(distortionManager),FS());
    mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mColorHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aColor");
    mLOLHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"LOL");
    GLHelper::checkGlError("glGetAttribLocation OGProgramColor");
}


void GLProgramVC::beforeDraw(const GLuint buffer) const {
    glLineWidth(4.0f);
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray((GLuint)mColorHandle);
    glVertexAttribPointer((GLuint)mColorHandle, 4/*rgba*/,GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),(GLvoid*)offsetof(Vertex,colorRGBA));
    if(distortionManager!= nullptr){
        glUniform2fv(mLOLHandle,(GLsizei)(VDDC::ARRAY_SIZE),(GLfloat*)distortionManager->lol);
    }
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

void GLProgramVC::drawIndexed(GLuint indexBuffer, Mat4x4 ViewM, Mat4x4 ProjM, int indicesOffset,
                              int numberIndices, GLenum mode) const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, ViewM);
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, ProjM);
#ifdef WIREFRAME
    //glDrawArrays(GL_LINES, verticesOffset, numberVertices);
    //glDrawArrays(GL_POINTS, verticesOffset, numberVertices);
    glDrawElements(GL_LINES,numberIndices,GL_UNSIGNED_SHORT, (void*)(sizeof(INDEX_DATA)*indicesOffset));
#else
    glDrawArrays(mode,indicesOffset, numberIndices);
#endif
}
