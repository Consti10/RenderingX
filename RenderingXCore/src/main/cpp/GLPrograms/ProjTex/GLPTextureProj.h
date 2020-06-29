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
#include <VertexBuffer.hpp>
#include <VertexIndexBuffer.hpp>

// TODO replace <0,1> with GL_TEXTURE_BORDER_COLOR ..

class GLPTextureProj {
private:
    GLuint mProgram;
    GLuint aPosition,mSamplerHandle;
    //GLuint aTexCoord;
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
    explicit GLPTextureProj();
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
        s<<"varying vec4 vTexCoordProj;\n";

        s<<"void main() {\n";
        //s<<"gl_Position = (uProjMatrix*(uViewMatrix*uModelMatrix))* aPosition;\n";
        s<<"vec4 pos=vec4(aPosition.xy,0,1);\n";
        s<<"gl_Position = pos;\n";

        //s<<"vTextureCoordProj = uTextureMatrix * uModelMatrix * aPosition;\n";
        //s<<"vTextureCoordProj = uTextureMatrix * aPosition;";
        s<<"vTexCoordProj = uTextureMatrix * pos;\n";
        s<<"}\n";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        //s<<"#version 300 es \n";
        s<<"precision mediump float;\n";
        s<<"varying vec4 vTexCoordProj;\n";
        s<<"uniform sampler2D sTexture;\n";

        s<<"bool isInside(const in float inX){ return (inX>=0.0 && inX<=1.0);}";
        s<<"bool isInside(const in vec2 inX){ return (isInside(inX.x) && isInside(inX.y));}";

        s<<"void main() {\n";
        s<<"vec2 pos=vTexCoordProj.xy / vTexCoordProj.w;";
        s<<"vec4 textureColorProj;";
        s<<"if(isInside(pos) ){"; //&& vTexCoordProj.w>0.0
        s<<"textureColorProj =  texture2D(sTexture,pos);";
        s<<"}else{";
        s<<"textureColorProj =  vec4(0.0,0.0,1.0,1.0);";
        s<<"}";
        //s<<"float alpha=1.0;";
        //s<<"if(textureColorProj.r==1.0 && textureColorProj.g==1.0 && textureColorProj.b==1.0){";
        //s<<"alpha=0.0;";
        //s<<"}";
        //s<<"gl_FragColor = mix(textureColor, textureColorProj, 0.95);\n";
        s<<"gl_FragColor = textureColorProj;\n";
        //s<<"gl_FragColor = vec4(textureColorProj.rgb*alpha,alpha);\n";

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

/*void RendererDistortion::drawEye(gvr::Eye eye, glm::mat4 viewM, glm::mat4 projM, bool meshColorGreen, bool occlusion) {
    const auto& tmp=meshColorGreen ? greenMeshB : blueMeshB;
    mGLProgramVC->drawX(viewM,projM,tmp);

    //if(occlusion){
    //    mBasicGLPrograms->vc2D.drawX(glm::mat4(1.0f),glm::mat4(1.0f),mOcclusionMesh[eye==GVR_LEFT_EYE ? 0 : 1]);
    //}

    // Dev proj texturing
    /*glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
/*{
    glProgramTexture->beforeDraw(glBufferTextured.vertexB,mTextureMonaLisa);
    glm::mat4 modelM=glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-3));

    glProgramTexture->draw(viewM*modelM,projM,0,glBufferTextured.nVertices,glBufferTextured.mMode);
    glProgramTexture->afterDraw();
}*/
/*{
    constexpr float DEFAULT_CAMERA_Z=10.0f;
    constexpr auto DEFAULT_CAMERA_POSITION=glm::vec3(0,0,DEFAULT_CAMERA_Z);
    constexpr auto DEFAULT_CAMERA_LOOK_AT=glm::vec3(0,0,0);
    constexpr auto DEFAULT_CAMERA_UP=glm::vec3(0,1,0);
    const auto DEFAULT_EYE_VIEW=glm::lookAt(DEFAULT_CAMERA_POSITION,DEFAULT_CAMERA_LOOK_AT,DEFAULT_CAMERA_UP);
    glProgramTextureProj->beforeDraw(glBufferTextured.vertexB, mTextureMonaLisa);
    glProgramTextureProj->draw(glm::scale(glm::mat4(1.0),glm::vec3(3.0f,3.0f,3.0f)),DEFAULT_EYE_VIEW,projM,0,glBufferTextured.nVertices,glBufferTextured.mMode);

    glm::mat4 scaleBiasMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(0.5f));
    // Construct the combined matrix to transform the vertex position to its projective
    // texture coordiantes.
    glm::mat4 modelM=glm::translate(glm::vec3(0,0,0.2));
    //glm::mat4 modelM=glm::scale(glm::mat4(1.0f),glm::vec3(0.1,0.1,0.1));
    //glm::mat4 modelM=glm::rotate(glm::radians(20.0f),glm::vec3(0,1,0));
    //glm::mat4 modelM=glm::mat4(1.0);

    //glm::mat4 viewM2=glm::lookAt()
    glm::mat4 projM2=glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);

    glm::mat4 projectorTransformMatrix = scaleBiasMatrix * projM2 *(viewM*modelM);

    glProgramTextureProj->updateTexMatrix(projectorTransformMatrix);
    glProgramTextureProj->afterDraw();
}


mBasicGLPrograms->vc.drawX(viewM,projM,tmp);*/

/*GLHelper::checkGlError("RendererDistortion::drawEye");
}*/

#endif //GL_PROGRAM_TEXTURE_PROJECTIVE
