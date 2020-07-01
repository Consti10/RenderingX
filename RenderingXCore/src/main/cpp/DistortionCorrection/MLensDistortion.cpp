//
// Created by Consti10 on 04/12/2019.
//

#include <sstream>
#include <AndroidLogger.hpp>
#include "MLensDistortion.h"

float MLensDistortion::GetYEyeOffsetMeters(const int vertical_alignment,
                                           const float tray_to_lens_distance,
                                           const float screen_height_meters) {
     MLOGD<<"Vertical alignment "<<vertical_alignment<<" tray_to_lens_distance "<<tray_to_lens_distance;
    switch (vertical_alignment) {
        case 1:
        default:
            return screen_height_meters / 2.0f;
        case 0:
            return tray_to_lens_distance - kDefaultBorderSizeMeters;
        case 2:
            return screen_height_meters - tray_to_lens_distance -
                   kDefaultBorderSizeMeters;
    }
}

std::array<float, 4> MLensDistortion::CalculateFov(
        const std::array<float, 4> device_fov,
        const float GetYEyeOffsetMeters,
        const float screen_to_lens_distance, const float inter_lens_distance,
        const PolynomialRadialDistortion &distortion, float screen_width_meters,
        float screen_height_meters) {
    const float eye_to_screen_distance = screen_to_lens_distance;
    const float outer_distance =
            (screen_width_meters - inter_lens_distance) / 2.0f;
    const float inner_distance = inter_lens_distance / 2.0f;
    const float bottom_distance = GetYEyeOffsetMeters;
    const float top_distance = screen_height_meters - bottom_distance;

    const float outer_angle =static_cast<float>(
            atan(
                    distortion.Distort({outer_distance / eye_to_screen_distance, 0})[0]) *
            180.0f / M_PI);
    const float inner_angle =static_cast<float>(
            atan(
                    distortion.Distort({inner_distance / eye_to_screen_distance, 0})[0]) *
            180.0f / M_PI);
    const float bottom_angle =static_cast<float>(
            atan(distortion.Distort(
                    {0, bottom_distance / eye_to_screen_distance})[1]) *
            180.0f / M_PI);
    const float top_angle =static_cast<float>(
            atan(distortion.Distort({0, top_distance / eye_to_screen_distance})[1]) *
            180.0f / M_PI);

    const std::array<float,4> ret={
            std::min(outer_angle, device_fov[0]),
            std::min(inner_angle, device_fov[1]),
            std::min(bottom_angle, device_fov[2]),
            std::min(top_angle, device_fov[3]),
    };
    //LOGD("FOVz %f %f %f %f",ret[0],ret[1],ret[2],ret[3]);
    //const float fov=45;
    //return {fov,fov,fov,fov};
    //fov of e.g 20 results in not covering the whole screen
    //return {ret[1],ret[0],ret[3],ret[2]};
    MLOGD<<"FOV (L,R,B,T) ("<<ret[0]<<","<<ret[1]<<","<<ret[2]<<","<<ret[3]<<")";
    //return {50,30,40,40};
    return ret;
}



void MLensDistortion::CalculateViewportParameters(
        int eye,
        const float GetYEyeOffsetMeters,
        const float screen_to_lens_distance,
        const float inter_lens_distance,
        const std::array<float, 4> &fov, float screen_width_meters,
        float screen_height_meters, ViewportParams *screen_params,
        ViewportParams *texture_params) {
    screen_params->width =
            screen_width_meters / screen_to_lens_distance;
    screen_params->height =
            screen_height_meters / screen_to_lens_distance;

    screen_params->x_eye_offset =
            eye == 0
            ? ((screen_width_meters - inter_lens_distance) / 2) /
              screen_to_lens_distance
            : ((screen_width_meters + inter_lens_distance) / 2) /
              screen_to_lens_distance;
    //? 1.0f : 2.0f;
    screen_params->y_eye_offset =
            GetYEyeOffsetMeters /
            screen_to_lens_distance;

    texture_params->width = static_cast<float>(tan(fov[0] * M_PI / 180) + tan(fov[1] * M_PI / 180));
    texture_params->height = static_cast<float>(tan(fov[2] * M_PI / 180) + tan(fov[3] * M_PI / 180));

    texture_params->x_eye_offset = static_cast<float>(tan(fov[0] * M_PI / 180));
    texture_params->y_eye_offset = static_cast<float>(tan(fov[2] * M_PI / 180));
}

