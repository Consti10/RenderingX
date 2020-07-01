//
// Created by Constantin on 1/9/2020.
//

#ifndef FPV_VR_2018_VRHEADSETPARAMS_H
#define FPV_VR_2018_VRHEADSETPARAMS_H

#include <glm/mat4x4.hpp>
#include "MLensDistortion.h"
#include "VDDC.h"
#include "MatrixHelper.h"

//TODO find better name ?

class DistortionEngine {
public:
    void setGvrApi(gvr::GvrApi* gvrApi);
private:
    gvr::GvrApi *gvr_api;
    glm::mat4 eyeFromHead[2];
    glm::mat4 mProjectionM[2];
    glm::mat4 latestHeadSpaceFromStartSpaceRotation=glm::mat4(1.0f);
    gvr::Mat4f latestHeadSpaceFromStartSpaceRotation_=toGVR(latestHeadSpaceFromStartSpaceRotation);
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


#endif //FPV_VR_2018_VRHEADSETPARAMS_H
