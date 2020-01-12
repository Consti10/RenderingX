//
// Created by Constantin on 1/9/2020.
//

#ifndef FPV_VR_2018_VRHEADSETPARAMS_H
#define FPV_VR_2018_VRHEADSETPARAMS_H

#include <glm/mat4x4.hpp>
#include "MLensDistortion.h"
#include "DistortionManager.h"

class VRHeadsetParams {
public:
    void setGvrApi(gvr::GvrApi* gvrApi);
private:
    gvr::GvrApi *gvr_api;
    glm::mat4 eyeFromHead[2];
    glm::mat4 mProjectionM[2];
public:
    int screenWidthP=1920;
    int screenHeightP=1080;
public:
    static constexpr float MIN_Z_DISTANCE=0.1f;
    static constexpr float MAX_Z_DISTANCE=100.0f;
public:
    std::array<MLensDistortion::ViewportParams,2> screen_params;
    std::array<MLensDistortion::ViewportParams,2> texture_params;
    MPolynomialRadialDistortion mInverse{{0}};
    float MAX_RAD_SQ;
public:
    void updateHeadsetParams(const MDeviceParams& mDP,int screenWidthP,int screenHeightP);
    void updateDistortionManager(DistortionManager& distortionManager);
public:
    //we do not want the view (rotation) to change during rendering of one frame/eye
    //else we could end up with multiple elements rendered in different perspectives
    void updateLatestHeadSpaceFromStartSpaceRotation();
    //returns the latest 'cached' head rotation
    glm::mat4 GetLatestHeadSpaceFromStartSpaceRotation();
    gvr::Mat4f GetLatestHeadSpaceFromStartSpaceRotation_();
private:
    glm::mat4 latestHeadSpaceFromStartSpaceRotation;
    gvr::Mat4f latestHeadSpaceFromStartSpaceRotation_;
public:
    glm::mat4 GetEyeFromHeadMatrix(gvr::Eye eye);

    glm::mat4 GetProjectionMatrix(gvr::Eye eye);

    //Set the viewport to exactly half framebuffer size
    //where framebuffer size==screen size
    void setOpenGLViewport(gvr::Eye eye);
};


#endif //FPV_VR_2018_VRHEADSETPARAMS_H
