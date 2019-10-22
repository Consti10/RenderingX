/**
 * Based on: GLRenderTextureExternal.cpp
 * Modified by: Brian Webb (2019)
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 **/

#include "GLProgramSpherical.h"
#include "Helper/GLHelper.hpp"

constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;

GLProgramSpherical::GLProgramSpherical(const GLuint videoTexture,const DistortionManager* distortionManager):
distortionManager(distortionManager)
{
    mTexture=videoTexture;

    // Create the GLSL program
    mProgram = GLHelper::createProgram(vs_textureExt_360(enableDist,optionalCoeficients), fs_textureExt_360());

    // Get the location of the uniform variables
    mMVMatrixHandle = (GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle = (GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    mSamplerHandle = glGetUniformLocation(mProgram, "sTextureExt" );

    // Get the location of the attributes
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    //mNormalHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aNormal");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");

    // Configure the texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);

    GLHelper::checkGlError("GLProgramSpherical()");
}

void GLProgramSpherical::beforeDraw(GLuint glBuffVertices) {
    // bind the GLSL texture
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture);
    glUniform1i(mSamplerHandle,1);

    // enable the attributes
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    //glEnableVertexAttribArray((GLuint)mNormalHandle);
    glEnableVertexAttribArray((GLuint)mTextureHandle);

    // bind the sphere
    glBindBuffer(GL_ARRAY_BUFFER, glBuffVertices);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,glBuffIndices);

    // set the attribute arrays
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, sizeof(Sphere::Vertex),nullptr);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(Sphere::Vertex),(GLvoid*)offsetof(Sphere::Vertex,u) );
    //glVertexAttribPointer((GLuint)mPositionHandle, POSITION_COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_BYTES,nullptr);
    //glVertexAttribPointer((GLuint)mTextureHandle,TEXTURE_COORDS_PER_VERTEX,GL_FLOAT,GL_FALSE,VERTEX_STRIDE_BYTES,(GLvoid*)(POSITION_COORDS_PER_VERTEX*sizeof(float)));
    if(distortionManager!= nullptr){
        glUniform2fv(mLOLHandle,(GLsizei)(VDDC::ARRAY_SIZE),(GLfloat*)distortionManager.lol);
    }
}

void GLProgramSpherical::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM,int vertexCount) const{
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    //glDrawElements(GL_TRIANGLES,indexCount, GL_UNSIGNED_INT, (void*)nullptr);

    glDrawArrays(GL_TRIANGLE_STRIP, 0,vertexCount);

    GLHelper::checkGlError("GLProgramSpherical::draw()");
}

void GLProgramSpherical::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    //glDisableVertexAttribArray((GLuint)mNormalHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

