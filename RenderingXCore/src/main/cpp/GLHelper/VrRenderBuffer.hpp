//
// Created by geier on 02/07/2020.
//

#ifndef RENDERINGX_VRRENDERBUFFER_H
#define RENDERINGX_VRRENDERBUFFER_H

#include <GLES2/gl2.h>
#include <AndroidLogger.hpp>
#include <GLHelper.hpp>

class VrRenderBuffer{
public:
    GLuint framebuffer;        // framebuffer object. VR applications render into framebuffer
    GLuint texture;            // texture object. Is distorted / reprojected by the compositor layer renderer
    GLuint WIDTH_PX=0,HEIGH_PX=0;
    void initializeGL(int W,int H){
        WIDTH_PX=W;
        HEIGH_PX=H;
        GLHelper::checkGlError("createVrRenderbuffer1");
        // Create render texture.
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //  GL_RGBA8_OES
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W,H, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // Create render target.
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               texture, 0);
        auto status=glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status!=GL_FRAMEBUFFER_COMPLETE){
            MLOGE<<"Framebuffer not complete "<<status;
        }
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        GLHelper::checkGlError("createVrRenderbuffer2");
    }
    // Bind this framebuffer to OpenGL
    // Also sets scissor and viewport appropriately
    void bind(){
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glScissor(0,0,WIDTH_PX,HEIGH_PX);
        glViewport(0,0,WIDTH_PX,HEIGH_PX);
    }
    // Flush to ensure synchronisation
    // bind default framebuffer instead (which un-binds this render buffer)
    void unbind(){
        glFlush();
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
};

#endif //RENDERINGX_VRRENDERBUFFER_H
