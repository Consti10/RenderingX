//
// Created by geier on 17/07/2020.
//

#ifndef RENDERINGX_DIRECTRENDER_HPP
#define RENDERINGX_DIRECTRENDER_HPP

#include <Extensions.hpp>

class DirectRender{
public:
    DirectRender(bool QCOM_TILED_RENDERING_AVAILABLE):QCOM_TILED_RENDERING_AVAILABLE(QCOM_TILED_RENDERING_AVAILABLE){
        if(QCOM_TILED_RENDERING_AVAILABLE){
            QCOM_tiled_rendering::init();
        }
    }
    const bool QCOM_TILED_RENDERING_AVAILABLE;
    void begin(bool leftEye, int viewPortW, int viewPortH){
        int x,y,w,h;
        if(leftEye){
            x=0;
            y=0;
            w=viewPortW;
            h=viewPortH;
        }else{
            x=viewPortW;
            y=0;
            w=viewPortW;
            h=viewPortH;
        }
        //NOTE: glClear has to be called from the application, depending on the GPU time (I had to differentiate because of the updateTexImage2D)
        if(QCOM_TILED_RENDERING_AVAILABLE){
            QCOM_tiled_rendering::glStartTilingQCOM(x,y,w,h,0);
        }else{
            const GLenum attachmentsSG[3] = { GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT};
            Extensions::glInvalidateFramebuffer_( GL_FRAMEBUFFER, 3, attachmentsSG );
        }
        glScissor(x,y,w,h);
        glViewport(x,y,w,h);
    }
    void end(bool whichEye){
        if(QCOM_TILED_RENDERING_AVAILABLE){
            QCOM_tiled_rendering::EndTilingQCOM();
        }else{
            const GLenum attachmentsSG[2] = { GL_DEPTH_EXT, GL_STENCIL_EXT};
            Extensions::glInvalidateFramebuffer_( GL_FRAMEBUFFER, 2, attachmentsSG );
        }
    }
};
#endif //RENDERINGX_DIRECTRENDER_HPP
