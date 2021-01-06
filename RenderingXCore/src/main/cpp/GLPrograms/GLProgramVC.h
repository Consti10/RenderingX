/*****************************************************
 * GL program vertex color
 * Abstraction for drawing simple colored geometry in form of x,y,z, r,g,b,a vertices
 * The application only has to create and fill a VBO with matching data.
 * Setting up the rendering variables and rendering is done via beforeDraw(),draw() and afterDraw().
 ****************************************************/

#ifndef GLRENDERCOLOR
#define GLRENDERCOLOR

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <TrueColor.hpp>
#include <GLMeshBuffer.hpp>

struct ColoredVertex{
    float x,y,z;
    TrueColor colorRGBA;
};
using COLORED_INDEX_DATA=GLuint;
using ColoredMeshData=AbstractMeshData<ColoredVertex,COLORED_INDEX_DATA>;
using ColoredGLMeshBuffer=AbstractGLMeshBuffer<ColoredVertex,COLORED_INDEX_DATA>;

// Abstract class that loads the apropriate vertex and fragment shader for rendering colored geometry
// See the 2 implementat
class AGLProgramVC {
protected:
    // The MVP multiplication is optional.
    // GLProgramVC does the mvp multiplication and GLProgramVC2D does not transform the raw mesh data
    const bool DO_MVP_MULTIPLICATION;
    GLuint mProgram;
    GLuint mPositionHandle,mColorHandle;
    GLuint mMVPMatrixHandle;
    AGLProgramVC(const bool DO_MVP_MULTIPLICATION);
public:
    void beforeDraw(GLuint buffer) const;
    void beforeDraw(GLBuffer<ColoredVertex>& buffer)const{
        beforeDraw(buffer.getGLBufferId());
    }
    void afterDraw() const;
private:
    static constexpr auto VS=R"(
attribute vec4 aPosition;
attribute vec4 aColor;
#ifdef DO_MVP_MULTIPLICATION
uniform mat4 uMVPMatrix;
#endif
varying vec4 vColor;
void main(){
#ifdef DO_MVP_MULTIPLICATION
gl_Position = uMVPMatrix*aPosition;
#else
gl_Position=vec4(aPosition.xyz,1);
#endif
vColor = aColor;
}
        )";
    static constexpr auto FS=R"(
precision mediump float;
varying vec4 vColor;
void main(){
gl_FragColor = vColor;
}
    )";
};

class GLProgramVC: public AGLProgramVC{
public:
    GLProgramVC():AGLProgramVC(true){}
    void draw(const glm::mat4& ViewM,const glm::mat4& ProjM, int verticesOffset,int numberVertices, GLenum mode) const;
    void drawIndexed(GLuint indexBuffer,const glm::mat4& ViewM,const glm::mat4& ProjM,int indicesOffset,int numberIndices, GLenum mode) const;
    void drawX(const glm::mat4& ViewM,const glm::mat4 ProjM,const ColoredGLMeshBuffer& mesh)const;
};

class GLProgramVC2D: public AGLProgramVC{
public:
    GLProgramVC2D():AGLProgramVC(false){}
    void draw(int verticesOffset,int numberVertices, GLenum mode) const;
    void drawIndexed(GLuint indexBuffer,int indicesOffset,int numberIndices, GLenum mode) const;
    void drawX(const ColoredGLMeshBuffer& mesh)const;
};


#endif
