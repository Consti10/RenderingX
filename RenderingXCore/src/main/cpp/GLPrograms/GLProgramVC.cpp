
#include "GLProgramVC.h"

AGLProgramVC::AGLProgramVC(const bool DO_MVP_MULTIPLICATION1):DO_MVP_MULTIPLICATION(DO_MVP_MULTIPLICATION1){
    std::string flags;
    if(DO_MVP_MULTIPLICATION){
        flags="#define DO_MVP_MULTIPLICATION\n";
    }
    mProgram = GLHelper::createProgram(VS, FS, flags);
    mPositionHandle =GLHelper::GlGetAttribLocation((GLuint)mProgram, "aPosition");
    mColorHandle =GLHelper::GlGetAttribLocation((GLuint)mProgram, "aColor");
    if(DO_MVP_MULTIPLICATION){
        mMVPMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uMVPMatrix");
    }
    GLHelper::checkGlError("GLProgramVC()");
}

void AGLProgramVC::beforeDraw(const GLuint buffer) const {
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), nullptr);
    glEnableVertexAttribArray((GLuint)mColorHandle);
    glVertexAttribPointer((GLuint)mColorHandle, 4/*rgba*/,GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ColoredVertex),(GLvoid*)offsetof(ColoredVertex,colorRGBA));
}

void AGLProgramVC::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mColorHandle);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void GLProgramVC::draw(const glm::mat4 &ViewM, const glm::mat4 &ProjM, int verticesOffset,
                       int numberVertices, GLenum mode) const {
    const glm::mat4 mvp=ProjM*ViewM;
    glUniformMatrix4fv(mMVPMatrixHandle, 1, GL_FALSE, glm::value_ptr(mvp));
    glDrawArrays(mode, verticesOffset, numberVertices);
}

void GLProgramVC::drawIndexed(GLuint indexBuffer, const glm::mat4 &ViewM, const glm::mat4 &ProjM,
                              int indicesOffset, int numberIndices, GLenum mode) const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    const glm::mat4 mvp=ProjM*ViewM;
    glUniformMatrix4fv(mMVPMatrixHandle, 1, GL_FALSE, glm::value_ptr(mvp));
    glDrawArrays(mode,indicesOffset, numberIndices);
}

void GLProgramVC::drawX(const glm::mat4 &ViewM, glm::mat4 ProjM, const ColoredGLMeshBuffer &mesh) const {
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

void GLProgramVC2D::draw(int verticesOffset, int numberVertices, GLenum mode) const {
    glDrawArrays(mode, verticesOffset, numberVertices);
}

void GLProgramVC2D::drawIndexed(GLuint indexBuffer, int indicesOffset, int numberIndices,
                                GLenum mode) const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawArrays(mode,indicesOffset, numberIndices);
}

void GLProgramVC2D::drawX(const ColoredGLMeshBuffer &mesh) const {
    mesh.logWarningWhenDrawingMeshWithoutData();
    beforeDraw(mesh.getVertexBufferId());
    // MLOGD<<mesh.getCount()<<" "<<mesh.glBufferVertices.count<<" "<<mesh.glBufferIndices->count;
    if(mesh.hasIndices()){
        drawIndexed(mesh.getIndexBufferId(),0,mesh.getCount(),mesh.getMode());
    }else{
        draw(0,mesh.getCount(),mesh.getMode());
    }
    afterDraw();
}
