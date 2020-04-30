//
// Created by geier on 21/04/2020.
//

#ifndef RENDERINGX_MACROS_H
#define RENDERINGX_MACROS_H

#include "../NDKHelper/MDebug.hpp"
#include <GLES2/gl2.h>

// some OpenGL calls return a -1 on error, else a positive value (GLuint)

// uniform might just be optimized out
// https://www.khronos.org/opengl/wiki/GLSL_:_common_mistakes ... will optimize your uniform out.

static const GLuint _glGetUniformLocation(GLuint program, const GLchar *name){
    GLint location=glGetUniformLocation(program,name);
    if(location<0){
        LOGD("Error glGetUniformLocation %s. The Uniform is either missing in the Shader or optimized out.",name);
    }
    return (GLuint) location;
}

static const GLuint _glGetAttribLocation(GLuint program, const GLchar *name){
    GLint location=glGetAttribLocation(program,name);
    if(location<0){
        LOGD("Error glGetAttribLocation %s. The Attrib is either missing in the Shader or optimized out.",name);
    }
    return (GLuint) location;
}

#endif //RENDERINGX_MACROS_H
