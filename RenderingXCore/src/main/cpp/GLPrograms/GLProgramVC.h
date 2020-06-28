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
#include <VDDCManager.h>
#include <AbstractMesh.hpp>
#include <VertexBuffer.hpp>
#include <VertexIndexBuffer.hpp>

//#define WIREFRAME

using Mat4x4=const GLfloat*;

class GLProgramVC {
private:
    // 2 Dimensions or 3 dimensions or Vertex displacement distortion correction (3 dimensions)
    enum MODE{D2,D3,VDDC};
    GLuint mProgram;
    GLuint mPositionHandle,mColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    const VDDCManager* distortionManager;
    VDDCManager::UndistortionHandles* mUndistortionHandles;
public:
    struct Vertex{
        float x,y,z;
        TrueColor colorRGBA;
    };
    using INDEX_DATA=GLushort;
    using ColoredMesh=AbstractMesh<GLProgramVC::Vertex,GLProgramVC::INDEX_DATA>;
    explicit GLProgramVC(const VDDCManager* distortionManager=nullptr, bool coordinates2D=false);
    void beforeDraw(GLuint buffer) const;
    void draw(Mat4x4 ViewM, Mat4x4 ProjM, int verticesOffset,int numberVertices, GLenum mode) const;
    void draw(const glm::mat4& ViewM,const glm::mat4& ProjM, int verticesOffset,int numberVertices, GLenum mode) const;
    void drawIndexed(GLuint indexBuffer,Mat4x4 ViewM, Mat4x4 ProjM,int indicesOffset,int numberIndices, GLenum mode) const;
    void drawIndexed(GLuint indexBuffer,const glm::mat4& ViewM,const glm::mat4& ProjM,int indicesOffset,int numberIndices, GLenum mode) const;
    void afterDraw() const;
    //convenient methods for drawing a colored mesh with / without indices
    //calls beforeDraw(), draw() and afterDraw() properly
    void drawX(const glm::mat4& ViewM,const glm::mat4 ProjM,const ColoredMesh& mesh)const;
private:
    static const std::string VS(){
        std::stringstream s;
        //s<<"#version 100\n";
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec4 aColor;\n";
        s<<"varying vec4 vColor;\n";
        s<< VDDCManager::writeDistortionUtilFunctionsAndUniforms();
        s<<"void main(){\n";
        // Depending on the selected mode writing gl_Position is different
        s<<"#ifdef USE_2D_COORDINATES\n";
        s<<"gl_Position=vec4(aPosition.xy,0,1);\n";
        s<<"#elif defined(ENABLE_VDDC)\n";
        s<< VDDCManager::writeDistortedGLPosition();
        s<<"#else\n";
        s<<"gl_Position = (uPMatrix*uMVMatrix)* aPosition;\n";
        s<<"#endif\n";
        s<<"vColor = aColor;\n";
        s<<"gl_PointSize=15.0;";
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        s<<"precision mediump float;\n";
        s<<"varying vec4 vColor;\n";
        //s<<"varying float invisibleFragment;";
        s<<"void main(){\n";
        s<<"gl_FragColor = vColor;\n";
        //s<<"if(invisibleFragment>=0.9){";
        //s<<"gl_FragColor=vec4(0.0,0.0,0.0,0.0);";
        //s<<"}";
#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        s<<"\n";
        return s.str();
    }
};
using ColoredMesh=GLProgramVC::ColoredMesh;

class GLProgramVC2D: public GLProgramVC{
public:
    GLProgramVC2D():GLProgramVC(nullptr,true){
    }
};

#endif
