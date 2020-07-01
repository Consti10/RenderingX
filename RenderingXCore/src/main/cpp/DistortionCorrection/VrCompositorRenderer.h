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
#include <variant>
#include "MatrixHelper.h"

class VrCompositorRenderer {
private:
    gvr::GvrApi *gvr_api;
    glm::mat4 eyeFromHead[2];
    glm::mat4 mProjectionM[2];
    glm::mat4 latestHeadSpaceFromStartSpaceRotation=glm::mat4(1.0f);
    gvr::Mat4f latestHeadSpaceFromStartSpaceRotation_=toGVR(latestHeadSpaceFromStartSpaceRotation);

    static constexpr bool ENABLE_OCCLUSION_MESH=true;
    const TrueColor occlusionMeshColor;
    //One for left and right eye each
    std::array<GLProgramVC::ColoredMesh,2> mOcclusionMesh;
    const bool ENABLE_VDDC;
    VDDC::DataUnDistortion getDataUnDistortion()const;
    //this one is for drawing the occlusion mesh only
    std::unique_ptr<GLProgramVC2D> mGLProgramVC2D;
    // Sample from 'normal' OpenGL texture
    std::unique_ptr<GLProgramTexture> mGLProgramTexture;
    // Sample from 'external' OpenGL texture (aka video texture)
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt;
public:
    VrCompositorRenderer(gvr::GvrApi *gvr_api,const bool ENABLE_VDDC,const TrueColor occlusionMeshColor1=TrueColor2::BLACK);
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
    void addLayer(GLProgramTexture::TexturedMesh mesh, GLuint textureId, bool isExternalTexture, HEAD_TRACKING headTracking=FULL);

    void removeLayers();
    void drawLayers(gvr::Eye eye);

    // Add a 2D layer at position (0,0,Z) in VR 3D space.
    void addLayer2DCanvas(float z,float width,float height,GLuint textureId, bool isExternalTexture);

public:
    // NOT VR
    //void drawLayersMono(glm::mat4 ViewM, glm::mat4 ProjM);
    //void updateHeadsetParams(const MVrHeadsetParams &mDP);
    //
    struct VrRenderbuffer{
        GLuint framebuffer;        // framebuffer object. VR applications render into framebuffer
        GLuint texture;            // texture object. Is distorted / reprojected by the compositor layer renderer
        GLuint WIDTH_PX=0,HEIGH_PX=0;
    };
    static void createVrRenderbuffer(VrRenderbuffer& vrRenderbuffer,int W,int H);
    // DistortionEngine
public:
    //These values must match the surface that is used for rendering VR content
    //E.g. must be created as full screen surface
    int screenWidthP=1920;
    int screenHeightP=1080;
    //
    static constexpr float MIN_Z_DISTANCE=0.1f;
    static constexpr float MAX_Z_DISTANCE=100.0f;
public:
    std::array<MLensDistortion::ViewportParamsNDC,2> screen_params;
    std::array<MLensDistortion::ViewportParamsNDC,2> texture_params;
    PolynomialRadialDistortion mDistortion{};
    PolynomialRadialInverse mInverse{};
public:
    //update with vr headset params
    void updateHeadsetParams(const MVrHeadsetParams& mDP);

    //we do not want the view (rotation) to change during rendering of one frame/eye
    //else we could end up with multiple elements rendered in different perspectives
    void updateLatestHeadSpaceFromStartSpaceRotation();

    //returns the latest 'cached' head rotation
    glm::mat4 GetLatestHeadSpaceFromStartSpaceRotation()const;

    //same but return the gvr matrix type
    gvr::Mat4f GetLatestHeadSpaceFromStartSpaceRotation_()const;

    //returns translation matrix representing half inter-eye-distance
    glm::mat4 GetEyeFromHeadMatrix(gvr::Eye eye)const;

    //returns projection matrix created using the fov of the headset
    glm::mat4 GetProjectionMatrix(gvr::Eye eye)const;

    //Set the viewport to exactly half framebuffer size
    //where framebuffer size==screen size
    void setOpenGLViewport(gvr::Eye eye)const;

    //This one does not use the inverse and is therefore (relatively) slow compared to when
    //using the approximate inverse
    std::array<float, 2> UndistortedNDCForDistortedNDC(const std::array<float,2>& in_ndc,int eye)const{
        return MLensDistortion::UndistortedNDCForDistortedNDC(mDistortion,screen_params[eye],texture_params[eye],in_ndc,false);
    }
    static std::array<float,4> reverseFOV(const std::array<float,4>& fov){
        return {fov[1],fov[0],fov[2],fov[3]};
    }

};


#endif //FPV_VR_OS_VRCOMPOSITORRENDERER_H
