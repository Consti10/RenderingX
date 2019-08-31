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
#include <Types/Color.hpp>
#include <HelperShader/VDDC.hpp>

using Mat4x4=const GLfloat*;

class GLProgramVC {
private:
    GLuint mProgram;
    GLuint mPositionHandle,mColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    const bool distortionCorrection;
public:
    struct Vertex{
        float x,y,z;
        TrueColor colorRGBA;
    };
    explicit GLProgramVC(bool enableDist=false,const std::array<float,7> *optionalCoeficients= nullptr);
    void beforeDraw(GLuint buffer) const;
    void draw(Mat4x4 ViewM, Mat4x4 ProjM, int verticesOffset,int numberVertices, GLenum mode) const;
    void afterDraw() const;
private:
    static const std::string VS(const bool eVDDC,
                                const std::array<float, VDDC::N_UNDISTORTION_COEFICIENTS> *optionalCoeficients){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec4 aColor;\n";
        s<<"varying vec4 vColor;\n";
        if(eVDDC){
            s<<VDDC::undistortCoeficientsToString(*optionalCoeficients);
        }
        s<<"void main(){\n";
        s<< VDDC::writeGLPosition(eVDDC);
        s<<"vColor = aColor;\n";
#ifdef WIREFRAME
        s<<"gl_PointSize=15.0;";
#endif
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
