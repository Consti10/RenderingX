
//
// Created by Constantin on 1/9/2020.
//

#include <string>
#include <AndroidLogger.hpp>
#include <MatrixHelper.h>
#include "DistortionEngine.h"
#include "VDDCManager.h"
#include "XTestDistortion.h"


void DistortionEngine::setGvrApi(gvr::GvrApi *gvrApi) {
    this->gvr_api=gvrApi;
}

void DistortionEngine::updateHeadsetParams(const MVrHeadsetParams &mDP) {
    this->screenWidthP=mDP.screen_width_pixels;
    this->screenHeightP=mDP.screen_height_pixels;
    MLOGD<<MyVrHeadsetParamsAsString(mDP);
    mDistortion=PolynomialRadialDistortion(mDP.radial_distortion_params);

    const auto GetYEyeOffsetMeters= MLensDistortion::GetYEyeOffsetMeters(mDP.vertical_alignment,
                                                                         mDP.tray_to_lens_distance,
                                                                         mDP.screen_height_meters);
    const auto fovLeft= MLensDistortion::CalculateFov(mDP.device_fov_left, GetYEyeOffsetMeters,
                                                      mDP.screen_to_lens_distance,
                                                      mDP.inter_lens_distance,
                                                      mDistortion,
                                                      mDP.screen_width_meters, mDP.screen_height_meters);
    const auto fovRight=DistortionEngine::reverseFOV(fovLeft);

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
    MLOGD<<mDistortion.toString();
    MLOGD<<"Left Eye: "<<MLensDistortion::ViewportParamsAsString(screen_params[0],texture_params[0]);
    MLOGD<<"Right Eye: "<<MLensDistortion::ViewportParamsAsString(screen_params[1],texture_params[1]);

    //TODO calculate right maxRadSq
    /*const float maxX=1*texture_params[0].width+texture_params[0].x_eye_offset;
    const float maxY=1*texture_params[0].height+texture_params[0].y_eye_offset;
    const float maxR2=maxX*maxX+maxY*maxY;
    const float maxR=sqrt(maxR2);
    const float iMaxRad=mDistortion.DistortRadiusInverse(maxR);
    LOGD("Max X Y %f %f |  maxR2 %f maxR %f iMaxRad %f",maxX,maxY,maxR2,maxR,iMaxRad);*/

    //Find the maximum value we can use to create a inverse polynomial distortion that
    //never has a deviation higher from x in the range [0..maxRangeInverse]
    float maxRangeInverse=1.0f;
    for(float i=1.0f;i<=2.0f;i+=0.01f){
        const auto inverse=PolynomialRadialInverse(mDistortion, maxRangeInverse, VDDCManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
        const float maxDeviation=PolynomialRadialInverse::calculateMaxDeviation(mDistortion,inverse,maxRangeInverse);
        if(maxDeviation<=0.001f){
            maxRangeInverse=i;
        }
    }
    MLOGD<<"Max value used for getApproximateInverseDistortion()"<<maxRangeInverse;
    mInverse=PolynomialRadialInverse(mDistortion, maxRangeInverse, VDDCManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
    MLOGD<<"Inverse is:"<<mInverse.toStringX();

    //as long as the function is still strict monotonic increasing we can increase the value that will be used for
    //clamping later in the vertex shader.
    float MAX_RAD_SQ=0;
    float last=mInverse.DistortRadius(MAX_RAD_SQ);
    for(float i=MAX_RAD_SQ;i<3;i+=0.01f){
        const float d=mInverse.DistortRadius(i);
        if(d>=last){
            //LOGD("Increasing maxRadSq %f",MAX_RAD_SQ);
            MAX_RAD_SQ=i;
            last=d;
        }else{
            //LOGD("Cannot increase %f %f",last,d);
            last=d;
            break;
        }
    }
    mInverse.maxRadSq=MAX_RAD_SQ;

    mProjectionM[0]=perspective(fovLeft,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    mProjectionM[1]=perspective(fovRight,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    const float inter_lens_distance=mDP.inter_lens_distance;
    eyeFromHead[0]=glm::translate(glm::mat4(1.0f),glm::vec3(inter_lens_distance*0.5f,0,0));
    eyeFromHead[1]=glm::translate(glm::mat4(1.0f),glm::vec3(-inter_lens_distance*0.5f,0,0));

    //
    //test2();
    //test3();
}


void DistortionEngine::updateDistortionManager(VDDCManager &distortionManager)const {
    distortionManager.updateDistortion(mInverse,screen_params,texture_params);
}

void DistortionEngine::updateLatestHeadSpaceFromStartSpaceRotation() {
    latestHeadSpaceFromStartSpaceRotation_=gvr_api->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    latestHeadSpaceFromStartSpaceRotation=toGLM(latestHeadSpaceFromStartSpaceRotation_);
}

glm::mat4 DistortionEngine::GetLatestHeadSpaceFromStartSpaceRotation()const{
    return latestHeadSpaceFromStartSpaceRotation;
}

gvr::Mat4f DistortionEngine::GetLatestHeadSpaceFromStartSpaceRotation_()const {
    return latestHeadSpaceFromStartSpaceRotation_;
}

glm::mat4 DistortionEngine::GetEyeFromHeadMatrix(gvr::Eye eye)const{
    return eyeFromHead[eye];
}

glm::mat4 DistortionEngine::GetProjectionMatrix(gvr::Eye eye)const{
    return mProjectionM[eye];
}

void DistortionEngine::setOpenGLViewport(gvr::Eye eye) {
    const int ViewPortW=(int)(screenWidthP/2.0f);
    const int ViewPortH=screenHeightP;
    if(eye==0){
        glViewport(0,0,ViewPortW,ViewPortH);
    }else{
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
    }
}
