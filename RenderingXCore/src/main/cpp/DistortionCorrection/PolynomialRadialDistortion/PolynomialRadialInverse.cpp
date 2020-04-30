//
// Created by geier on 30/04/2020.
//

#include "PolynomialRadialInverse.h"
#include "LinearAlgebraHelper.hpp"
#include <cmath>
#include <sstream>

std::vector<float> PolynomialRadialInverse::getApproximateInverseCoefficients(const PolynomialRadialDistortion& distortion,float maxRadius,unsigned int numCoefficients){
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

std::string PolynomialRadialInverse::toStringX()const {
    std::stringstream ss;
    ss<<"PolynomialRadialInverse MaxRadSq "<<maxRadSq<<" k1..kn(";
    for(const float coefficient : getCoefficients()){
        ss<<coefficient<<",";
    }
    ss<<")";
    return ss.str();
}