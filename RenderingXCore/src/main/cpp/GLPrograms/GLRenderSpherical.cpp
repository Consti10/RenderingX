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

#include "GLRenderSpherical.h"
#include "../Helper/GLHelper.h"

#define TAG "GLRenderSpherical"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define GL_TEXTURE_EXTERNAL_OES 0x00008d65

GLRenderSpherical::GLRenderSpherical(const GLuint videoTexture) {
    mTexture[0]=videoTexture;

    // Create the sphere class
    mSphere = new Sphere(1.0, 280, 90);

    // Create the GLSL program
    mProgram = createProgram(vs_textureExt_360(), fs_textureExt_360());

    // Get the location of the uniform variables
    mMVMatrixHandle = (GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle = (GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    mSamplerHandle = glGetUniformLocation(mProgram, "sTextureExt" );

    // Get the location of the attributes
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mNormalHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aNormal");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");

    // Configure the texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture[0]);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);

    // Load the sphere.
    glGenBuffers(2,mGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mGLBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, mSphere->getInterleavedVertexSize(), mSphere->getInterleavedVertices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBuffer[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSphere->getIndexSize(), mSphere->getIndices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    checkGlError(TAG);
}

void GLRenderSpherical::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM) const{

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // bind the GLSL texture
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture[0]);
    glUniform1i(mSamplerHandle,1);

    // enable the attributes
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glEnableVertexAttribArray((GLuint)mNormalHandle);
    glEnableVertexAttribArray((GLuint)mTextureHandle);

    // bind the spchere
    glBindBuffer(GL_ARRAY_BUFFER, mGLBuffer[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBuffer[1]);

    // set the attribute arrays
    int stride = mSphere->getInterleavedStride();
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer((GLuint)mNormalHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE,stride,(GLvoid*)(3*sizeof(float)));
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,stride,(GLvoid*)(6*sizeof(float)));

    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawElements(GL_TRIANGLES, mSphere->getIndexCount(), GL_UNSIGNED_INT, (void*)0);

    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mNormalHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
