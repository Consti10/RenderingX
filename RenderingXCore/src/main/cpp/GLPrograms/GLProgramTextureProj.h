/*******************************************************************
 * Abstraction for rendering textured vertices. Optionally samples from 'external texture' aka video texture
 *******************************************************************/

#ifndef GL_PROGRAM_TEXTURE_PROJECTIVE
#define GL_PROGRAM_TEXTURE_PROJECTIVE

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <GLBufferHelper.hpp>
#include <VertexBuffer.hpp>
#include <VertexIndexBuffer.h>


class GLProgramTextureProj {
private:
    GLuint mProgram;
    GLuint aPosition,aTexCoord,mSamplerHandle;
    GLuint uModelMatrix,uViewMatrix,uProjMatrix;
    GLuint uTextureMatrix;
    static constexpr auto MY_TEXTURE_UNIT1=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT1=1;
    static constexpr auto MY_TEXTURE_UNIT2=GL_TEXTURE2;
    static constexpr auto MY_SAMPLER_UNIT2=2;
public:
    struct Vertex{
        float x,y,z;
        float u,v;
    };
    using INDEX_DATA=GLuint;
    explicit GLProgramTextureProj();
    void beforeDraw(GLuint buffer,GLuint texture) const;
    void draw(const glm::mat4x4& ModelM,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices,GLenum mode=GL_TRIANGLES) const;
    void afterDraw() const;
    void updateTexMatrix(const glm::mat4x4& texmatrix);
private:
    static const std::string VS(){
        std::stringstream s;
        s<<"uniform mat4 uModelMatrix;";
        s<<"uniform mat4 uViewMatrix;\n";
        s<<"uniform mat4 uProjMatrix;\n";
        s<<"uniform mat4 uTextureMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"varying vec4 vTextureCoordProj;\n";

        s<<"void main() {\n";
        s<<"gl_Position = (uProjMatrix*(uViewMatrix*uModelMatrix))* aPosition;\n";
        s<<"vTexCoord = aTexCoord;\n";

        //s<<"vTextureCoordProj = uTextureMatrix * uModelMatrix * aPosition;\n";
        s<<"vTextureCoordProj = uTextureMatrix * aPosition;";
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        //s<<"#version 300 es \n";
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"varying vec4 vTextureCoordProj;\n";
        s<<"uniform sampler2D sTexture;\n";
        s<<"bool isInside(const in float inX){ return (inX>=0.0 && inX<=1.0);}";
        s<<"bool isInside(const in vec2 inX){ return (isInside(inX.x) && isInside(inX.y));}";
        s<<"void main() {\n";

        //s<<"vec4 textureColor=texture2D(sTexture,vTexCoord);\n";
        s<<"vec4 textureColor=vec4(1.0,0.0,0.0,1.0);\n";
        //s<<"vec4 textureColorProj = vec4(1.0,0.0,0.0,1.0);\n";
        //s<<"vec4 textureColorProj =  texture2DProj(sTexture, vTextureCoordProj);\n";
        s<<"vec2 coord=vTextureCoordProj.xy / vTextureCoordProj.w;";
        //s<<"bool front= vTextureCoordProj.z";
        s<<"vec4 textureColorProj;";
        s<<"if( isInside(coord) ){";
        s<<"textureColorProj =  texture2D(sTexture,coord);";
        s<<"}else{";
        s<<"textureColorProj =  vec4(0.0,0.0,1.0,1.0);";
        s<<"}";
        s<<"gl_FragColor = mix(textureColor, textureColorProj, 0.95);\n";

        s<<"}\n";
        return s.str();
    }
};


//s<<"vec4 textureColorProj = textureProj(texProj, textureCoordProj);\n";
//s<<"vec4 textureColorProj = textureProj(sTexture,vTextureCoordProj);";
//s<<"vec4 textureColorProj = vec4(1.0,0.0,0.0,1.0);\n";
//s<<"vec4 textureColor=texture2D(sTexture,vTexCoord);\n";
//s<<"vec4 textureColor=vec4(1.0,0.0,0.0,1.0);\n";
//s<<"gl_FragColor = mix(textureColor, textureColorProj, 0.4);\n";
#endif //GL_PROGRAM_TEXTURE_PROJECTIVE
