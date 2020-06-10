#ifndef RENDERINGX_CORE_HELPER_GLHelper
#define RENDERINGX_CORE_HELPER_GLHelper

#include <GLES2/gl2.h>
#include <string>
#include <sstream>
#include "AndroidLogger.hpp"

//#define CRASH_APPLICATION_ON_GL_ERROR

// Convenient methods for:
// Check GL error(s)
// compile shader program from vertex and fragment shader string
// and more
namespace GLHelper{
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
        ss<<__FILE__<<__LINE__;
        bool anyError=false;
        while ((error = glGetError()) != GL_NO_ERROR) {
            ss<<" |"<<GlErrorString(error);
            anyError=true;
        }
        if(anyError){
            MLOGE<<ss.str();
            // CRASH_APPLICATION_ON_GL_ERROR
            if(false){
                std::exit(-1);
            }
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
            MLOGD<<"Couldn't compile shader "+std::string(infoLog);
            MLOGD<<"Code is:\n"<<shaderCode;
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
                MLOGD<<"Couldn't create shader program";
                glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }
    // some OpenGL calls return a -1 on error, else a positive value (GLuint)
    // these functions print a warning on -1 and return GLuint
    // uniform might just be optimized out
    // https://www.khronos.org/opengl/wiki/GLSL_:_common_mistakes ... will optimize your uniform out.
    static const GLuint GlGetUniformLocation(GLuint program, const GLchar *name){
        GLint location=glGetUniformLocation(program,name);
        if(location<0){
            MLOGE<<"glGetUniformLocation "<<name<<". The Uniform is either missing in the Shader or optimized out.";
        }
        return (GLuint) location;
    }

    static const GLuint GlGetAttribLocation(GLuint program, const GLchar *name){
        GLint location=glGetAttribLocation(program,name);
        if(location<0){
            MLOGE<<"glGetAttribLocation "<<name<<". The Attrib is either missing in the Shader or optimized out.";
        }
        return (GLuint) location;
    }
    static constexpr auto ALL_BUFFERS=GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT;
    static void updateSetClearColor(int& index){
        index++;
        if(index % 2){
            glClearColor(0.0f,0.0f,0.0f,0.0f);
        }else{
            glClearColor(1.0f,1.0f,0.0f,0.0f);
        }
    }
};

#endif