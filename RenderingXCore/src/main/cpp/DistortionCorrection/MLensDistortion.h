//
// Created by Consti10 on 04/12/2019.
//

#ifndef CARDBOARD_MLENSDISTORTION_H
#define CARDBOARD_MLENSDISTORTION_H

#include <array>
#include <memory>
#include <math.h>

#include "MPolynomialRadialDistortion.h"

//Based on @cardboard/LensDistortion

constexpr float kDefaultBorderSizeMeters = 0.003f;

struct MDeviceParams{
    const float screen_width_meters;
    const float screen_height_meters;
    const float screen_to_lens_distance;
    const float inter_lens_distance;
    const int vertical_alignment;
    const float tray_to_lens_distance;
    const std::array<float,4> device_fov_left;
    const std::vector<float> radial_distortion_params;
};

// All values in tanangle units.
class MLensDistortion {
public:
    struct ViewportParams{
        float width;
        float height;
        float x_eye_offset;
        float y_eye_offset;
    };
    static std::array<float,4> reverseFOV(const std::array<float,4>& fov){
        return {fov[1],fov[0],fov[2],fov[3]};
    }
public:
    static float GetYEyeOffsetMeters(const int vertical_alignment,
                                     const float tray_to_lens_distance,
                                     const float screen_height_meters);
    static std::array<float, 4> CalculateFov(
            const std::array<float, 4> device_fov,
            const float GetYEyeOffsetMeters,
            const float screen_to_lens_distance, const float inter_lens_distance,
            const MPolynomialRadialDistortion &distortion, float screen_width_meters,
            float screen_height_meters);
    static void CalculateViewportParameters(int eye,
                                            const float GetYEyeOffsetMeters,
                                            const float screen_to_lens_distance,
                                            const float inter_lens_distance,
                                            const std::array<float, 4> &fov,
                                            float screen_width_meters,
                                            float screen_height_meters,
                                            ViewportParams *screen_params,
                                            ViewportParams *texture_params);

    static void CalculateViewportParameters_NDC(int eye,
                                            const float GetYEyeOffsetMeters,
                                            const float screen_to_lens_distance,
                                            const float inter_lens_distance,
                                            const std::array<float, 4> &fov,
                                            float screen_width_meters,
                                            float screen_height_meters,
                                            ViewportParams& screen_params,
                                            ViewportParams& texture_params);


    static std::array<float, 2> UndistortedUvForDistortedUv(
            const MPolynomialRadialDistortion &distortion,
            const ViewportParams &screen_params, const ViewportParams &texture_params,
            const std::array<float, 2> &in);

    //same as UndistortedUvForDistortedUv, but takes the inverse polynomial instead such that the distorted point
    //can be calulated with less computational effort
    static std::array<float, 2> UndistortedUvForDistortedUv_Inverse(
            const MPolynomialRadialDistortion &inverseDistortion,
            const ViewportParams &screen_params, const ViewportParams &texture_params,
            const std::array<float, 2> &in);


    //This one takes and returns values between -1 and 1 (e.g. OpenGL viewport coordinates)
    //Viewport Parameters have to be calculated accordingly @CalculateViewportParameters_NDC
    static std::array<float, 2> UndistortedNDCForDistortedNDC(
            const MPolynomialRadialDistortion &inverseDistortion,
            const ViewportParams &screen_params, const ViewportParams &texture_params,
            const std::array<float, 2> &in);

    static std::string MDeviceParamsAsString(const MDeviceParams& dp);

};




#endif //CARDBOARD_MLENSDISTORTION_H
