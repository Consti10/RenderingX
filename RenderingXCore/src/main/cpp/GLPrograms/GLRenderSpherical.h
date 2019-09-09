/**
 * Based on: GLRenderTextureExternal.h
 * Modified by: Brian Webb (2019)
 *
 * Foobar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 **/

/*******************************************************************
 * Abstraction for drawing an android surface as an OpenGL Texture
 *******************************************************************/
#ifndef GLRENDERSPHERICAL
#define GLRENDERSPHERICAL

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include "android/log.h"
#include <GLRenderSphere.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLRenderSpherical {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mMVPMatrixHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mNormalHandle,mPMatrixHandle;
    GLuint mGLBuffer[2];
    GLuint mTexture[1];
    Sphere* mSphere;
public:
    GLRenderSpherical(const GLuint videoTexture);
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM) const;
};

#endif
