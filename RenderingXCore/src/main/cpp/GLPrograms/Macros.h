//
// Created by geier on 21/04/2020.
//

#ifndef RENDERINGX_MACROS_H
#define RENDERINGX_MACROS_H

#include "../Helper/MDebug.hpp"
#include <GLES2/gl2.h>


static const GLuint _glGetUniformLocation(GLuint program, const GLchar *name){
    GLint location=glGetUniformLocation(program,name);
    if(location<0){
        LOGD("Error glGetUniformLocation %s",name);
    }
    return (GLuint) location;
}

static const GLuint _glGetAttribLocation(GLuint program, const GLchar *name){
    GLint location=glGetAttribLocation(program,name);
    if(location<0){
        LOGD("Error glGetAttribLocation %s",name);
    }
    return (GLuint) location;
}

#endif //RENDERINGX_MACROS_H
