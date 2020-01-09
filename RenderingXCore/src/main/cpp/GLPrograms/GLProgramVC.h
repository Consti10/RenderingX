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
#include <array>
#include <Color/Color.hpp>
#include <DistortionCorrection/DistortionManager.h>


//#define WIREFRAME

using Mat4x4=const GLfloat*;

class GLProgramVC {
private:
    GLuint mProgram;
    GLuint mPositionHandle,mColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    const DistortionManager* distortionManager;
    DistortionManager::UndistortionHandles mUndistortionHandles;
public:
    struct Vertex{
        float x,y,z;
        TrueColor colorRGBA;
    };
    using INDEX_DATA=GLushort;
    explicit GLProgramVC(const DistortionManager* distortionManager=nullptr,bool coordinates2D=false);
    void beforeDraw(GLuint buffer) const;
    void draw(Mat4x4 ViewM, Mat4x4 ProjM, int indicesOffset,int numberIndices, GLenum mode) const;
    void drawIndexed(GLuint indexBuffer,Mat4x4 ViewM, Mat4x4 ProjM,int indicesOffset,int numberIndices, GLenum mode) const;
    void afterDraw() const;
private:
    static const std::string VS(const DistortionManager* distortionManager1,bool coordinates2D){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec4 aColor;\n";
        s<<"varying vec4 vColor;\n";
        s<< DistortionManager::writeDistortionParams(distortionManager1);
        s<<"void main(){\n";
        if(coordinates2D){
            s<<"gl_Position = vec4(aPosition.xy,0,1);";
        }else{
            s<<DistortionManager::writeGLPosition(distortionManager1);
        }
        //s<<"gl_Position = (uPMatrix*uMVMatrix)* aPosition";
        s<<"vColor = aColor;\n";
        s<<"gl_PointSize=15.0;";
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        s<<"precision mediump float;\n";
        s<<"varying vec4 vColor;\n";
        s<<"void main(){\n";
        s<<"gl_FragColor = vColor;\n";
#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        s<<"\n";
        return s.str();
    }
};

#endif
