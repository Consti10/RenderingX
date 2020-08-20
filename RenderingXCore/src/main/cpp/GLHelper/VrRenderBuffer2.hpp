//
// Created by geier on 02/07/2020.
//

#ifndef RENDERINGX_VRRENDERBUFFER2_H
#define RENDERINGX_VRRENDERBUFFER2_H

#include <GLES2/gl2.h>
#include <AndroidLogger.hpp>
#include <GLHelper.hpp>
#include <Extensions.h>
#include <FramebufferTexture.hpp>

// Double buffered VR renderbuffer
class VrRenderBuffer2{
public:
    VrRenderBuffer2(std::optional<std::string> defaultTextureUrl=std::nullopt):defaultTextureUrl(defaultTextureUrl){}
    // no copy, only move constructor
    VrRenderBuffer2(const VrRenderBuffer2&)=delete;
    VrRenderBuffer2(VrRenderBuffer2&&)=default;
    const std::optional<std::string> defaultTextureUrl;
    GLuint defaultTexture;
    GLuint WIDTH_PX=0,HEIGH_PX=0;
    std::array<FramebufferTexture,2> buffers;
    int currentRenderTexture=0;
    using CLOCK=std::chrono::steady_clock;
    struct TimingInformation{
        CLOCK::time_point startSubmitCommands;
        CLOCK::time_point stopSubmitCommands;
        CLOCK::time_point gpuFinishedRendering;
    };
    TimingInformation timingInformation[2];

    void loadDefaultTexture(JNIEnv* env,jobject androidContext){
        assert(defaultTextureUrl!=std::nullopt);
        glGenTextures(1, &defaultTexture);
        GLProgramTexture::loadTexture(defaultTexture,env,androidContext,defaultTextureUrl->c_str());
    }

    void initializeGL(){
        for(FramebufferTexture& buffer:buffers){
            buffer.initializeGL();
        }
    }

    void setSize(int W,int H){
        WIDTH_PX=W;
        HEIGH_PX=H;
        for(FramebufferTexture& buffer:buffers){
            buffer.setSize(WIDTH_PX,HEIGH_PX);
        }
    }

    void bind1(){
        assert(defaultTextureUrl==std::nullopt);
        glBindFramebuffer(GL_FRAMEBUFFER, buffers[currentRenderTexture].framebuffer);
        glScissor(0,0,WIDTH_PX,HEIGH_PX);
        glViewport(0,0,WIDTH_PX,HEIGH_PX);
        timingInformation[currentRenderTexture].startSubmitCommands=CLOCK::now();
    }

    // Submit rendering commands in between

    void unbindAndSwap() {
        assert(defaultTextureUrl==std::nullopt);
        timingInformation[currentRenderTexture].stopSubmitCommands=CLOCK::now();
        FenceSync fenceSync;
        // wait until rendering complete
        fenceSync.wait(EGL_FOREVER_KHR);
        //MLOGD<<"OSD fence sync took "<<MyTimeHelper::R(fenceSync.getDeltaCreationSatisfied());
        //
        timingInformation[currentRenderTexture].gpuFinishedRendering=CLOCK::now();
        // Now it is safe to swap buffers
        currentRenderTexture=incrementAndModulo(currentRenderTexture);
        GLHelper::checkGlError("after U");
    }

    GLuint getLatestRenderedTexture(TimingInformation* timingForThisFrame=nullptr){
        if(defaultTextureUrl!=std::nullopt){
            return defaultTexture;
        }
        //return textures[0];
        int currentSampleIndex=currentRenderTexture+1;
        if(timingForThisFrame!=nullptr){
            *timingForThisFrame=timingInformation[currentSampleIndex];
        }
        currentSampleIndex=currentSampleIndex % 2;
        //MLOGD<<"curr sample index "<<currentSampleIndex;
        return buffers[currentSampleIndex].texture;
    }

    static int incrementAndModulo(int value){
        value++;
        value=value % 2;
        return value;
    }
};

#endif //RENDERINGX_VRRENDERBUFFER2_H
