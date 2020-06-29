//
// Created by geier on 22/06/2020.
//

#ifndef FPV_VR_OS_VRCOMPOSITORRENDERER_H
#define FPV_VR_OS_VRCOMPOSITORRENDERER_H

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GLProgramTexture.h>
#include <GLProgramVC.h>
#include <GLBuffer.hpp>
#include <TexturedGeometry.hpp>
#include <DistortionEngine.h>
#include <variant>


class VrCompositorRenderer {
public:
    VDDCManager vddcManager;
    DistortionEngine distortionEngine;
private:
    const TrueColor occlusionMeshColor;
    //One for left and right eye each
    std::array<GLProgramVC::ColoredMesh,2> mOcclusionMesh;
public:
    VrCompositorRenderer(const VDDCManager::DISTORTION_MODE distortionMode,const TrueColor occlusionMeshColor1=TrueColor2::BLACK);
    // Call this once when the OpenGL context is available
    void initializeGL();
    // NONE == position is fixed
    enum HEAD_TRACKING{
        NONE,
        FULL
    };
    // https://developer.oculus.com/documentation/unity/unity-ovroverlay/
    struct VRLayer{
        GLProgramTexture::TexturedMesh mesh;
        GLuint textureId;
        bool isExternalTexture;
        //
        HEAD_TRACKING headTracking;
    };
    // List of layer descriptions
    std::vector<VRLayer> mVrLayerList;
    void addLayer(GLProgramTexture::TexturedMesh mesh, GLuint textureId, bool isExternalTexture, HEAD_TRACKING headTracking=NONE);
    //
    struct DebugLayer{
        ColoredMesh mesh;
    };
    std::vector<DebugLayer> debug;

    void removeLayers();
    void drawLayers(gvr::Eye eye);
    //
    std::unique_ptr<GLProgramVC2D> mGLProgramVC2D;
    // Sample from 'normal' OpenGL texture
    std::unique_ptr<GLProgramTexture> mGLProgramTexture;
    // Sample from 'external' OpenGL texture (aka video texture)
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt;
    //
    std::unique_ptr<GLProgramVC> mGLProgramVC;
public:
    // NOT VR
    //void drawLayersMono(glm::mat4 ViewM, glm::mat4 ProjM);
    void updateHeadsetParams(const MVrHeadsetParams &mDP);
    //
    struct VrRenderbuffer{
        GLuint WIDTH_PX=0,HEIGH_PX=0;
        GLuint framebuffer;        // framebuffer object. VR applications render into framebuffer
        GLuint texture;            // texture object. Is distorted / reprojected by the compositor layer renderer
    };
    static void createVrRenderbuffer(VrRenderbuffer& vrRenderbuffer,int W,int H);
};


#endif //FPV_VR_OS_VRCOMPOSITORRENDERER_H
