/*******************************************************************
 * Abstraction for drawing an android surface as an OpenGL Texture
 *******************************************************************/

#ifndef GLRENDERTEXTUREEXTERNAL
#define GLRENDERTEXTUREEXTERNAL

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <HelperShader/VDDC.hpp>

class GLProgramTextureExt {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    GLuint mTexture;
    const bool distortionCorrection;
public:
    struct Vertex{
        float x,y,z;
        float u,v;
    };
    explicit GLProgramTextureExt(GLuint videoTexture,bool enableDist=false,const std::array<float,7> *optionalCoeficients= nullptr);
    void beforeDraw(GLuint buffer) const;
    void draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices) const;
    void drawIndexed(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices,GLuint indexBuffer) const;
    void afterDraw() const;
private:
    static const std::string VS(const bool eVDDC,
                                const std::array<float, VDDC::N_UNDISTORTION_COEFICIENTS> *optionalCoeficients){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        if(eVDDC){
            s<<VDDC::undistortCoeficientsToString(*optionalCoeficients);
        }
        s<<"void main() {\n";
        s<< VDDC::writeGLPosition(eVDDC);
        s<<"  vTexCoord = aTexCoord;\n";
#ifdef WIREFRAME
        s<<"gl_PointSize=15.0;";
#endif
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        s<<"#extension GL_OES_EGL_image_external : require\n";
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"uniform samplerExternalOES sTextureExt;\n";
        s<<"void main() {\n";
        s<<"gl_FragColor = texture2D( sTextureExt, vTexCoord );\n";
        //s.append("gl_FragColor.rgb = vec3(1.0,1.0,1.0);\n");
#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        return s.str();
    }
};

#endif
