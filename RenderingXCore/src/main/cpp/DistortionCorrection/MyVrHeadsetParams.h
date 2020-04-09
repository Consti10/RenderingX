//
// Created by geier on 08/04/2020.
//

#ifndef RENDERINGX_MYVRHEADSETPARAMS_H
#define RENDERINGX_MYVRHEADSETPARAMS_H

#include <vector>
#include <array>
#include <sstream>

// Links to java MyVrHeadsetParams

struct MyVrHeadsetParams{
    const float screen_width_meters;
    const float screen_height_meters;
    const float screen_to_lens_distance;
    const float inter_lens_distance;
    const int vertical_alignment;
    const float tray_to_lens_distance;
    const std::array<float,4> device_fov_left;
    const std::vector<float> radial_distortion_params;
    const int screen_width_pixels;
    const int screen_height_pixels;
};

static MyVrHeadsetParams createFromJava(JNIEnv *env, jobject MyVrHeadsetParams){
    LOGD("Test %s",NDKHelper::getJavaNameForPrimitive<float>());
    jclass c =env->GetObjectClass(MyVrHeadsetParams);
    return {
            NDKHelper::getClassMemberValue<float>(env,c,MyVrHeadsetParams,"ScreenWidthMeters"),
            NDKHelper::getClassMemberValue<float>(env,c,MyVrHeadsetParams,"ScreenHeightMeters"),
            NDKHelper::getClassMemberValue<float>(env,c,MyVrHeadsetParams,"ScreenToLensDistance"),
            NDKHelper::getClassMemberValue<float>(env,c,MyVrHeadsetParams,"InterLensDistance"),
            NDKHelper::getClassMemberValue<int>(env,c,MyVrHeadsetParams,"VerticalAlignment"),
            NDKHelper::getClassMemberValue<float>(env,c,MyVrHeadsetParams,"VerticalDistanceToLensCenter"),
            NDKHelper::getClassMemberValueArray<4>(env,c,MyVrHeadsetParams,"fov"),
            NDKHelper::getClassMemberValueVector(env,c,MyVrHeadsetParams,"kN"),
            NDKHelper::getClassMemberValue<int>(env,c,MyVrHeadsetParams,"ScreenWidthPixels"),
            NDKHelper::getClassMemberValue<int>(env,c,MyVrHeadsetParams,"ScreenHeightPixels")
    };
}

static std::string MyVrHeadsetParamsAsString(const MyVrHeadsetParams& dp) {
    std::stringstream ss;
    ss<<"MyVrHeadsetParams:\n";
    ss<<"screen_width_meters "<<dp.screen_width_meters<<" ";
    ss<<"screen_height_meters "<<dp.screen_height_meters<<"\n";
    ss<<"screen_to_lens_distance "<<dp.screen_to_lens_distance<<" ";
    ss<<"inter_lens_distance "<<dp.inter_lens_distance<<"\n";
    ss<<"vertical_alignment "<<dp.vertical_alignment<<"";
    ss<<"tray_to_lens_distance "<<dp.tray_to_lens_distance<<"\n";
    ss<<"device_fov_left ["<<dp.device_fov_left[0]<<","<<dp.device_fov_left[0]<<","<<dp.device_fov_left[0]<<","<<dp.device_fov_left[0]<<")\n";
    ss<<"radial_distortion_params"<<PolynomialRadialDistortion(dp.radial_distortion_params).toString()<<"";
    return ss.str();
}

#endif //RENDERINGX_MYVRHEADSETPARAMS_H
