//
// Created by Constantin on 2/16/2019.
//

#ifndef FPV_VR_GLPROGRAMLINE_H
#define FPV_VR_GLPROGRAMLINE_H

#include <GLES2/gl2.h>
#include <jni.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <TrueColor.hpp>
#include <GLBuffer.hpp>

/**
 * Drawing a line with OpenGL can be more complicated than it seems at first.
 * Most mobile SOCs don't support a line width bigger than N pixels (where N is most likely 6). Also, these lines are normally not anti-aliased unless MSAA is enabled.
 * This shader can be used to draw an aliased line between two points with a base and outline color without the need for MSAA on the render surface
 */
class GLProgramLine {
private:
    GLuint mProgram;
    GLuint mPositionHandle,mNormalHandle,mLineWidthHandle,mBaseColorHandle,mOutlineColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    GLuint uEdge,uBorderEdge,uOutlineStrength;
    /*using INDEX_DATA=GLushort;
    static constexpr int INDEX_BUFFER_SIZE=65535; //max size of GL unsigned short
    // Create one big index buffer that is used to draw all the line vertices
    GLBuffer<INDEX_DATA> mGLIndicesB;*/
public:
    // A line is constructed by 6 vertices that form 2 triangles
    struct Vertex{
        float x,y;//z is always 0
        float normalX,normalY;
        float lineW;
        TrueColor baseColor;
        TrueColor outlineColor;
    };
    //static constexpr int VERTICES_PER_LINE=6;
    static constexpr const int VERTICES_PER_LINE=6; //2 quads
    static constexpr const int INDICES_PER_LINE=6;
    explicit GLProgramLine();
    void beforeDraw(GLuint buffer) const;
    void beforeDraw(GLBuffer<Vertex>& buffer)const{
        beforeDraw(buffer.getGLBufferId());
    }
    void setOtherUniforms(float outlineWidth=0.4f,float edge=0.1f,float borderEdge=0.1f)const;
    void draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices) const;
    void afterDraw() const;
    //convenient method for GLBuffer
    void drawX(const glm::mat4x4& ViewM, const  glm::mat4x4& ProjM,const GLBuffer<Vertex>& vb)const;
public:
    // creates and writes the data that can be rendered by OpenGL (this OpenGL shader)
    static void convertLineToRenderingData(const glm::vec2& start, const glm::vec2& end, float lineWidth,
                                           Vertex array[], int arrayOffset, TrueColor baseColor=TrueColor2::BLACK, TrueColor outlineColor=TrueColor2::WHITE);
    static void appendLineRenderingData(std::vector<GLProgramLine::Vertex>& data,const glm::vec2& start, const glm::vec2& end, float lineWidth,TrueColor baseColor=TrueColor2::BLACK, TrueColor outlineColor=TrueColor2::WHITE);
public:
    // same as above, but returns the data in a std::vector instead of writing into a c-style buffer
    static std::vector<GLProgramLine::Vertex> makeLine(const glm::vec2& start, const glm::vec2& end, float lineWidth,TrueColor baseColor=TrueColor2::BLACK, TrueColor outlineColor=TrueColor2::WHITE);
    // create a horizontal line starting at @param start and having a width of @param width and a height of @param lineHeight
    // the line extrudes half of @param lineHeight up and down from @param start, respectively
    static std::vector<GLProgramLine::Vertex> makeHorizontalLine(const glm::vec2 start,float width,float lineHeight, TrueColor baseColor=TrueColor2::BLACK, TrueColor outlineColor=TrueColor2::WHITE);
private:
    static constexpr auto VS=R"(
uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;
attribute vec2 aPosition;
attribute vec2 aNormal;
attribute float aLineWidth;
attribute vec4 aBaseColor;
attribute vec4 aOutlineColor;
varying vec2 vNormal;
varying vec4 vBaseColor;
varying vec4 vOutlineColor;
vec2 extruded_pos;
vec4 delta;
vec4 pos;
void main(){
//delta = vec4(aNormal.xy * aLineWidth, 0.0, 0.0);
//pos = uMVMatrix * vec4(aPosition.xy, 0.0, 1.0);
//gl_Position = uPMatrix * (pos + delta);
extruded_pos=aPosition+(aNormal*aLineWidth);
gl_Position = (uPMatrix*uMVMatrix)* vec4(extruded_pos.xy,0.0,1.0);

vNormal=aNormal;
vBaseColor=aBaseColor;
vOutlineColor=aOutlineColor;
}
)";
    static constexpr auto FS=R"(
precision mediump float;
varying vec2 vNormal;
varying vec4 vBaseColor;
varying vec4 vOutlineColor;

const float width=0.5;
uniform float uEdge;
uniform float  uOutlineStrength;
uniform float uBorderEdge;

void main(){
float distanceInv=abs(length(vNormal));

float alpha = 1.0- smoothstep(width,width+uEdge,distanceInv);
float outlineAlpha = 1.0 - smoothstep(width+uOutlineStrength,width+uOutlineStrength+uBorderEdge,distanceInv);
float overallAlpha= alpha + (1.0 - alpha) * outlineAlpha ;
vec3 overallColor= mix(vOutlineColor.rgb,vBaseColor.rgb, alpha / overallAlpha);
gl_FragColor=vec4(overallColor*overallAlpha,overallAlpha);

}

)";

};


#endif //FPV_VR_GLPROGRAMLINE_H
