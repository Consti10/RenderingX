#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-flp30-c"
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

void VRHeadsetParams::updateHeadsetParams(const MDeviceParams &mDP,int screenWidthP,int screenHeightP) {
    this->screenWidthP=screenWidthP;
    this->screenHeightP=screenHeightP;
    LOGD("%s",MLensDistortion::MDeviceParamsAsString(mDP).c_str());
    mDistortion=MPolynomialRadialDistortion(mDP.radial_distortion_params);
    //Workaround for bug
    const std::array<float,2> daydreamV2{0.4331F,-0.0856F};
    if(mDP.radial_distortion_params.at(0)==daydreamV2[0] && mDP.radial_distortion_params.at(1)==daydreamV2[1]){
        std::vector<float> daydreamV1{0.385F,0.593F};
        mDistortion=MPolynomialRadialDistortion(daydreamV1);
        LOGD("Daydream v2 distortion params detected. Using v1 instead");
    }
    const auto GetYEyeOffsetMeters= MLensDistortion::GetYEyeOffsetMeters(mDP.vertical_alignment,
                                                                         mDP.tray_to_lens_distance,
                                                                         mDP.screen_height_meters);
    const auto fovLeft= MLensDistortion::CalculateFov(mDP.device_fov_left, GetYEyeOffsetMeters,
                                                      mDP.screen_to_lens_distance,
                                                      mDP.inter_lens_distance,
                                                      mDistortion,
                                                      mDP.screen_width_meters, mDP.screen_height_meters);
    const auto fovRight=MLensDistortion::reverseFOV(fovLeft);

    MLensDistortion::CalculateViewportParameters_NDC(0, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovLeft,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     screen_params[0], texture_params[0]);
    MLensDistortion::CalculateViewportParameters_NDC(1, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovRight,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     screen_params[1], texture_params[1]);
    LOGD("%s",mDistortion.toString().c_str());
    LOGD("Left Eye: %s",MLensDistortion::ViewportParamsAsString(screen_params[0],texture_params[0]).c_str());
    LOGD("Right Eye: %s", MLensDistortion::ViewportParamsAsString(screen_params[1],texture_params[1]).c_str());

    /*LOGD("Jetzt gehts los");
    MLensDistortion::UndistortedNDCForDistortedNDC(mDistortion,screen_params[0],texture_params[0],{1,1},
                                                   false);
    LOGD("JGL end");

    const float maxX=1*texture_params[0].width-texture_params[0].x_eye_offset;
    const float maxY=1*texture_params[0].height-texture_params[0].y_eye_offset;
    const float maxR=sqrt(maxX*maxX+maxY*maxY);
    const float maxR2=mDistortion.DistortRadiusInverse(maxR);
    LOGD("Max X Y %f %f |  maxR %f maxR2 %f",maxX,maxY,maxR,maxR2);*/

    //The daydream headset v2 distortion function is the only function where we cannot achieve a proper maxRadSq value
    //with a deviation of less than 0.001f

    MAX_RAD_SQ=1.0f;
    for(float i=1.0f;i<=2.0f;i+=0.01f){
        const auto& inverse=mDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
        const float maxDeviation=MPolynomialRadialDistortion::calculateMaxDeviation(mDistortion,inverse,sqrt(MAX_RAD_SQ));
        if(maxDeviation<=0.002f){
            MAX_RAD_SQ=i;
        }
    }
    LOGD("Max Rad Sq%f",MAX_RAD_SQ);
    mInverse=mDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
    mProjectionM[0]=perspective(fovLeft,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    mProjectionM[1]=perspective(fovRight,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    const float inter_lens_distance=mDP.inter_lens_distance;
    eyeFromHead[0]=glm::translate(glm::mat4(1.0f),glm::vec3(inter_lens_distance*0.5f,0,0));
    eyeFromHead[1]=glm::translate(glm::mat4(1.0f),glm::vec3(-inter_lens_distance*0.5f,0,0));

    //as long as the inverse is strict monotone increasing, we can increase the maxRadSq value
    /*float lastValue=0;
    for(float r2=MAX_RAD_SQ;r2<3.0f;r2+=0.01f){
        const float v=mInverse.DistortRadius(sqr)
    }*/
    MAX_RAD_SQ=3.0f;
}


void VRHeadsetParams::updateDistortionManager(DistortionManager &distortionManager) {
    distortionManager.updateDistortion(mInverse,MAX_RAD_SQ,screen_params,texture_params);
}

void VRHeadsetParams::updateLatestHeadSpaceFromStartSpaceRotation() {
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

glm::mat4 VRHeadsetParams::GetProjectionMatrix(gvr::Eye eye) {
    return mProjectionM[eye];
}

void VRHeadsetParams::setOpenGLViewport(gvr::Eye eye) {
    const int ViewPortW=(int)(screenWidthP/2.0f);
    const int ViewPortH=screenHeightP;
    if(eye==0){
        glViewport(0,0,ViewPortW,ViewPortH);
    }else{
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
    }
}

#pragma clang diagnostic pop