std::array<float, 2> MLensDistortion::UndistortedUvForDistortedUv(
        const PolynomialRadialDistortion &distortion,
        const ViewportParams &screen_params, const ViewportParams &texture_params,
        const std::array<float, 2> &in,const bool isInverse){
    // Convert input from normalized [0, 1] pre distort texture space to
    // eye-centered tanangle units.
    std::array<float, 2> distorted_uv_tanangle = {
            in[0] * texture_params.width - texture_params.x_eye_offset,
            in[1] * texture_params.height - texture_params.y_eye_offset};

    std::array<float, 2> undistorted_uv_tanangle = isInverse ?
                                                   distortion.Distort(distorted_uv_tanangle):
                                                   distortion.DistortInverse(distorted_uv_tanangle);

    // Convert output from tanangle units to normalized [0, 1] screen coordinates.
    return {(undistorted_uv_tanangle[0] + screen_params.x_eye_offset) /
            screen_params.width,
            (undistorted_uv_tanangle[1] + screen_params.y_eye_offset) /
            screen_params.height};
}



void MLensDistortion::CalculateViewportParameters_NDC(
        int eye,
        const float GetYEyeOffsetMeters,
        const float screen_to_lens_distance,
        const float inter_lens_distance,
        const std::array<float, 4> &fov, float screen_width_meters,
        float screen_height_meters, ViewportParamsNDC& screen_params,
        ViewportParamsNDC& texture_params) {
    screen_params.width =
            4 / (screen_width_meters / screen_to_lens_distance);
    screen_params.height =
            2 / (screen_height_meters / screen_to_lens_distance);

    if(eye==0){
        screen_params.x_eye_offset=((screen_width_meters-inter_lens_distance)*2 / screen_width_meters)-1;
    }else{
        screen_params.x_eye_offset=((screen_width_meters+inter_lens_distance)*2 / screen_width_meters)-3;
    }
    screen_params.y_eye_offset=(GetYEyeOffsetMeters*2/screen_height_meters)-1;

    texture_params.width = static_cast<float>(tan(fov[0] * M_PI / 180) + tan(fov[1] * M_PI / 180))*0.5f;
    texture_params.height = static_cast<float>(tan(fov[2] * M_PI / 180) + tan(fov[3] * M_PI / 180))*0.5f;

    texture_params.x_eye_offset = -(static_cast<float>((tan(fov[0] * M_PI / 180)-tan(fov[1] * M_PI / 180))*0.5));
    texture_params.y_eye_offset = -(static_cast<float>((tan(fov[2] * M_PI / 180)-tan(fov[3] * M_PI / 180))*0.5));
}

//Almost the same as the original (inverse) version, but note we do a
// x*a+b instead of (x+a)/b when transforming back to screen coordinates
std::array<float, 2> MLensDistortion::UndistortedNDCForDistortedNDC(
        const PolynomialRadialDistortion &inverseDistortion,
        const MLensDistortion::ViewportParamsNDC &screen_params,
        const MLensDistortion::ViewportParamsNDC &texture_params, const std::array<float, 2> &in,const bool isInverse) {
    std::array<float, 2> distorted_ndc_tanangle = {
            in[0] * texture_params.width + texture_params.x_eye_offset,
            in[1] * texture_params.height + texture_params.y_eye_offset};

    const float r2=distorted_ndc_tanangle[0]*distorted_ndc_tanangle[0]+
            distorted_ndc_tanangle[1]*distorted_ndc_tanangle[1];
    //if(r2>=3.203659-0.001f){
    //    return std::array<float,2>{0,0};
    //}

    std::array<float, 2> undistorted_ndc_tanangle = isInverse ?
            inverseDistortion.Distort(distorted_ndc_tanangle):
            inverseDistortion.DistortInverse(distorted_ndc_tanangle);

    std::array<float,2> ret={
            undistorted_ndc_tanangle[0]*screen_params.width+screen_params.x_eye_offset,
            undistorted_ndc_tanangle[1]*screen_params.height+screen_params.y_eye_offset};
    return ret;
}


std::string
MLensDistortion::ViewportParamsAsString(const MLensDistortion::ViewportParams &screen_params,
                                        const MLensDistortion::ViewportParams &texture_params) {
    std::stringstream ss;
    ss<<"Screen_params width:"<<screen_params.width<<" height:"<<screen_params.height<<"\n";
    ss<<"Screen_params offset X:"<<screen_params.x_eye_offset<<" Y:"<<screen_params.y_eye_offset<<"\n";
    ss<<"Texture_params width:"<<texture_params.width<<" height:"<<texture_params.height<<"\n";
    ss<<"Texture_params offset X:"<<texture_params.x_eye_offset<<" Y:"<<texture_params.y_eye_offset<<"\n";
    return ss.str();
}
static const MLensDistortion::ViewportParams convert(const MLensDistortion::ViewportParamsNDC& vp){
    return {vp.width,vp.height,vp.x_eye_offset,vp.y_eye_offset};
}
std::string
MLensDistortion::ViewportParamsNDCAsString(const MLensDistortion::ViewportParamsNDC &screen_params,
                                        const MLensDistortion::ViewportParamsNDC &texture_params) {
    return ViewportParamsAsString(convert(screen_params),convert(texture_params));
}
