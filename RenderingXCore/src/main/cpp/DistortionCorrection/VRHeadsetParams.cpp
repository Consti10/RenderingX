//
// Created by Constantin on 1/9/2020.
//

#include <string>
#include <Helper/MDebug.hpp>
#include <MatrixHelper.h>
#include "VRHeadsetParams.h"
#include "DistortionManager.h"

void VRHeadsetParams::setGvrApi(gvr::GvrApi *gvrApi) {
    this->gvr_api=gvrApi;
}


void VRHeadsetParams::updateHeadsetParams(const MDeviceParams &mDP) {
    LOGD("%s",MLensDistortion::MDeviceParamsAsString(mDP).c_str());
    auto polynomialRadialDistortion=MPolynomialRadialDistortion(mDP.radial_distortion_params);
    //auto polynomialRadialDistortion=MPolynomialRadialDistortion({0.441, 0.156});

    const auto GetYEyeOffsetMeters= MLensDistortion::GetYEyeOffsetMeters(mDP.vertical_alignment,
                                                                         mDP.tray_to_lens_distance,
                                                                         mDP.screen_height_meters);
    const auto fovLeft= MLensDistortion::CalculateFov(mDP.device_fov_left, GetYEyeOffsetMeters,
                                                      mDP.screen_to_lens_distance,
                                                      mDP.inter_lens_distance,
                                                      polynomialRadialDistortion,
                                                      mDP.screen_width_meters, mDP.screen_height_meters);
    const auto fovRight=MLensDistortion::reverseFOV(fovLeft);


    MLensDistortion::CalculateViewportParameters_NDC(0, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovLeft,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     &screen_params[0], &texture_params[0]);
    MLensDistortion::CalculateViewportParameters_NDC(1, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovRight,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     &screen_params[1], &texture_params[1]);

    MAX_RAD_SQ=1.0f;
    bool done=false;
    while(MAX_RAD_SQ<2.0f && !done){
        const auto& inverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
        LOGD("Max Rad Sq%f",MAX_RAD_SQ);
        for(float r=0;r<MAX_RAD_SQ;r+=0.01f) {
            const float deviation = MPolynomialRadialDistortion::calculateDeviation(r,polynomialRadialDistortion,inverse);
            LOGD("r %f | Deviation %f",r,deviation);
            if (deviation > 0.001f) {
                done = true;
                MAX_RAD_SQ-= 0.01f;
                break;
            }
        }
        MAX_RAD_SQ+=0.01f;
    }
    LOGD("Max Rad Sq%f",MAX_RAD_SQ);
    mInverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
    mProjectionM[0]=perspective(fovLeft,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    mProjectionM[1]=perspective(fovRight,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    const float inter_lens_distance=mDP.inter_lens_distance;
    eyeFromHead[0]=glm::translate(glm::mat4(1.0f),glm::vec3(inter_lens_distance*0.5f,0,0));
    eyeFromHead[1]=glm::translate(glm::mat4(1.0f),glm::vec3(-inter_lens_distance*0.5f,0,0));
}

void VRHeadsetParams::updateDistortionManager(DistortionManager &distortionManager) {
    distortionManager.updateDistortion(mInverse,MAX_RAD_SQ,screen_params,texture_params);
}

void VRHeadsetParams::updateHeadView() {
    latestHeadSpaceFromStartSpaceRotation_=gvr_api->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    latestHeadSpaceFromStartSpaceRotation=toGLM(latestHeadSpaceFromStartSpaceRotation_);
}

glm::mat4 VRHeadsetParams::GetLatestHeadSpaceFromStartSpaceRotation() {
    return latestHeadSpaceFromStartSpaceRotation;
}

gvr::Mat4f VRHeadsetParams::GetLatestHeadSpaceFromStartSpaceRotation_() {
    return latestHeadSpaceFromStartSpaceRotation_;
}

glm::mat4 VRHeadsetParams::GetEyeFromHeadMatrix(gvr::Eye eye) {
    return eyeFromHead[eye];
}

gvr::Mat4f VRHeadsetParams::GetEyeFromHeadMatrix_(gvr::Eye eye) {
    return toGVR(GetEyeFromHeadMatrix(eye));
}



glm::mat4 VRHeadsetParams::GetViewMatrix(int eye) {
    return glm::mat4();
}
