//
// Created by Consti10 on 11/12/2019.
//

#include "PolynomialRadialDistortion.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <android/log.h>
#include "LinearAlgebraHelper.hpp"


PolynomialRadialDistortion::PolynomialRadialDistortion(
        const std::vector<float>& coefficients)
        : coefficients_(coefficients) {
}

float PolynomialRadialDistortion::DistortionFactor(const float r_squared) const {
    //__android_log_print(ANDROID_LOG_DEBUG,"TAG","R2 is:%f",r_squared);
    float r_factor = 1.0f;
    float distortion_factor = 1.0f;

    for (float ki : coefficients_) {
        r_factor *= r_squared;
        distortion_factor += ki * r_factor;
    }
    return distortion_factor;
}

float PolynomialRadialDistortion::DistortRadius(float r) const {
    return r * DistortionFactor(r * r);
}

float PolynomialRadialDistortion::DistortRadiusInverse(const float radius)const {
    if (std::fabs(radius - 0.0f) < std::numeric_limits<float>::epsilon()) {
        return 0;
    }
    // Based on the shape of typical distortion curves, |radius| / 2 and
    // |radius| / 3 are good initial guesses for the Secant method that will
    // remain within the intended range of the polynomial.
    float r0 = radius / 2.0f;
    float r1 = radius / 3.0f;
    float dr0 = radius - DistortRadius(r0);
    while (std::fabs(r1 - r0) > 0.0001f /** 0.1mm */) {
        const float dr1 = radius - DistortRadius(r1);
        const float r2 = r1 - dr1 * ((r1 - r0) / (dr1 - dr0));
        r0 = r1;
        r1 = r2;
        dr0 = dr1;
    }
    return r1;
}

std::array<float, 2> PolynomialRadialDistortion::Distort(
        const std::array<float, 2>& p) const {
    float distortion_factor = DistortionFactor(p[0] * p[0] + p[1] * p[1]);
    return std::array<float, 2>{distortion_factor * p[0],
                                distortion_factor * p[1]};
}


std::array<float,2> PolynomialRadialDistortion::DistortInverse(const std::array<float, 2> &p) const {
    const float radius = std::sqrt(p[0] * p[0] + p[1] * p[1]);
    float inverseRadius=DistortRadiusInverse(radius);
    return std::array<float, 2>{(inverseRadius/radius) * p[0], (inverseRadius/radius) * p[1]};
}

std::vector<float> PolynomialRadialDistortion::getCoefficients()const {
    return coefficients_;
}

std::string PolynomialRadialDistortion::toString()const {
    std::stringstream ss;
    ss<<"PolynomialRadialDistortion:: k1..k"<<((int)coefficients_.size())<<"(";
    for(const float coefficient : coefficients_){
        ss<<coefficient<<",";
    }
    ss<<")";
    return ss.str();
}

//------------------------------------------------------ Inverse stuff------------------------------------------------------

std::string PolynomialRadialInverse::toStringX()const {
    std::stringstream ss;
    ss<<"PolynomialRadialInverse MaxRadSq "<<maxRadSq<<" k1..kn(";
    for(const float coefficient : getCoefficients()){
        ss<<coefficient<<",";
    }
    ss<<")";
    return ss.str();
}

std::vector<float> PolynomialRadialInverse::getApproximateInverseCoefficients(const PolynomialRadialDistortion& distortion,float maxRadius,int numCoefficients){
    constexpr unsigned int CALCULATION_SIZE=100; //100 by default
    std::vector<std::vector<double>> matA(CALCULATION_SIZE,std::vector<double>(numCoefficients));
    std::vector<double> vecY(CALCULATION_SIZE);
    for(int i = 0; i < CALCULATION_SIZE; ++i) {
        float r = maxRadius * (float)(i + 1) / ((float)CALCULATION_SIZE);
        auto rp = (double)distortion.DistortRadius(r);
        double v = rp;

        for(int j = 0; j < numCoefficients; ++j) {
            v *= rp * rp;
            matA[i][j] = v;
        }

        vecY[i] = (double)r - rp;
    }
    const auto vecK = LinearAlgebraHelper::solveLeastSquares(matA, vecY);
    return std::vector<float>(vecK.begin(),vecK.end());
}

float PolynomialRadialInverse::calculateDeviation(const PolynomialRadialDistortion &distortion,const PolynomialRadialInverse &inverseDistortion,const float radius) {
    const auto v1=distortion.DistortRadiusInverse(radius);
    const auto v2=inverseDistortion.DistortRadius(radius);
    const auto deviation=std::abs(v2-v1);
    return deviation;
}

float PolynomialRadialInverse::calculateMaxDeviation(const PolynomialRadialDistortion &distortion,
                                                     const PolynomialRadialInverse &inverse,
                                                     const float stepSize) {
    float maxDeviation=0.0F;
    for(float r2=0;r2<=inverse.maxRadSq;r2+=stepSize) {
        const float deviation = PolynomialRadialInverse::calculateDeviation(distortion,inverse,r2);
        if (deviation > maxDeviation) {
            maxDeviation=deviation;
        }
    }
    return maxDeviation;
}
