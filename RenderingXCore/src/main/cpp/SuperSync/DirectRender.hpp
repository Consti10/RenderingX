//
// Created by geier on 17/07/2020.
//

#ifndef RENDERINGX_DIRECTRENDER_HPP
#define RENDERINGX_DIRECTRENDER_HPP

#include <Extensions.hpp>

// Direct Rendering refers to rendering a specific area ( and the specific area only)
// This class hides the difference(s) between the two major GPU manufacturer: Qualcomm and MALI (ARM)
class DirectRender{
public:
    using GLViewport=std::array<int,4>;

    DirectRender(bool QCOM_TILED_RENDERING_AVAILABLE):
    QCOM_TILED_RENDERING_AVAILABLE(QCOM_TILED_RENDERING_AVAILABLE){
        if(QCOM_TILED_RENDERING_AVAILABLE){
            //QCOM_tiled_rendering::init();
        }else{
        }
    }
    static void setGlViewport(const GLViewport& viewport){
        glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    }
    static void setGlScissor(const GLViewport& viewport){
        glScissor(viewport[0],viewport[1],viewport[2],viewport[3]);
    }
    const bool QCOM_TILED_RENDERING_AVAILABLE;
    void begin(const GLViewport& viewport)const{
        if(QCOM_TILED_RENDERING_AVAILABLE){
            Extensions::StartTilingQCOM(viewport);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }else{
            const GLenum attachmentsSG[3] = { GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT};
            Extensions::glInvalidateFramebuffer_( GL_FRAMEBUFFER, 3, attachmentsSG );
        }
        setGlViewport(viewport);
        setGlScissor(viewport);
    }
    void end()const{
        if(QCOM_TILED_RENDERING_AVAILABLE){
            Extensions::EndTilingQCOM();
        }else{
            const GLenum attachmentsSG[2] = { GL_DEPTH_EXT, GL_STENCIL_EXT};
            Extensions::glInvalidateFramebuffer_( GL_FRAMEBUFFER, 2, attachmentsSG );
        }
    }
};
//taken from github (so should be the way to go) but i was unable to confirm it yet beacuse of the lack of a MALI GPU
//with clear visually working,but takes too much time on my testing QCOM GPU (I don't have a mali gpu).
//without a clear this one has NO 'tearing fails', but obviously the visual problems
#endif //RENDERINGX_DIRECTRENDER_HPP
