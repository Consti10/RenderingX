/*******************************************************************
 * Abstraction for rendering textured vertices. Optionally samples from 'external texture' aka video texture
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
#include <DistortionCorrection/DistortionManager.h>

//#define WIREFRAME

class GLProgramTexture {
private:
    const bool USE_EXTERNAL_TEXTURE;
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    const DistortionManager* distortionManager;
    DistortionManager::UndistortionHandles mUndistortionHandles;

    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT=1;

public:
    struct Vertex{
        float x,y,z;
        float u,v;
    };
    using INDEX_DATA=GLushort;
    explicit GLProgramTexture(const bool USE_EXTERNAL_TEXTURE=true,const DistortionManager* distortionManager=nullptr,const bool use2dCoordinates=false);
    void beforeDraw(GLuint buffer,GLuint texture) const;
    void draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices) const;
    void drawIndexed(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices,GLuint indexBuffer) const;
    void afterDraw() const;

    static void loadTexture(GLuint texture,JNIEnv *env, jobject androidContext,const char* name);
private:
    static const std::string VS(const DistortionManager* distortionManager1,const bool use2dCoordinates){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        //s<<"varying float overwrite;";
        s<< DistortionManager::writeDistortionParams(distortionManager1);
        s<<"void main() {\n";
        if(use2dCoordinates){
            s<<"gl_Position = vec4(aPosition.xy*2.0,0,1);";
        }else{
            s<<DistortionManager::writeGLPosition(distortionManager1);
        }
        s<<"vTexCoord = aTexCoord;\n";
        /*s<<"vec4 lul = (uMVMatrix * vec4(aTexCoord, 0.0,1.0));";
        s<<"vec4 lul2=lul;";
        s<<"vec3 ndc = lul2.xyz / lul2.w;";
        s<<"vTexCoord=lul.xy;";*/
#ifdef WIREFRAME
        s<<"gl_PointSize=15.0;";
#endif
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(const bool externalTexture){
        std::stringstream s;
        if(externalTexture){
            s<<"#extension GL_OES_EGL_image_external : require\n";
        }
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        if(externalTexture){
            s<<"uniform samplerExternalOES sTexture;\n";
        }else{
            s<<"uniform sampler2D sTexture;\n";
        }
        s<<"void main() {\n";
        s<<"gl_FragColor = texture2D(sTexture,vTexCoord);\n";
        //s.append("gl_FragColor.rgb = vec3(1.0,1.0,1.0);\n");
#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        return s.str();
    }
};

#endif
