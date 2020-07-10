
#include "GLProgramVC.h"


GLProgramVC::GLProgramVC(bool coordinates2D){
    std::string flags;
    if(coordinates2D){
        flags="#define USE_2D_COORDINATES\n";
    }
    mProgram = GLHelper::createProgram(VS(), FS(), flags);
    mMVMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle =GLHelper::GlGetAttribLocation((GLuint)mProgram, "aPosition");
    mColorHandle =GLHelper::GlGetAttribLocation((GLuint)mProgram, "aColor");
    GLHelper::checkGlError("GLProgramVC()");
}

void GLProgramVC::beforeDraw(const GLuint buffer) const {
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

void GLProgramVC::draw(const glm::mat4& ViewM,const glm::mat4& ProjM, int indicesOffset, int numberIndices,
                       GLenum mode) const {
    draw(glm::value_ptr(ViewM),glm::value_ptr(ProjM),indicesOffset,numberIndices,mode);
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

void GLProgramVC::drawIndexed(GLuint indexBuffer,const glm::mat4& ViewM,const glm::mat4& ProjM,int indicesOffset,int numberIndices, GLenum mode) const{
    drawIndexed(indexBuffer,glm::value_ptr(ViewM),glm::value_ptr(ProjM),indicesOffset,numberIndices,mode);
}

void GLProgramVC::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mColorHandle);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    //distortionManager.afterDraw();
}

void GLProgramVC::drawX(const glm::mat4 &ViewM, const glm::mat4 ProjM,
                        const GLProgramVC::ColoredGLMesh &mesh) const {
    mesh.logWarningWhenDrawingMeshWithoutData();
    beforeDraw(mesh.getVertexBufferId());
    // MLOGD<<mesh.getCount()<<" "<<mesh.glBufferVertices.count<<" "<<mesh.glBufferIndices->count;
    if(mesh.hasIndices()){
        drawIndexed(mesh.getIndexBufferId(),ViewM,ProjM,0,mesh.getCount(),mesh.getMode());
    }else{
        draw(ViewM,ProjM,0,mesh.getCount(),mesh.getMode());
    }
    afterDraw();
}




