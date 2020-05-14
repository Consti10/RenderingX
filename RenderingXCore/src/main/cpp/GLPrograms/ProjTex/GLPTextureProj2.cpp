
#include <NDKHelper.hpp>
#include "GLPTextureProj2.h"

constexpr auto TAG="GLRenderTexture(-External)";
constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;


GLPTextureProj2::GLPTextureProj2(){
    mProgram = GLHelper::createProgram(VS(),FS());
    uModelMatrix=GLHelper::GlGetUniformLocation(mProgram, "uModelMatrix");
    uViewMatrix=GLHelper::GlGetUniformLocation(mProgram, "uViewMatrix");
    uProjMatrix=GLHelper::GlGetUniformLocation(mProgram, "uProjMatrix");
    uTextureMatrix=GLHelper::GlGetUniformLocation(mProgram,"uTextureMatrix");
    aPosition = GLHelper::GlGetAttribLocation(mProgram, "aPosition");
    //aTexCoord = _glGetAttribLocation(mProgram, "aTexCoord");
    mSamplerHandle = GLHelper::GlGetUniformLocation (mProgram, "sTexture" );
    GLHelper::checkGlError(TAG);
}

void GLPTextureProj2::beforeDraw(const GLuint buffer, GLuint texture) const{
    glUseProgram(mProgram);

    glActiveTexture(MY_TEXTURE_UNIT1);
    glBindTexture(GL_TEXTURE_2D,texture);
    glUniform1i(mSamplerHandle, MY_SAMPLER_UNIT1);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    //glEnableVertexAttribArray(aTexCoord);
    //glVertexAttribPointer(aTexCoord, 2/*uv*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, u));
    GLHelper::checkGlError("GLPTextureProj2::beforeDraw");
}

void GLPTextureProj2::draw(const glm::mat4x4& ModelM, const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const int verticesOffset, const int numberVertices, GLenum mode) const{
    glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelM));
    glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawArrays(mode, verticesOffset, numberVertices);
    GLHelper::checkGlError("GLPTextureProj2::draw");
}


void GLPTextureProj2::afterDraw() const{
    glDisableVertexAttribArray(aPosition);
    //glDisableVertexAttribArray(aTexCoord);
    glBindTexture(GL_TEXTURE_2D,0);
    //distortionManager.afterDraw();
    GLHelper::checkGlError("GLPTextureProj2::afterDraw");
}


void GLPTextureProj2::updateTexMatrix(const glm::mat4x4& texmatrix) {
    glUniformMatrix4fv(uTextureMatrix, 1, GL_FALSE, glm::value_ptr(texmatrix));
}
