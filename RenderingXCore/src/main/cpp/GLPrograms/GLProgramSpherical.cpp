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

GLProgramSpherical::GLProgramSpherical(const GLuint videoTexture,float radius,bool enableDist,const std::array<float,7> *optionalCoeficients)
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

void GLProgramSpherical::uploadToGPU(const Sphere &sphere, GLuint glBuffVertices, GLuint glBuffIndices) {
    glBindBuffer(GL_ARRAY_BUFFER, glBuffVertices);
    glBufferData(GL_ARRAY_BUFFER, sphere.getInterleavedVertexSize(), sphere.getInterleavedVertices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.getIndexSize(), sphere.getIndices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //LOGD("mSphere.getIndexSize():%d",mSphere.getIndexSize());
    //LOGD("mSphere.getIndexCount():%d",mSphere.getIndexCount());
    //LOGD("mSphere.getInterleavedVertexSize():%d",mSphere.getInterleavedVertexSize());
    //LOGD("mSphere.getInterleavedVertexCount():%d",mSphere.getInterleavedVertexCount());
}

void GLProgramSpherical::beforeDraw(GLuint glBuffVertices, GLuint glBuffIndices) {
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,glBuffIndices);

    // set the attribute arrays
    //const int stride = mSphere.getInterleavedStride();
    const int interleavedStride=32;
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, interleavedStride,(GLvoid*) 0);
    //glVertexAttribPointer((GLuint)mNormalHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE,stride,(GLvoid*)(3*sizeof(float)));
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,interleavedStride,(GLvoid*)(6*sizeof(float)));
}

void GLProgramSpherical::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM,int indexCount) const{
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawElements(GL_TRIANGLES,indexCount, GL_UNSIGNED_INT, (void*)nullptr);
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

