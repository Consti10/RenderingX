//
// Created by Constantin on 2/16/2019.
//

#include "GLProgramLine.h"

GLProgramLine::GLProgramLine(){
    mProgram = GLHelper::createProgram(VS,FS);
    mMVMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle =GLHelper::GlGetAttribLocation(mProgram, "aPosition");
    mNormalHandle=GLHelper::GlGetAttribLocation(mProgram, "aNormal");
    mLineWidthHandle=GLHelper::GlGetAttribLocation(mProgram,"aLineWidth");
    mBaseColorHandle=GLHelper::GlGetAttribLocation(mProgram,"aBaseColor");
    mOutlineColorHandle=GLHelper::GlGetAttribLocation(mProgram,"aOutlineColor");
    uEdge=GLHelper::GlGetUniformLocation(mProgram,"uEdge");
    uBorderEdge=GLHelper::GlGetUniformLocation(mProgram,"uBorderEdge");
    uOutlineStrength=GLHelper::GlGetUniformLocation(mProgram,"uOutlineStrength");
    glUseProgram(mProgram);
    setOtherUniforms();
    glUseProgram(0);
    GLHelper::checkGlError("GLProgramLine())");
    // {0,1,2,
    //  0,2,3}; and so on
    /*std::vector<INDEX_DATA> indices(INDEX_BUFFER_SIZE);
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
    mGLIndicesB.uploadGL(indices,GL_STATIC_DRAW);*/
}

void GLProgramLine::beforeDraw(GLuint buffer) const {
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(mPositionHandle);
    glVertexAttribPointer(mPositionHandle, 2, GL_FLOAT, GL_FALSE,sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(mNormalHandle);
    glVertexAttribPointer(mNormalHandle,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,normalX));
    glEnableVertexAttribArray(mLineWidthHandle);
    glVertexAttribPointer(mLineWidthHandle,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,lineW));
    glEnableVertexAttribArray(mBaseColorHandle);
    glVertexAttribPointer(mBaseColorHandle,4,GL_UNSIGNED_BYTE, GL_TRUE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,baseColor));
    glEnableVertexAttribArray(mOutlineColorHandle);
    glVertexAttribPointer(mOutlineColorHandle,4,GL_UNSIGNED_BYTE, GL_TRUE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,outlineColor));
    //
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndicesB.getGLBufferId());
}

void GLProgramLine::setOtherUniforms(float outlineWidth,float edge, float borderEdge) const {
    glUniform1f(uOutlineStrength,outlineWidth);
    glUniform1f(uEdge,edge);
    glUniform1f(uBorderEdge,borderEdge);
}

void GLProgramLine::draw(const glm::mat4x4 &ViewM, const glm::mat4x4 &ProjM, int verticesOffset,
                         int numberVertices) const {
    //const auto mvp=ViewM*ProjM;
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawArrays(GL_TRIANGLES, verticesOffset, numberVertices);
    //glDrawElements(GL_TRIANGLES,numberVertices,GL_UNSIGNED_SHORT, (void*)(sizeof(INDEX_DATA)*verticesOffset));
}

void GLProgramLine::afterDraw() const {
    glDisableVertexAttribArray(mPositionHandle);
    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mLineWidthHandle);
    glDisableVertexAttribArray(mBaseColorHandle);
    glDisableVertexAttribArray(mOutlineColorHandle);
    //distortionManager.afterDraw();
}

void GLProgramLine::drawX(const glm::mat4x4& ViewM, const  glm::mat4x4& ProjM,const GLBuffer<Vertex>& vb)const{
    beforeDraw(vb.getGLBufferId());
    draw(ViewM,ProjM,0,vb.getCount());
    afterDraw();
}

