//
// Created by geier on 08/04/2020.
//

#ifndef RENDERINGX_MVRHEADSETPARAMS_HPP
#define RENDERINGX_MVRHEADSETPARAMS_HPP

#include <vector>
#include <array>
#include <sstream>
#include <NDKJavaClassMember.hpp>

// Links to java MVrHeadsetParams

struct MVrHeadsetParams{
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

static MVrHeadsetParams createFromJava2(JNIEnv *env, jobject androidContext){
    // First, Construct an instance of the java class to obtain values from
    jclass jcMVrHeadsetParams = env->FindClass("constantin/renderingx/core/MVrHeadsetParams");
    assert(jcMVrHeadsetParams!=nullptr);
    // find the constructor that takes an context
    jmethodID constructor = env->GetMethodID( jcMVrHeadsetParams, "<init>", "(Landroid/content/Context;)V" );
    assert(constructor!=nullptr);
    jobject instanceMVrHeadsetParams = env->NewObject(jcMVrHeadsetParams,constructor,androidContext);
    assert(instanceMVrHeadsetParams!= nullptr);
    ClassMemberFromJava helper(env, instanceMVrHeadsetParams);
    return {
            helper.get<float>("ScreenWidthMeters"),
            helper.get<float>("ScreenHeightMeters"),
            helper.get<float>("ScreenToLensDistance"),
            helper.get<float>("InterLensDistance"),
            helper.get<int>("VerticalAlignment"),
            helper.get<float>("VerticalDistanceToLensCenter"),
            helper.getFloatArrayFixed<4>("fov"),
            helper.get<std::vector<float>>("kN"),
            helper.get<int>("ScreenWidthPixels"),
            helper.get<int>("ScreenHeightPixels")
    };
}

static std::string MyVrHeadsetParamsAsString(const MVrHeadsetParams& dp) {
    std::stringstream ss;
    ss<<"MVrHeadsetParams:\n";
    ss<<"screen_width_meters "<<dp.screen_width_meters<<" ";
    ss<<"screen_height_meters "<<dp.screen_height_meters<<"\n";
    ss<<"screen_to_lens_distance "<<dp.screen_to_lens_distance<<" ";
    ss<<"inter_lens_distance "<<dp.inter_lens_distance<<"\n";
    ss<<"vertical_alignment "<<dp.vertical_alignment<<"";
    ss<<"tray_to_lens_distance "<<dp.tray_to_lens_distance<<"\n";
    ss<<"device_fov_left ["<<dp.device_fov_left[0]<<","<<dp.device_fov_left[0]<<","<<dp.device_fov_left[0]<<","<<dp.device_fov_left[0]<<")\n";
    ss<<"radial_distortion_params"<<PolynomialRadialDistortion(dp.radial_distortion_params).toString()<<"\n";
    ss<<"screen_width_pixels"<<dp.screen_width_pixels<<"\n";
    ss<<"screen_height_pixels"<<dp.screen_height_pixels<<"";
    return ss.str();
}

#endif //RENDERINGX_MVRHEADSETPARAMS_HPP
