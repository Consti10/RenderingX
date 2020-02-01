#ifndef RENDERINGX_CORE_HELPER_GLHelper
#define RENDERINGX_CORE_HELPER_GLHelper

#include <GLES2/gl2.h>
#include <string>
#include <sstream>
#include "MDebug.hpp"

//#define CRASH_APPLICATION_ON_GL_ERROR

//Convenient methods for:
// Check GL error(s)
// compile shader program from vertex and fragment shader string

class GLHelper{
private:
    inline static const std::string TAG="GLHelper";
public:
    static const char *GlErrorString(GLenum error ){
        switch ( error ){
            case GL_NO_ERROR:						return "GL_NO_ERROR";
            case GL_INVALID_ENUM:					return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:					return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:				return "GL_INVALID_OPERATION";
            case GL_INVALID_FRAMEBUFFER_OPERATION:	return "GL_INVALID_FRAMEBUFFER_OPERATION";
            case GL_OUT_OF_MEMORY:					return "GL_OUT_OF_MEMORY";
            default: return "unknown";
        }
    }
    static void checkGlError(const std::string& caller) {
        GLenum error;
        std::stringstream ss;
        ss<<"GLError:"<<caller.c_str();
        bool anyError=false;
        while ((error = glGetError()) != GL_NO_ERROR) {
            //__android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"%s GLError:%d",op.c_str(),error);
            ss<<" |"<<GlErrorString(error);
            anyError=true;
        }
        if(anyError){
            MDebug::log(ss.str(),TAG);
#ifdef CRASH_APPLICATION_ON_GL_ERROR
            std::exit(-1);
#endif
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
            const int size=1024*4;
            GLchar infoLog[size];
            GLsizei len;
            glGetShaderInfoLog(shader,size,&len,infoLog);
            MDebug::log("Couldn't compile shader "+std::string(infoLog),TAG);
            MDebug::log("Code is:"+shaderCode);
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