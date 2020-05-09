
#include <NDKHelper.hpp>
#include "GLPTextureProj.h"

constexpr auto TAG="GLRenderTexture(-External)";
constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;


GLPTextureProj::GLPTextureProj(){
    mProgram = GLHelper::createProgram(VS(),FS());
    uModelMatrix=_glGetUniformLocation(mProgram, "uModelMatrix");
    uViewMatrix=_glGetUniformLocation(mProgram, "uViewMatrix");
    uProjMatrix=_glGetUniformLocation(mProgram, "uProjMatrix");
    uTextureMatrix=_glGetUniformLocation(mProgram,"uTextureMatrix");
    aPosition = _glGetAttribLocation(mProgram, "aPosition");
    //aTexCoord = _glGetAttribLocation(mProgram, "aTexCoord");
    mSamplerHandle = _glGetUniformLocation (mProgram, "sTexture" );
    GLHelper::checkGlError(TAG);
}

void GLPTextureProj::beforeDraw(const GLuint buffer, GLuint texture) const{
    glUseProgram(mProgram);

    glActiveTexture(MY_TEXTURE_UNIT1);
    glBindTexture(GL_TEXTURE_2D,texture);
    glUniform1i(mSamplerHandle, MY_SAMPLER_UNIT1);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    //glEnableVertexAttribArray(aTexCoord);
    //glVertexAttribPointer(aTexCoord, 2/*uv*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, u));
    GLHelper::checkGlError("GLPTextureProj::beforeDraw");
}

void GLPTextureProj::draw(const glm::mat4x4& ModelM, const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const int verticesOffset, const int numberVertices, GLenum mode) const{
    glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelM));
    glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawArrays(mode, verticesOffset, numberVertices);
    GLHelper::checkGlError("GLPTextureProj::draw");
}


void GLPTextureProj::afterDraw() const{
    glDisableVertexAttribArray(aPosition);
    //glDisableVertexAttribArray(aTexCoord);
    glBindTexture(GL_TEXTURE_2D,0);
    //distortionManager.afterDraw();
    GLHelper::checkGlError("GLPTextureProj::afterDraw");
}


void GLPTextureProj::updateTexMatrix(const glm::mat4x4& texmatrix) {
    glUniformMatrix4fv(uTextureMatrix, 1, GL_FALSE, glm::value_ptr(texmatrix));
}
