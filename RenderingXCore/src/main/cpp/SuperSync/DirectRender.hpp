//
// Created by geier on 17/07/2020.
//

#ifndef RENDERINGX_DIRECTRENDER_HPP
#define RENDERINGX_DIRECTRENDER_HPP

#include <Extensions.h>

// Direct Rendering refers to rendering a specific area ( and the specific area only)
// This class hides the difference(s) between the two major GPU manufacturer: Qualcomm and MALI (ARM)
// https://community.arm.com/developer/tools-software/graphics/f/discussions/9609/sliced-front-buffer-rendering-on-mali-gpus
namespace DirectRender{
    using GLViewport=std::array<int,4>;
    static void setGlViewport(const GLViewport& viewport){
        glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    }
    static void setGlScissor(const GLViewport& viewport){
        glScissor(viewport[0],viewport[1],viewport[2],viewport[3]);
    }
    static void begin(const GLViewport& viewport){
        if(Extensions::QCOM_tiled_rendering){
            Extensions::StartTilingQCOM(viewport);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }else{
            const GLenum attachmentsColorDepthStencil[3] = {GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT};
            Extensions::glInvalidateFramebuffer_(GL_FRAMEBUFFER, 3, attachmentsColorDepthStencil );
        }
        setGlViewport(viewport);
        setGlScissor(viewport);
    }
    static void end(){
        if(Extensions::QCOM_tiled_rendering){
            Extensions::EndTilingQCOM();
        }else{
            const GLenum attachmentsDepthStencil[2] = {GL_DEPTH_EXT, GL_STENCIL_EXT};
            Extensions::glInvalidateFramebuffer_(GL_FRAMEBUFFER, 2, attachmentsDepthStencil );
        }
    }
};
//taken from github (so should be the way to go) but i was unable to confirm it yet beacuse of the lack of a MALI GPU
//with clear visually working,but takes too much time on my testing QCOM GPU (I don't have a mali gpu).
//without a clear this one has NO 'tearing fails', but obviously the visual problems
#endif //RENDERINGX_DIRECTRENDER_HPP
