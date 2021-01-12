#ifndef GLRENDERTEXTUREEXTERNAL
#define GLRENDERTEXTUREEXTERNAL

#include <VDDC.hpp>
#include <GLMeshBuffer.hpp>
#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/*******************************************************************
 * Abstraction for rendering textured vertices. Optionally samples from 'external texture' aka video texture.
 * Optionally applies V.D.D.C ( Vertex displacement distortion correction) to the Vertices
 *******************************************************************/

// A textured vertex has x,y,z position and u,v texture positions
struct TexturedVertex{
    float x,y,z;
    float u,v;
};
using TexturedMeshData=AMeshData<TexturedVertex,GLuint>;
using TexturedGLMeshBuffer=AGLMeshBuffer<TexturedVertex,GLuint>;

// Same as above, but for a so called 'Stereo Vertex* which is a Vertex with the same x,y,z coordinates for left and right eye
// but different u,v values for left and right eye, respectively
struct TexturedStereoVertex{
    float x,y,z;
    float u_left,v_left;
    float u_right,v_right;
};
using TexturedStereoMeshData=AMeshData<TexturedStereoVertex,GLuint>;
using TexturedStereoGLMeshBuffer=AGLMeshBuffer<TexturedStereoVertex,GLuint>;

namespace TexturedStereoVertexHelper{
    // convert TexturedMeshData to TexturedStereoMeshData duplicating the u,v coordinates for left and right eye
    TexturedStereoMeshData convert(const TexturedMeshData& input);
    // convert TexturedStereoMeshData to TexturedMeshData by selecting either the left or right eye u,v coordinates only
    TexturedMeshData convert(const TexturedStereoMeshData& input,const bool left);
}

// Abstract GLProgram Texture. Abstract because there is a declaration for
//1) Sample from "normal" texture -> GLProgramTexture
//2) Sample from "external" texture -> GLProgramTextureExt below
// e.g. you should use these constructor(s) instead
// TODO: I could make this templated
class AGLProgramTexture {
private:
    const bool USE_EXTERNAL_TEXTURE;
    const bool ENABLE_VDDC;
    const bool USE_2D_COORDINATES;
    const bool MAP_EQUIRECTANGULAR_TO_INSTA360;
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    // Only active if V.D.D.C is enabled
    std::optional<VDDC::UnDistortionUniformHandles> mUndistortionHandles;
    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT=1;
public:
    using INDEX_DATA=GLuint;
    /**
     * Construct a GLProgramTexture which is a c++ representation of an OpenGL shader that renders textured vertices
     * @param USE_EXTERNAL_TEXTURE Sample from *External texture* ( aka SurfaceTexture, VideoTexture) or not
     * @param ENABLE_VDDC Enable/Disable V.D.D.C
     * @param USE_2D_COORDINATES
     * @param mapEquirectangularToInsta360 Experimental do not use
     */
    explicit AGLProgramTexture(const bool USE_EXTERNAL_TEXTURE,const bool ENABLE_VDDC=false, const bool USE_2D_COORDINATES=false, const bool mapEquirectangularToInsta360=false);
    /*
     * Call beforeDraw(), draw() or drawIndexed() and afterDraw() to render a textured mesh
     */
    void beforeDraw(GLuint buffer,GLuint texture) const;
    void beforeDraw(GLBuffer<TexturedVertex>& buffer,GLuint texture)const{
        beforeDraw(buffer.getGLBufferId(),texture);
    }
    void draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices,GLenum mode=GL_TRIANGLES) const;
    void drawIndexed(GLuint indexBuffer,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int indicesOffset, int numberIndices,GLenum mode) const;
    void afterDraw() const;
    // Upload an image as texture to the specified texture unit
    static void loadTexture(GLuint texture,JNIEnv *env, jobject androidContext,const char* name);
    // convenient methods for drawing a textured mesh with / without indices
    // calls beforeDraw(), draw() and afterDraw() properly
    void drawX(GLuint texture,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM,const TexturedGLMeshBuffer& mesh)const;
    // update the uniform values to perform VDDC for left or right eye
    void updateUnDistortionUniforms(bool leftEye, const VDDC::DataUnDistortion& dataUnDistortion)const;
public:
    void beforeDrawStereoVertex(GLuint buffer,GLuint texture,bool useLeftTextureCoords=false) const;
    void drawXStereoVertex(GLuint texture,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM,const TexturedStereoGLMeshBuffer& mesh,bool useLeftTextureCoords=false)const;
private:
    static const std::string VS(){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"#ifdef ENABLE_VDDC\n";
        s<< VDDC::writeDistortionUtilFunctionsAndUniforms();
        s<<"#endif //ENABLE_VDDC\n";
        s<<"void main() {\n";
        // Depending on the selected mode writing gl_Position is different
        s<<"#ifdef USE_2D_COORDINATES\n";
        s<<"gl_Position=aPosition;\n";
        s<<"#elif defined(ENABLE_VDDC)\n";
        s<<"gl_Position=CalculateVertexPosition(uPolynomialRadialInverse,uScreenParams,uTextureParams,uMVMatrix,uPMatrix,aPosition);\n";
        s<<"#else\n";
        s<<"gl_Position = (uPMatrix*uMVMatrix)* aPosition;\n";
        s<<"#endif\n";
        s<<"vTexCoord = aTexCoord;\n";
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(const bool MAP_EQUIRECTANGULAR_TO_INSTA360){
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
        if(MAP_EQUIRECTANGULAR_TO_INSTA360){
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
        if(MAP_EQUIRECTANGULAR_TO_INSTA360){
            s<<"vec2 newTexCoord=map_equirectangular(vTexCoord.x,vTexCoord.y);";
            s<<"gl_FragColor = texture2D(sTexture,newTexCoord);\n";
        }else{
            s<<"gl_FragColor = texture2D(sTexture,vTexCoord);\n";
        }
        //s<<"if(invisibleFragment>=0.9){";
        //s<<"gl_FragColor=vec4(1.0,0.0,0.0,1.0);\n";
        //s<<"}";
        s<<"}\n";
        return s.str();
    }
};

// Sample from normal (not external) texture
class GLProgramTexture: public AGLProgramTexture{
public:
    GLProgramTexture(const bool ENABLE_VDDC=false, const bool USE_2D_COORDINATES=false, const bool mapEquirectangularToInsta360=false):
            AGLProgramTexture(false,ENABLE_VDDC, USE_2D_COORDINATES, mapEquirectangularToInsta360){
    }
};

// Sample from external texture (aka video texture)
class GLProgramTextureExt: public AGLProgramTexture{
public:
    GLProgramTextureExt(const bool ENABLE_VDDC=false, const bool USE_2D_COORDINATES=false, const bool mapEquirectangularToInsta360=false):
            AGLProgramTexture(true,ENABLE_VDDC, USE_2D_COORDINATES, mapEquirectangularToInsta360){
    }
};



#endif
