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

// Double buffered
class VrRenderBuffer2{
public:
    VrRenderBuffer2(std::optional<std::string> defaultTextureUrl=std::nullopt):defaultTextureUrl(defaultTextureUrl){}
    // no copy, only move constructor
    //VrRenderBuffer2(const VrRenderBuffer2&)=delete;
    //VrRenderBuffer2(VrRenderBuffer2&&)=default;
    const std::optional<std::string> defaultTextureUrl;
    GLuint defaultTexture;
    GLuint WIDTH_PX=0,HEIGH_PX=0;
    std::array<FramebufferTexture,2> buffers;
    std::mutex mMutex;
    int currentRenderBufferIdx=0;
    int currentSampleBufferIdx=1;
    bool newFrameAvailable=false;

    void loadDefaultTexture(JNIEnv* env,jobject androidContext){
        assert(defaultTextureUrl!=std::nullopt);
        glGenTextures(1, &defaultTexture);
        AGLProgramTexture::loadTexture(defaultTexture, env, androidContext, defaultTextureUrl->c_str());
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

    void bind(){
        assert(defaultTextureUrl==std::nullopt);
        buffers[currentRenderBufferIdx].bind();
    }

    // Submit rendering commands in between

    void unbindAndSwap() {
        assert(defaultTextureUrl==std::nullopt);
        buffers[currentRenderBufferIdx].unbindAndFinishCommands();
        // Now it is safe to swap buffers
        std::lock_guard<std::mutex> lock(mMutex);
        std::swap(currentRenderBufferIdx, currentSampleBufferIdx);
        newFrameAvailable=true;
    }
    GLuint getLatestRenderedTexture(FramebufferTexture::TimingInformation* timingForThisFrame=nullptr){
        if(defaultTextureUrl!=std::nullopt){
            return defaultTexture;
        }
        if(timingForThisFrame!=nullptr){
            *timingForThisFrame=buffers[currentSampleBufferIdx].timingInformation;
        }
        return buffers[currentSampleBufferIdx].texture;
    }
    GLuint getLatestRenderedTexture(bool& isNewFrame,FramebufferTexture::TimingInformation& timingInformation){
        if(defaultTextureUrl!=std::nullopt){
            return defaultTexture;
        }
        std::lock_guard<std::mutex> lock(mMutex);
        if(newFrameAvailable){
            isNewFrame=true;
            newFrameAvailable=false;
        }
        timingInformation=buffers[currentSampleBufferIdx].timingInformation;
        return buffers[currentSampleBufferIdx].texture;
    }

    static int incrementAndModulo(int value){
        value++;
        value=value % 2;
        return value;
    }
};

#endif //RENDERINGX_VRRENDERBUFFER2_H
