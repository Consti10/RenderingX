//
// Created by geier on 20/08/2020.
//

#ifndef RENDERINGX_FRAMEBUFFERTEXTURE_HPP
#define RENDERINGX_FRAMEBUFFERTEXTURE_HPP

#include <GLES2/gl2.h>
#include <AndroidLogger.hpp>
#include <GLHelper.hpp>
#include <chrono>

// Wrapper around one framebuffer that is bound to a texture id
// Width and Height can be changed dynamically after initializeGL() is called
class FramebufferTexture{
public:
    FramebufferTexture()=default;
    FramebufferTexture(const FramebufferTexture&)=delete;
    FramebufferTexture(FramebufferTexture&&)=default;
    using CLOCK=std::chrono::steady_clock;
    struct TimingInformation{
        CLOCK::time_point startSubmitCommands;
        CLOCK::time_point stopSubmitCommands;
        CLOCK::time_point gpuFinishedRendering;
    };
    TimingInformation timingInformation;

    GLuint framebuffer;
    GLuint texture;
    GLuint WIDTH_PX=0,HEIGH_PX=0;
    void initializeGL(){
        glGenTextures(1, &texture);
        glGenFramebuffers(1, &framebuffer);
        // Default to a size of 64x64
        setSize(64,64);
    }
    void setSize(int W,int H){
        if(WIDTH_PX==W && HEIGH_PX==H){
            return;
        }
        WIDTH_PX=W;
        HEIGH_PX=H;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //  GL_RGBA8_OES
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH_PX,HEIGH_PX, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               texture, 0);
        auto status=glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status!=GL_FRAMEBUFFER_COMPLETE){
            MLOGE<<"Framebuffer not complete "<<status;
        }
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
    // Bind the framebuffer to render to it
    void bind(){
        glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
        timingInformation.startSubmitCommands=CLOCK::now();
        glScissor(0,0,WIDTH_PX,HEIGH_PX);
        glViewport(0,0,WIDTH_PX,HEIGH_PX);
    }
    // Unbind the framebuffer
    // Does not return until all OpenGL commands have been executed
    void unbindAndFinishCommands(){
        timingInformation.stopSubmitCommands=CLOCK::now();
        glFinish();
        //MLOGD<<"FramebufferTexture fence sync took "<<MyTimeHelper::R(fenceSync.getDeltaCreationSatisfied());
        timingInformation.gpuFinishedRendering=CLOCK::now();
    }
};
#endif //RENDERINGX_FRAMEBUFFERTEXTURE_HPP
