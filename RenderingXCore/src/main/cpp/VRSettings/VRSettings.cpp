//
// Created by Constantin on 1/29/2019.
//

#include <array>
#include <sstream>
#include <SharedPreferences.hpp>
#include <AndroidLogger.hpp>
#include "VRSettings.h"
#include "IDVR.hpp"

VRSettings::VRSettings(JNIEnv *env, jobject androidContext) {
    SharedPreferences settingsN(env,androidContext,"pref_vr_rendering");

    VR_SCENE_SCALE_PERCENTAGE=settingsN.getFloat(IDVR::VR_SCENE_SCALE_PERCENTAGE,100.0F);
    //Default on (0==on, 1==off)
    VR_DISTORTION_CORRECTION_MODE=settingsN.getInt(IDVR::VR_DISTORTION_CORRECTION_MODE,VR_DISTORTION_CORRECTION_MODE_ENABLED);
    //
    VR_ENABLE_DEBUG=settingsN.getBoolean(IDVR::VR_ENABLE_DEBUG,false);

    //GHT_MODE=settingsN.getInt(IDVR::GroundHeadTrackingMode);
    //bool headTracking=GHT_MODE!=0;
    GroundHeadTrackingEnable=settingsN.getBoolean(IDVR::GroundHeadTrackingEnable);
    GHT_X=settingsN.getBoolean(IDVR::GroundHeadTrackingX);
    GHT_Y=settingsN.getBoolean(IDVR::GroundHeadTrackingY);
    GHT_Z=settingsN.getBoolean(IDVR::GroundHeadTrackingZ);
    GHT_OSD_FIXED_TO_HEAD=settingsN.getBoolean(IDVR::GHT_OSD_FIXED_TO_HEAD,false);

    VR_RENDERING_MODE=settingsN.getInt(IDVR::VR_RENDERING_MODE,0);

    //MLOGD<<VR_SCENE_SCALE_PERCENTAGE<<" "<<VR_DISTORTION_CORRECTION_MODE;
}