static void writePos(GLProgramLine::Vertex &v,const glm::vec2 &pos){
    v.x=pos.x;
    v.y=pos.y;
}
static void writeColor(GLProgramLine::Vertex &v, const TrueColor baseColor, const TrueColor outlineColor){
    v.baseColor=baseColor;
    v.outlineColor=outlineColor;
}
static void writeNormal(GLProgramLine::Vertex &v,const glm::vec2 &normal,const float lineWidth){
    v.normalX=normal.x;
    v.normalY=normal.y;
    v.lineW=lineWidth;
}
static void writeVertex(GLProgramLine::Vertex& v,const glm::vec2 &pos,const glm::vec2 &normal,const float lineWidth,const TrueColor baseColor, const TrueColor outlineColor){
    v.x=pos.x;
    v.y=pos.y;
    v.normalX=normal.x;
    v.normalY=normal.y;
    v.lineW=lineWidth;
    v.baseColor=baseColor;
    v.outlineColor=outlineColor;
}

void GLProgramLine::convertLineToRenderingData(const glm::vec2 &start, const glm::vec2 &end, const float lineWidth,
                                               GLProgramLine::Vertex *array, int arrayOffset,
                                               const TrueColor baseColor, const TrueColor  outlineColor) {
    //const glm::vec3 start=glm::vec3(start1.x,start1.y,0);
    //const glm::vec3 end=glm::vec3(end1.x,end1.y,0);

    const glm::vec2 dir=glm::normalize(glm::vec2(end-start));
    // yes, we have to swap x and y here
    const glm::vec2 normal=glm::vec2(dir.y,dir.x);

    //const float w=1.0f;
    //const glm::vec3 up=glm::vec3(0,w,0);
    //const glm::vec3 down=glm::vec3(0,-w,0);
    const glm::vec2 up=normal;
    const glm::vec2 down=-normal;
    GLProgramLine::Vertex& p1=array[arrayOffset];
    GLProgramLine::Vertex& p2=array[arrayOffset+1];
    GLProgramLine::Vertex& p3=array[arrayOffset+2];
    GLProgramLine::Vertex& p4=array[arrayOffset+3];
    GLProgramLine::Vertex& p5=array[arrayOffset+4];
    GLProgramLine::Vertex& p6=array[arrayOffset+5];

    writeVertex(p1,start,up,lineWidth,baseColor,outlineColor);
    writeVertex(p2,end,up,lineWidth,baseColor,outlineColor);
    writeVertex(p3,start,down,lineWidth,baseColor,outlineColor);
    //
    writeVertex(p4,start,down,lineWidth,baseColor,outlineColor);
    writeVertex(p5,end,up,lineWidth,baseColor,outlineColor);
    writeVertex(p6,end,down,lineWidth,baseColor,outlineColor);
}

void GLProgramLine::appendLineRenderingData(std::vector<GLProgramLine::Vertex> &data,
                                            const glm::vec2 &start, const glm::vec2 &end,
                                            float lineWidth, TrueColor baseColor,
                                            TrueColor outlineColor) {
    auto p=&data[data.size()];
    data.resize(data.size()+GLProgramLine::VERTICES_PER_LINE);
    convertLineToRenderingData(start,end,lineWidth,p,0,baseColor,outlineColor);
}

std::vector<GLProgramLine::Vertex>
GLProgramLine::makeLine(const glm::vec2 &start, const glm::vec2 &end, float lineWidth,
                        TrueColor baseColor, TrueColor outlineColor) {
    std::vector<GLProgramLine::Vertex> ret(GLProgramLine::VERTICES_PER_LINE);
    convertLineToRenderingData(start,end,lineWidth,ret.data(),0,baseColor,outlineColor);
    return ret;
}

std::vector<GLProgramLine::Vertex>
GLProgramLine::makeHorizontalLine(const glm::vec2 start, float width, float lineHeight,
                                  TrueColor baseColor, TrueColor outlineColor) {
    auto start1=glm::vec2(start);
    start1.y+=lineHeight/2.0f;
    auto end=start1+glm::vec2(width,0);
    return makeLine(start1,end,lineHeight,baseColor,outlineColor);
}




