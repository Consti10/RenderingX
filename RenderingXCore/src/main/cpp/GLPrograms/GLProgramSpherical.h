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
#include "../Sphere/Sphere.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <DistortionCorrection/VDDC.hpp>

class GLProgramSpherical {
public:
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    //GLuint mNormalHandle;
    GLuint mTexture;
public:
    GLProgramSpherical(const GLuint videoTexture,float radius=1.0f,bool enableDist=false,const std::array<float,7> *optionalCoeficients= nullptr);
    void beforeDraw(GLuint glBuffVertices);
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM,int vertexCount) const;
    void afterDraw()const;
private:
    static const std::string vs_textureExt_360(const bool eVDDC,const std::array<float, VDDC::N_UNDISTORTION_COEFICIENTS> *optionalCoeficients){
        std::stringstream ss;
        ss<<"uniform mat4 uMVMatrix;\n";
        ss<<"uniform mat4 uPMatrix;\n";
        ss<<"attribute vec4 aPosition;\n";
        ss<<"attribute vec2 aTexCoord;\n";
        ss<<"varying vec2 vTexCoord;\n";
        if(eVDDC){
            ss<<VDDC::undistortCoeficientsToString(*optionalCoeficients);
        }
        ss<<"void main() {\n";
        ss<<VDDC::writeGLPosition(eVDDC); //gl_Position = (uPMatrix * uMVMatrix) * aPosition;
        ss<<"  vTexCoord = aTexCoord;\n";
        ss<<"}\n";
        return ss.str();
    }
    static const std::string fs_textureExt_360(){
        std::stringstream ss;
        ss<<"#extension GL_OES_EGL_image_external : require\n";
        ss<<"precision mediump float;\n";
        ss<<"varying vec2 vTexCoord;\n";
        ss<<"uniform samplerExternalOES sTextureExt;\n";

        ss<<"vec2 map_equirectangular(in float x,in float y){\n"
                 "        float pi = 3.14159265359;\n"
                 "        float pi_2 = 1.57079632679;\n"
                 "        float xy;\n"
                 "        if (y < 0.5){\n"
                 "            xy = 2.0 * y;\n"
                 "        } else {\n"
                 "            xy = 2.0 * (1.0 - y);\n"
                 "        }\n"
                 "        float sectorAngle = 2.0 * pi * x;\n"
                 "        float nx = xy * cos(sectorAngle);\n"
                 "        float ny = xy * sin(sectorAngle);\n"
                 "        float scale = 0.93;\n"
                 //"float s;"
                 //"float t;"
                 //"        if(y < 0.5){"
                 //"         t = -ny * scale / 2.0 + 0.5;\n"
                 //"         s = -nx * scale / 4.0 + 0.25;\n"
                 //"        }else{"
                 //"         t = ny * scale / 2.0 + 0.5;\n"
                 //"         s = -nx * scale / 4.0 + 0.25;\n"
                 //"        }\n"
                 "        float t = -ny * scale / 2.0 + 0.5;\n"
                 "        float s = -nx * scale / 4.0 + 0.25;\n"
                 "        if (y > 0.5) {\n"
                 "            s = 1.0 - s;\n"
                 "        }\n"
                 "        return vec2(s,t);"
                 "    }";

        ss<<"void main() {\n";
        //ss<<"vec2 newTexCoord=map_equirectangular(vTexCoord.x,vTexCoord.y);";
        ss<<"vec2 newTexCoord=vTexCoord;";
        ss<<"  gl_FragColor = texture2D(sTextureExt, newTexCoord);\n";
        //s.append("gl_FragColor=vec4(1.0,0.0,0.0,1.0);");
        ss<<"}\n";
        return ss.str();
    }

};


//ss<<"vec2 map_equirectangular(in float x,in float y){\n"
//                 "        float pi = 3.14159265359;\n"
//                 "        float pi_2 = 1.57079632679;\n"
//                 "        float xy;\n"
//                 "        if (y < 0.5){\n"
//                 "            xy = 2.0 * y;\n"
//                 "        } else {\n"
//                 "            xy = 2.0 * (1.0 - y);\n"
//                 "        }\n"
//                 "        float sectorAngle = 2.0 * pi * x;\n"
//                 "        float nx = xy * cos(sectorAngle);\n"
//                 "        float ny = xy * sin(sectorAngle);\n"
//                 "        float scale = 0.93;\n"
//                 "        float t = -ny * scale / 2.0 + 0.5;\n"
//                 "        float s = -nx * scale / 4.0 + 0.25;\n"
//                 "        if (y > 0.5) {\n"
//                 "            s = 1.0 - s;\n"
//                 "        }\n"
//                 "        return vec2(s,t);"
//                 "    }";
#endif
