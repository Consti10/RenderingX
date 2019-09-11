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

GLProgramSpherical::GLProgramSpherical(const GLuint videoTexture):
    mSphere{1.0, 280, 90}
{
    mTexture[0]=videoTexture;

    // Create the GLSL program
    mProgram = GLHelper::createProgram(vs_textureExt_360(), fs_textureExt_360());

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
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture[0]);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);

    // Load the sphere.
    glGenBuffers(1,&mGLBufferVertices);
    glGenBuffers(1,&mGLBufferIndices);
    glBindBuffer(GL_ARRAY_BUFFER, mGLBufferVertices);
    glBufferData(GL_ARRAY_BUFFER, mSphere.getInterleavedVertexSize(), mSphere.getInterleavedVertices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSphere.getIndexSize(), mSphere.getIndices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLHelper::checkGlError("GLProgramSpherical()");
}

void GLProgramSpherical::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM) const{

    // bind the GLSL texture
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture[0]);
    glUniform1i(mSamplerHandle,1);

    // enable the attributes
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    //glEnableVertexAttribArray((GLuint)mNormalHandle);
    glEnableVertexAttribArray((GLuint)mTextureHandle);

    // bind the sphere
    glBindBuffer(GL_ARRAY_BUFFER, mGLBufferVertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mGLBufferIndices);

    // set the attribute arrays
    int stride = mSphere.getInterleavedStride();
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, stride, 0);
    //glVertexAttribPointer((GLuint)mNormalHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE,stride,(GLvoid*)(3*sizeof(float)));
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,stride,(GLvoid*)(6*sizeof(float)));

    //emuglGLESv2_enc: a vertex attribute index out of boundary is detected. Skipping corresponding vertex attribute. buf=0xea118b10
    //emuglGLESv2_enc: Out of bounds vertex attribute info: clientArray? 0 attribute 2 vbo 13 allocedBufferSize 172800 bufferDataSpecified? 1 wantedStart 0 wantedEnd 613660
    //mSphere.printSelf();

    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    //glDrawElements(GL_TRIANGLES, mSphere.getIndexCount(), GL_UNSIGNED_INT, (void*)nullptr);
    glDrawElements(GL_TRIANGLES, 149520, GL_UNSIGNED_INT, (void*) nullptr);

    glDisableVertexAttribArray((GLuint)mPositionHandle);
    //glDisableVertexAttribArray((GLuint)mNormalHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

    GLHelper::checkGlError("GLProgramSpherical::draw()");
}
