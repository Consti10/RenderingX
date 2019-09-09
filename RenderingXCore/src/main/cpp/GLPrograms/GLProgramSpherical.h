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
#include "Sphere.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class GLProgramSpherical {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mMVPMatrixHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mNormalHandle,mPMatrixHandle;
    GLuint mGLBuffer[2];
    GLuint mTexture[1];
    Sphere* mSphere;
public:
    GLProgramSpherical(const GLuint videoTexture);
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM) const;
private:
    static const std::string vs_textureExt_360(){
        std::string s;
        s.append("uniform mat4 uMVMatrix;\n");
        s.append("uniform mat4 uPMatrix;\n");
        s.append("attribute vec3 aPosition;\n");
        s.append("attribute vec3 aNormal;\n");
        s.append("attribute vec2 aTexCoord;\n");
        s.append("varying vec2 vTexCoord;\n");
        s.append("varying vec3 vNormal;\n");
        s.append("void main() {\n");
        s.append("  gl_Position = uPMatrix * uMVMatrix * vec4(aPosition, 1.0);\n");
        s.append("  vTexCoord = aTexCoord;\n");
        s.append("  vNormal = aNormal;");
        s.append("}\n");
        return s;
    }
    static const std::string fs_textureExt_360(){
        std::string s;
        s.append("#extension GL_OES_EGL_image_external : require\n");
        s.append("precision mediump float;\n");
        s.append("varying vec2 vTexCoord;\n");
        s.append("varying vec3 vNormal;\n");
        s.append("uniform samplerExternalOES sTextureExt;\n");
        s.append("void main() {\n");
        s.append("  float pi = 3.14159265359;\n");
        s.append("  float pi_2 = 1.57079632679;\n");
        s.append("  float xy;\n");
        s.append("  if (vTexCoord.y < 0.5) {\n");
        s.append("    xy = 2.0 * vTexCoord.y;\n");
        s.append("  } else {\n");
        s.append("    xy = 2.0 * (1.0 - vTexCoord.y);\n");
        s.append("  }\n");
        s.append("  float sectorAngle = 2.0 * pi * vTexCoord.x;\n");
        s.append("  float nx = xy * cos(sectorAngle);\n");
        s.append("  float ny = xy * sin(sectorAngle);\n");
        s.append("  float scale = 0.93;\n");
        s.append("  float t = -ny * scale / 2.0 + 0.5;\n");
        s.append("  float s = -nx * scale / 4.0 + 0.25;\n");
        s.append("  if (vTexCoord.y > 0.5) {\n");
        s.append("    s = 1.0 - s;\n");
        s.append("  }\n");
        s.append("  vec3 normal = normalize(vNormal);\n");
        s.append("  gl_FragColor = texture2D(sTextureExt, vec2(s, t));\n");
        s.append("}\n");
        return s;
    }
};

#endif
