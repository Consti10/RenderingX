/*******************************************************************
 * Abstraction for rendering textured vertices. Optionally samples from 'external texture' aka video texture
 *******************************************************************/

#ifndef GLRENDERTEXTUREEXTERNAL
#define GLRENDERTEXTUREEXTERNAL

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <VDDCManager.h>
#include <AbstractMesh.hpp>

//#define WIREFRAME
class GLProgramTexture {
private:
    const bool USE_EXTERNAL_TEXTURE;
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    const VDDCManager* distortionManager;
    VDDCManager::UndistortionHandles* mUndistortionHandles;
    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT=1;
    const bool mapEquirectangularToInsta360;
public:
    struct Vertex{
        float x,y,z;
        float u,v;
    };
    using INDEX_DATA=GLuint;
    using TexturedMesh=AbstractMesh<GLProgramTexture::Vertex,GLProgramTexture::INDEX_DATA>;
    explicit GLProgramTexture(const bool USE_EXTERNAL_TEXTURE, const VDDCManager* distortionManager=nullptr, const bool use2dCoordinates=false, const bool mapEquirectangularToInsta360=false);
    void beforeDraw(GLuint buffer,GLuint texture) const;
    void draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices,GLenum mode=GL_TRIANGLES) const;
    void drawIndexed(GLuint indexBuffer,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int indicesOffset, int numberIndices,GLenum mode) const;
    void afterDraw() const;
    static void loadTexture(GLuint texture,JNIEnv *env, jobject androidContext,const char* name);
    // convenient methods for drawing a textured mesh with / without indices
    // calls beforeDraw(), draw() and afterDraw() properly
    void drawX(GLuint texture,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM,const TexturedMesh& mesh);
private:
    static const std::string VS(){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        //s<<"varying float overwrite;";
        s<< VDDCManager::writeDistortionUtilFunctionsAndUniforms();
        s<<"void main() {\n";
        // Depending on the selected mode writing gl_Position is different
        s<<"#ifdef USE_2D_COORDINATES\n";
        s<<"gl_Position=vec4(aPosition.xy,0,1);\n";
        s<<"#elif defined(ENABLE_VDDC)\n";
        s<< VDDCManager::writeDistortedGLPosition();
        s<<"#else\n";
        s<<"gl_Position = (uPMatrix*uMVMatrix)* aPosition;\n";
        s<<"#endif\n";
        //
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
    static const std::string FS(const bool mapEquirectangularToInsta360){
        std::stringstream s;
        s<<"#ifdef USE_EXTERNAL_TEXTURE\n";
        s<<"#extension GL_OES_EGL_image_external : require\n";
        s<<"#endif\n";
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"#ifdef USE_EXTERNAL_TEXTURE\n";
        s<<"uniform samplerExternalOES sTexture;\n";
        s<<"#else\n";
        s<<"uniform sampler2D sTexture;\n";
        s<<"#endif\n";
        if(mapEquirectangularToInsta360){
            s<<"vec2 map_equirectangular(in float x,in float y){\n"
               "        float pi = 3.14159265359;\n"
               "        float pi_2 = 1.57079632679;\n"
               "        float xy;\n"
               "        if (y < 0.5){\n"
               "            xy = 2.0 * y;\n"
               "        } else {\n"
               "            xy = 2.0 * (1.0 - y);\n"
               "        }\n"
               "        float sectorAngle = 2.0 * pi * x;\n"
               "        float nx = xy * cos(sectorAngle);\n"
               "        float ny = xy * sin(sectorAngle);\n"
               "        float scale = 0.93;\n"
               "        float t = -ny * scale / 2.0 + 0.5;\n"
               "        float s = -nx * scale / 4.0 + 0.25;\n"
               "        if (y > 0.5) {\n"
               "            s = 1.0 - s;\n"
               "        }\n"
               "        return vec2(1.0-s,t);"
               "    }";
        }
        s<<"void main() {\n";
        if(mapEquirectangularToInsta360){
            s<<"vec2 newTexCoord=map_equirectangular(vTexCoord.x,vTexCoord.y);";
            s<<"gl_FragColor = texture2D(sTexture,newTexCoord);\n";
        }else{
            s<<"gl_FragColor = texture2D(sTexture,vTexCoord);\n";
        }
        //s<<"if(invisibleFragment>=0.9){";
        //s<<"gl_FragColor=vec4(1.0,0.0,0.0,1.0);\n";
        //s<<"}";
#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        return s.str();
    }
};
using TexturedMesh=GLProgramTexture::TexturedMesh;

class GLProgramTextureExt: public GLProgramTexture{
public:
    GLProgramTextureExt(const VDDCManager* dm=nullptr, const bool mapEquirectangularToInsta360=false): GLProgramTexture(true, dm, false, mapEquirectangularToInsta360){
    }
};

#endif
