//
// Created by geier on 02/07/2020.
//

#ifndef RENDERINGX_VRRENDERBUFFER2_H
#define RENDERINGX_VRRENDERBUFFER2_H

#include <GLES2/gl2.h>
#include <AndroidLogger.hpp>
#include <GLHelper.hpp>
#include <Extensions.hpp>


class VrRenderBuffer2{
public:
    GLuint framebuffers[2];
    GLuint WIDTH_PX=0,HEIGH_PX=0;
    //bool texturesCreated=false;
    GLuint textures[2];
    int currentRenderTexture=0;

    std::unique_ptr<FenceSync> mLastFenceSync;

    void createRenderTextures(int W,int H){
        WIDTH_PX=W;
        HEIGH_PX=H;
        for(int i=0;i<2;i++){
            glGenTextures(1, &textures[i]);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            //  GL_RGBA8_OES
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W,H, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D,0);
        }
    }

    void createFrameBuffers(){
        for(int i=0;i<2;i++){
            glGenFramebuffers(1, &framebuffers[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   textures[i], 0);
            GLHelper::checkFramebufferStatus(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_FRAMEBUFFER,0);
        }
    }

    void bind1(){
        GLHelper::checkGlError("before B");
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[currentRenderTexture]);
        GLHelper::checkFramebufferStatus(GL_FRAMEBUFFER);
        glScissor(0,0,WIDTH_PX,HEIGH_PX);
        glViewport(0,0,WIDTH_PX,HEIGH_PX);
        GLHelper::checkGlError("after B");
    }

    // Submit rendering commands in between

    void unbindAndSwap() {
        GLHelper::checkGlError("before U");
        //FenceSync fenceSync;
        // wait until rendering complete
        glFlush();
        glFinish();
        // Now it is safe to swap buffers
        currentRenderTexture=incrementAndModulo(currentRenderTexture);
        GLHelper::checkGlError("after U");
    }

    GLuint getLatestRenderedTexture(){
        //return textures[0];
        int currentSampleIndex=currentRenderTexture+1;
        currentSampleIndex=currentSampleIndex % 2;
        //MLOGD<<"curr sample index "<<currentSampleIndex;
        return textures[currentSampleIndex];
    }

    static int incrementAndModulo(int value){
        value++;
        value=value % 2;
        return value;
    }
};

#endif //RENDERINGX_VRRENDERBUFFER2_H
