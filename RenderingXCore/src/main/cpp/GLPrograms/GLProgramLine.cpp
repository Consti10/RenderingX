//
// Created by Constantin on 2/16/2019.
//

#include "GLProgramLine.h"

GLProgramLine::GLProgramLine(const DistortionManager* distortionManager):
distortionManager(distortionManager) {
    mProgram = GLHelper::createProgram(VS(distortionManager),FS());
    mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mNormalHandle=(GLuint)glGetAttribLocation((GLuint)mProgram, "aNormal");
    mLineWidthHandle=(GLuint)glGetAttribLocation((GLuint)mProgram,"aLineWidth");
    mBaseColorHandle=(GLuint)glGetAttribLocation((GLuint)mProgram,"aBaseColor");
    mOutlineColorHandle=(GLuint)glGetAttribLocation((GLuint)mProgram,"aOutlineColor");
    uEdge=(GLuint)glGetUniformLocation(mProgram,"uEdge");
    uBorderEdge=(GLuint)glGetUniformLocation(mProgram,"uBorderEdge");
    uOutlineStrength=(GLuint)glGetUniformLocation(mProgram,"uOutlineStrength");
    if(distortionManager!=nullptr){
        mUndistortionHandles=distortionManager->getUndistortionUniformHandles(mProgram);
    }
    glUseProgram(mProgram);
    setOtherUniforms();
    glUseProgram(0);
    GLHelper::checkGlError("glGetAttribLocation GLProgramLine");
}

void GLProgramLine::beforeDraw(GLuint buffer) const {
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(mPositionHandle);
    glVertexAttribPointer(mPositionHandle, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(mNormalHandle);
    glVertexAttribPointer(mNormalHandle,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,normalX));
    glEnableVertexAttribArray(mLineWidthHandle);
    glVertexAttribPointer(mLineWidthHandle,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,lineW));
    glEnableVertexAttribArray(mBaseColorHandle);
    glVertexAttribPointer(mBaseColorHandle,4,GL_UNSIGNED_BYTE, GL_TRUE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,baseColor));
    glEnableVertexAttribArray(mOutlineColorHandle);
    glVertexAttribPointer(mOutlineColorHandle,4,GL_UNSIGNED_BYTE, GL_TRUE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,outlineColor));
    if(distortionManager!= nullptr){
        distortionManager->beforeDraw(mUndistortionHandles);
    }
}

void GLProgramLine::setOtherUniforms(float outlineWidth,float edge, float borderEdge) const {
    glUniform1f(uOutlineStrength,outlineWidth);
    glUniform1f(uEdge,edge);
    glUniform1f(uBorderEdge,borderEdge);
}

void GLProgramLine::draw(const glm::mat4x4 &ViewM, const glm::mat4x4 &ProjM, int verticesOffset,
                         int numberVertices) const {
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawArrays(GL_TRIANGLES, verticesOffset, numberVertices);
}

void GLProgramLine::afterDraw() const {
    glDisableVertexAttribArray(mPositionHandle);
    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mLineWidthHandle);
    glDisableVertexAttribArray(mBaseColorHandle);
    glDisableVertexAttribArray(mOutlineColorHandle);
    if(distortionManager!= nullptr){
        distortionManager->afterDraw();
    }
}

static void writePos(GLProgramLine::Vertex &v,const glm::vec3 &pos){
    v.x=pos.x;
    v.y=pos.y;
    v.z=pos.z;
}
static void writeColor(GLProgramLine::Vertex &v,const TrueColor baseColor,const TrueColor outlineColor){
    v.baseColor=baseColor;
    v.outlineColor=outlineColor;
}
static void writeNormal(GLProgramLine::Vertex &v,const glm::vec3 &normal,const float lineWidth){
    v.normalX=normal.x;
    v.normalY=normal.y;
    v.normalZ=normal.z;
    v.lineW=lineWidth;
}

void GLProgramLine::convertLineToRenderingData(const glm::vec3 &start,const glm::vec3 &end,const float lineWidth,
                                               GLProgramLine::Vertex *array, int arrayOffset,
                                               const TrueColor baseColor,const TrueColor  outlineColor) {
    const glm::vec3 dir=glm::normalize(glm::vec3(end-start));
    const glm::vec3 normal=glm::vec3(dir.y,dir.x,dir.z);

    //const float w=1.0f;
    //const glm::vec3 up=glm::vec3(0,w,0);
    //const glm::vec3 down=glm::vec3(0,-w,0);
    const glm::vec3 up=normal;
    const glm::vec3 down=-normal;
    GLProgramLine::Vertex& p1=array[arrayOffset];
    GLProgramLine::Vertex& p2=array[arrayOffset+1];
    GLProgramLine::Vertex& p3=array[arrayOffset+2];
    GLProgramLine::Vertex& p4=array[arrayOffset+3];
    GLProgramLine::Vertex& p5=array[arrayOffset+4];
    GLProgramLine::Vertex& p6=array[arrayOffset+5];

    writePos(p1, start);
    writeNormal(p1,up,lineWidth);
    writeColor(p1,baseColor,outlineColor);
    writePos(p2, end);
    writeNormal(p2,up,lineWidth);
    writeColor(p2,baseColor,outlineColor);
    writePos(p3, start);
    writeNormal(p3,down,lineWidth);
    writeColor(p3,baseColor,outlineColor);
    //
    writePos(p4, start);
    writeNormal(p4,down,lineWidth);
    writeColor(p4,baseColor,outlineColor);
    writePos(p5, end);
    writeNormal(p5,up,lineWidth);
    writeColor(p5,baseColor,outlineColor);
    writePos(p6, end);
    writeNormal(p6,down,lineWidth);
    writeColor(p6,baseColor,outlineColor);
}



