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
#include <Sphere/UvSphere.hpp>
#include "MatrixHelper.h"
#include <TimeHelper.hpp>
#include <SurfaceTextureUpdate.hpp>
#include <VrRenderBuffer2.hpp>

class VrCompositorRenderer {
public:
    /**
     * Can be constructed without a OpenGL context bound. Don't forget to call initializeGL once context becomes available.
     * @param gvr_api The gvr_api is used for head tracking only
     * @param ENABLE_VDDC if V.D.D.C is not enabled, the VR layers are rendered without distortion correction
     * @param occlusionMeshColor1 Use a custom color for the occlusion mesh for Debugging
     */
    VrCompositorRenderer(JNIEnv* env,jobject androidContext,gvr::GvrApi *gvr_api,const bool ENABLE_VDDC,const bool ENABLE_DEBUG1,const bool ENABLE_VIGNETTE=true);
    /**
     *  Call this once the OpenGL context is available
     */
    void initializeGL();
// Head Tracking begin   ---
private:
    const bool ENABLE_DEBUG;
    const bool ENABLE_VIGNETTE;
    gvr::GvrApi *gvr_api;
    //translation matrix representing half inter-eye-distance
    glm::mat4 eyeFromHead[2]{};
    //projection matrix created using the fov of the headset
    glm::mat4 mProjectionM[2]{};
    glm::mat4 latestHeadSpaceFromStartSpaceRotation=glm::mat4(1.0f);
public:
    // we do not want the view (rotation) to change during rendering of one frame/eye
    // else we could end up with multiple elements rendered in different perspectives
    void updateLatestHeadSpaceFromStartSpaceRotation();
    // returns the latest 'cached' head rotation
    glm::mat4 GetLatestHeadSpaceFromStartSpaceRotation()const;
// Head tracking end ---
// V.D.D.C begin ---
public:
    //These values must match the surface that is used for rendering VR content
    //E.g. must be created as full screen surface
    int SCREEN_WIDTH_PX=1920;
    int SCREEN_HEIGHT_PX=1080;
    int EYE_VIEWPORT_W=SCREEN_WIDTH_PX/2;
    int EYE_VIEWPORT_H=SCREEN_HEIGHT_PX;
    // Min and Max clip distance
    static constexpr float MIN_Z_DISTANCE=0.1f;
    static constexpr float MAX_Z_DISTANCE=100.0f;
private:
    std::array<MLensDistortion::ViewportParamsHSNDC,2> screen_params{};
    std::array<MLensDistortion::ViewportParamsHSNDC,2> texture_params{};
    PolynomialRadialDistortion mDistortion{};
    PolynomialRadialInverse mInverse{};
    VDDC::DataUnDistortion mDataUnDistortion=VDDC::DataUnDistortion::identity();
public:
    // update with vr headset params
    void updateHeadsetParams(const MVrHeadsetParams& mDP);
// V.D.D.C end ---
private:
    //One for left and right eye each
    std::array<ColoredGLMeshBuffer,2> mOcclusionMesh;
    const bool ENABLE_VDDC;
    //this one is for drawing the occlusion mesh only, no V.D.D.C, source mesh holds NDC
    std::unique_ptr<GLProgramVC2D> mGLProgramVC2D;
    // Use NDC (normalized device coordinates), both for normal and ext texture
    std::unique_ptr<GLProgramTexture> mGLProgramTexture2D;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt2D;
    // Apply V.D.D.C to the 3d coordinates, both for normal and ext texture
    std::unique_ptr<GLProgramTexture> mGLProgramTextureVDDC;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExtVDDC;
public:
    // NONE == position is fixed
    enum HEAD_TRACKING{
        NONE,
        FULL
    };
    // https://developer.oculus.com/documentation/unity/unity-ovroverlay/
    struct VRLayer{
        HEAD_TRACKING headTracking;
        // If head tracking is disabled for this layer we can pre-calculate the undistorted vertices
        // for both the left and right eye. Else, the vertex shader does the un-distortion and
        // we do not touch the mesh data.
        std::unique_ptr<GLProgramTexture::TexturedStereoGLMeshBuffer> meshLeftAndRightEye=nullptr;
        std::unique_ptr<GLProgramTexture::TexturedGLMeshBuffer> optionalLeftEyeDistortedMesh=nullptr;
        std::unique_ptr<GLProgramTexture::TexturedGLMeshBuffer> optionalRightEyeDistortedMesh=nullptr;
        GLuint textureId;
        bool isExternalTexture;
        // By supplying the content provider updating the layer each frame is much easier
        std::optional<SurfaceTextureUpdate> contentProvider1=std::nullopt;
        std::optional<VrRenderBuffer2> contentProvider2=std::nullopt;
    };
    // List of layer descriptions
    std::vector<VRLayer> mVrLayerList;
    void addLayer(const GLProgramTexture::TexturedStereoMeshData& meshData, GLuint textureId, bool isExternalTexture=false, HEAD_TRACKING headTracking=FULL);
    void addLayer(const GLProgramTexture::TexturedMeshData& meshData, GLuint textureId, bool isExternalTexture=false, HEAD_TRACKING headTracking=FULL){
        addLayer(GLProgramTexture::convert(meshData),textureId,isExternalTexture,headTracking);
    }
    void setLayerTextureId(ssize_t idx,GLuint textureId){
        mVrLayerList.at(idx).textureId=textureId;
    }
    void removeLayers();
    void drawLayers(gvr::Eye eye);
    // Add a 2D layer at position (0,0,Z) and (width,height) in VR 3D space.
    void addLayer2DCanvas(float z,float width,float height,GLuint textureId, bool isExternalTexture=false,HEAD_TRACKING headTracking=FULL);
    // Add a 360° video sphere
    void addLayerSphere360(float radius,UvSphere::MEDIA_FORMAT format,GLuint textureId, bool isExternalTexture=false);
public:
    // The left/right eye viewport is exactly the area covered when splitting the screen in half
    // while holding the device in landscape mode
    std::array<int,4> getViewportForEye(gvr::Eye eye){
        if(eye==GVR_LEFT_EYE){
            return {0,0,EYE_VIEWPORT_W,EYE_VIEWPORT_H};
        }
        return {EYE_VIEWPORT_W,0,EYE_VIEWPORT_W,EYE_VIEWPORT_H};
    }
public:
    //This one does not use the inverse and is therefore (relatively) slow compared to when
    //using the approximate inverse
    glm::vec2 UndistortedNDCForDistortedNDC(const glm::vec2& in_ndc,int eye)const{
        const auto ret= MLensDistortion::UndistortedNDCForDistortedNDC(mDistortion,mDataUnDistortion.screen_params[eye],mDataUnDistortion.texture_params[eye],{in_ndc.x,in_ndc.y},false);
        return glm::vec2(ret[0],ret[1]);
    }
    static std::array<float,4> reverseFOV(const std::array<float,4>& fov){
        return {fov[1],fov[0],fov[2],fov[3]};
    }
    glm::vec3 UndistortedCoordinatesFor3DPoint(const gvr::Eye eye, const glm::vec3 point, const glm::mat4 headSpaceFromStartSPaceRotation= glm::mat4(1.0f)){
        const int EYE_IDX=eye==GVR_LEFT_EYE ? 0 : 1;
        const auto MVMatrix=eyeFromHead[EYE_IDX]*headSpaceFromStartSPaceRotation;
        const glm::vec4 lola= VDDC::CalculateVertexPosition(mDataUnDistortion.radialDistortionCoefficients,
               mDataUnDistortion.screen_params[EYE_IDX],mDataUnDistortion.texture_params[EYE_IDX],MVMatrix,
                mProjectionM[EYE_IDX],glm::vec4(point,1.0f));
        // Dang what about w ?
        return glm::vec4(glm::vec3(lola)/lola.w,1.0);
        //MLOGD<<"w value"<<gl_Position.w;
    }
    // Distort the mesh for the selected perspective from either the left or right eye perspective
    TexturedMeshData distortMesh(const gvr::Eye eye,const TexturedMeshData& input){
        auto tmp=input;
        //if(input.hasIndices()){
        //    MLOGD<<"Merging indices into vertices";
        //    tmp.mergeIndicesIntoVertices();
        //}
        const int EYE_IDX=eye==GVR_LEFT_EYE ? 0 : 1;
        for(auto& vertex : tmp.vertices){
            const glm::vec3 pos=glm::vec3(vertex.x,vertex.y,vertex.z);
            const glm::vec3 newPos= UndistortedCoordinatesFor3DPoint(eye, pos);
            vertex.x=newPos.x;
            vertex.y=newPos.y;
            vertex.z=newPos.z;
        }
        return tmp;
    }
    // When Rendering the OpenGL layers the following OpenGL params
    // have to be set
    static void setGLParamsWhenRenderingLayers(){
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
    }
private:
    std::array<Chronometer,2> cpuTime={Chronometer{"CPU left"},Chronometer{"CPU right"}};
};


#endif //FPV_VR_OS_VRCOMPOSITORRENDERER_H
