
#include <NDKHelper.hpp>
#include "GLProgramTextureProj.h"
#include "Macros.h"

constexpr auto TAG="GLRenderTexture(-External)";
constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;


GLProgramTextureProj::GLProgramTextureProj(){
    mProgram = GLHelper::createProgram(VS(),FS());
    uMVMatrix=_glGetUniformLocation(mProgram, "uMVMatrix");
    uPMatrix=_glGetUniformLocation(mProgram, "uPMatrix");
    uTextureMatrix=_glGetUniformLocation(mProgram,"uTextureMatrix");
    aPosition = _glGetAttribLocation(mProgram, "aPosition");
    aTexCoord = _glGetAttribLocation(mProgram, "aTexCoord");
    mSamplerHandle = _glGetUniformLocation (mProgram, "sTexture" );
    GLHelper::checkGlError(TAG);
}

void GLProgramTextureProj::beforeDraw(const GLuint buffer,GLuint texture) const{
    glUseProgram(mProgram);

    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D,texture);
    glUniform1i(mSamplerHandle,MY_SAMPLER_UNIT);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(aTexCoord);
    glVertexAttribPointer(aTexCoord, 2/*uv*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, u));
    GLHelper::checkGlError("GLProgramTextureProj::beforeDraw");
}

void GLProgramTextureProj::draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const int verticesOffset, const int numberVertices,GLenum mode) const{
    glUniformMatrix4fv(uMVMatrix, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(uPMatrix, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawArrays(mode, verticesOffset, numberVertices);
    GLHelper::checkGlError("GLProgramTextureProj::draw");
}

void GLProgramTextureProj::drawIndexed(GLuint indexBuffer, const glm::mat4x4 &ViewM,
                                   const glm::mat4x4 &ProjM, int indicesOffset, int numberIndices,
                                   GLenum mode) const {
    glUniformMatrix4fv(uMVMatrix, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(uPMatrix, 1, GL_FALSE, glm::value_ptr(ProjM));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(mode,numberIndices,GL_UNSIGNED_INT, (void*)(indicesOffset*sizeof(INDEX_DATA)));
}

void GLProgramTextureProj::afterDraw() const{
    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aTexCoord);
    glBindTexture(GL_TEXTURE_2D,0);
    //distortionManager.afterDraw();
    GLHelper::checkGlError("GLProgramTextureProj::afterDraw");
}


void GLProgramTextureProj::updateTexMatrix(const glm::mat4x4& texmatrix) {
    glUniformMatrix4fv(uTextureMatrix, 1, GL_FALSE, glm::value_ptr(texmatrix));
}
