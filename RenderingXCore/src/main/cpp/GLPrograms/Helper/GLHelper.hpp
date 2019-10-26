/********************************************
 * Holds all OpenGL Shaders and helper functions
 * vs=Vertex Shader fs=Fragment Shader vddc=Vertex Displacement Distortion Correction
 * *******************************************/
#ifndef GL_HELPER
#define GL_HELPER

#include <string>
#include <GLES2/gl2.h>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <sstream>
#include "MDebug.hpp"

class GLHelper{
private:
    inline static const std::string TAG="GLHelper";
public:
    static const void allocateGLBufferStatic(GLuint buff,void* array,int arraySize){
        glBindBuffer(GL_ARRAY_BUFFER,buff);
        glBufferData(GL_ARRAY_BUFFER, arraySize,
                     array, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    static const void allocateGLBufferDynamic(GLuint buff,void* array,int arraySize){
        glBindBuffer(GL_ARRAY_BUFFER,buff);
        glBufferData(GL_ARRAY_BUFFER, arraySize,
                     array, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /*template<typename T, typename A>
    static const void allocateGLBufferDynamic(GLuint buff,std::vector<T,A> const& vec){{

    }*/


    static void checkGlError(const std::string& op) {
        int error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            __android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"%s GLError:%d",op.c_str(),error);
        }
    }
    static const GLuint loadShader(GLenum type,const std::string& shaderCode){
        GLuint shader = glCreateShader(type);
        // add the source code to the shader and compile it
        const char *c_str = shaderCode.c_str();
        glShaderSource(shader, 1, &c_str, nullptr);
        glCompileShader(shader);
        int result;
        glGetShaderiv(shader,GL_COMPILE_STATUS,&result);
        if(result!=GL_TRUE){
            GLchar infoLog[1024];
            GLsizei len;
            glGetShaderInfoLog(shader,1024,&len,infoLog);
            MDebug::log("Couldn't compile shader "+std::string(infoLog),TAG);
        }
        return shader;
    }
    static GLuint createProgram(const std::string& vertexSource,const std::string& fragmentSource) {
        auto vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
        auto fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);

        auto program = glCreateProgram();
        if (program != 0) {
            glAttachShader(program, vertexShader);
            GLHelper::checkGlError("glAttachShader");
            glAttachShader(program, fragmentShader);
            GLHelper::checkGlError("glAttachShader");
            glLinkProgram(program);
            int linkStatus;
            glGetProgramiv(program,GL_LINK_STATUS,&linkStatus);
            if (linkStatus != GL_TRUE) {
                MDebug::log("Couldn't create shader program");
                glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }
};


#endif