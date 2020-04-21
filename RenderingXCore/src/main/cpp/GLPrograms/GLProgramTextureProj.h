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
#include <VDDCManager.h>
#include <GLBufferHelper.hpp>
#include <VertexBuffer.hpp>
#include <VertexIndexBuffer.h>


class GLProgramTextureProj {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    GLuint mTextureMatrixHandle;
    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT=1;
public:
    struct Vertex{
        float x,y,z;
        float u,v;
    };
    using INDEX_DATA=GLuint;
    explicit GLProgramTextureProj();
    void beforeDraw(GLuint buffer,GLuint texture) const;
    void draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int verticesOffset, int numberVertices,GLenum mode=GL_TRIANGLES) const;
    void drawIndexed(GLuint indexBuffer,const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, int indicesOffset, int numberIndices,GLenum mode) const;
    void afterDraw() const;
    void updateTexMatrix(const glm::mat4x4& texmatrix);
private:
    static const std::string VS(){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"uniform mat4 uTextureMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"varying vec4 vTextureCoordProj;\n";

        s<<"void main() {\n";

        s<<"vTexCoord = aTexCoord;\n";

        s<<"vTextureCoordProj = uTextureMatrix * aPosition;\n";

        s<<"}\n";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"varying vec4 vTextureCoordProj;\n";
        s<<"varying float invisibleFragment;";
        s<<"uniform sampler2D sTexture;\n";

        s<<"void main() {\n";

        //s<<"vec4 textureColorProj = textureProj(texProj, textureCoordProj);\n";
        //s<<"vec4 textureColorProj = textureProj(sTexture,vTextureCoordProj);";
        s<<"vec4 textureColorProj = vec4(1.0,0.0,0.0,1.0);\n";
        s<<"vec4 textureColor=texture2D(sTexture,vTexCoord);\n";
        //s<<"vec4 textureColor=vec4(1.0,0.0,0.0,1.0);\n";
        s<<"gl_FragColor = mix(textureColor, textureColorProj, 0.4);\n";
        //s<<"gl_FragColor = texture2D(sTexture,vTexCoord);\n";

        s<<"}\n";
        return s.str();
    }
};



#endif //GL_PROGRAM_TEXTURE_PROJECTIVE
