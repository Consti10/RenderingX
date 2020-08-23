//
// Created by geier on 22/08/2020.
//

#ifndef RENDERINGX_VRRENDERBUFFER3_H
#define RENDERINGX_VRRENDERBUFFER3_H

#include <GLES2/gl2.h>
#include <AndroidLogger.hpp>
#include <GLHelper.hpp>
#include <Extensions.h>
#include <FramebufferTexture.hpp>

using CLOCK=std::chrono::steady_clock;
struct TimingInformation{
    CLOCK::time_point startSubmitCommands;
    CLOCK::time_point stopSubmitCommands;
    CLOCK::time_point gpuFinishedRendering;
};

struct Frame{
    const FramebufferTexture& renderBuffer;
    TimingInformation timingInformation;
};


class VrRenderBuffer3{
private:
    std::array<FramebufferTexture,3> buffers;
public:
   void acquireFrameForRendering(){

   }
   void submitFrameAfterRendering(){

   }
   void getLatestRenderedFrame(){

   }
};


#endif //RENDERINGX_VRRENDERBUFFER3_H
