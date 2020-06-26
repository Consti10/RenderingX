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
    //One for left and right eye each
    std::array<VertexBuffer,2> mOcclusionMesh;
public:
    VrCompositorRenderer(const VDDCManager::DISTORTION_MODE distortionMode);
    // Call this once when the OpenGL context is available
    void initializeGL();
    // NONE == position is fixed
    enum HEAD_TRACKING{
        NONE,
        FULL
    };
    // https://developer.oculus.com/documentation/unity/unity-ovroverlay/
    struct VRLayer{
        // Holds either Indexed vertices (VertexIndexBuffer)
        // or not-indexed vertices (VertexBuffer)
        // std::variant<VertexBuffer,VertexIndexBuffer> geometry;
        GLProgramTexture::Mesh mesh;
        GLuint textureId;
        bool isExternalTexture;
        //
        HEAD_TRACKING headTracking;
    };
    // List of layer descriptions
    std::vector<VRLayer> mVrLayerList;
    void addLayer(GLProgramTexture::Mesh mesh,GLuint textureId,bool isExternalTexture,HEAD_TRACKING headTracking=NONE);
    void removeLayers();
    void drawLayers(gvr::Eye eye);
    //
    std::unique_ptr<GLProgramVC2D> mGLProgramVC2D;
    // Sample from 'normal' OpenGL texture
    std::unique_ptr<GLProgramTexture> mGLProgramTexture;
    // Sample from 'external' OpenGL texture (aka video texture)
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt;
public:
    // NOT VR
    //void drawLayersMono(glm::mat4 ViewM, glm::mat4 ProjM);
    void updateHeadsetParams(const MVrHeadsetParams &mDP);
};


#endif //FPV_VR_OS_VRCOMPOSITORRENDERER_H
