//
// Created by Consti10 on 11/12/2019.
//

#include "MPolynomialRadialDistortion.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <android/log.h>


MPolynomialRadialDistortion::MPolynomialRadialDistortion(
        const std::vector<float>& coefficients)
        : coefficients_(coefficients) {
}

float MPolynomialRadialDistortion::DistortionFactor(float r_squared) const {
    float r_factor = 1.0f;
    float distortion_factor = 1.0f;

    for (float ki : coefficients_) {
        r_factor *= r_squared;
        distortion_factor += ki * r_factor;
    }
    return distortion_factor;
}

float MPolynomialRadialDistortion::DistortRadius(float r) const {
    return r * DistortionFactor(r * r);
}

float MPolynomialRadialDistortion::DistortRadiusInverse(const float radius)const {
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

std::array<float, 2> MPolynomialRadialDistortion::Distort(
        const std::array<float, 2>& p) const {
    float distortion_factor = DistortionFactor(p[0] * p[0] + p[1] * p[1]);
    return std::array<float, 2>{distortion_factor * p[0],
                                distortion_factor * p[1]};
}


std::array<float,2> MPolynomialRadialDistortion::DistortInverse(const std::array<float, 2> &p) const {
    const float radius = std::sqrt(p[0] * p[0] + p[1] * p[1]);
    float inverseRadius=DistortRadiusInverse(radius);
    return std::array<float, 2>{(inverseRadius/radius) * p[0], (inverseRadius/radius) * p[1]};
}

MPolynomialRadialDistortion MPolynomialRadialDistortion::getApproximateInverseDistortion(float maxRadius,int numCoefficients)const {
    constexpr unsigned int CALCULATION_SIZE=100; //100 by default
    std::vector<std::vector<double>> matA(CALCULATION_SIZE,std::vector<double>(numCoefficients));
    std::vector<double> vecY(CALCULATION_SIZE);
    for(int i = 0; i < CALCULATION_SIZE; ++i) {
        float r = maxRadius * (float)(i + 1) / ((float)CALCULATION_SIZE);
        auto rp = (double)DistortRadius(r);
        double v = rp;

        for(int j = 0; j < numCoefficients; ++j) {
            v *= rp * rp;
            matA[i][j] = v;
        }

        vecY[i] = (double)r - rp;
    }
    const auto vecK = solveLeastSquares(matA, vecY);
    std::vector<float> coefficients(vecK.size());
    for(int i = 0; i < vecK.size(); ++i) {
        coefficients[i] = (float)vecK[i];
    }
    return MPolynomialRadialDistortion(coefficients);
}

std::vector<double> MPolynomialRadialDistortion::solveLeastSquares(const std::vector<std::vector<double>>& matA,const std::vector<double>& vecY) {
    const auto numSamples = matA.size();
    const auto numCoefficients = matA[0].size();
    std::vector<std::vector<double>> matATA(numCoefficients,std::vector<double>(numCoefficients));
    int j;
    double sum;
    int i;
    for(int k = 0; k < numCoefficients; ++k) {
        for(j = 0; j < numCoefficients; ++j) {
            sum = 0.0;

            for(i = 0; i < numSamples; ++i) {
                sum += matA[i][j] * matA[i][k];
            }

            matATA[j][k] = sum;
        }
    }
    std::vector<double> vecATY(numCoefficients);
    for(j = 0; j < numCoefficients; ++j) {
        sum = 0.0;
        for(i = 0; i < numSamples; ++i) {
            sum += matA[i][j] * vecY[i];
        }
        vecATY[j] = sum;
    }
    return solveLinear(matATA, vecATY);
}

std::vector<double> MPolynomialRadialDistortion::solveLinear(std::vector<std::vector<double>>& a,std::vector<double>& y) {
    const auto i = (unsigned int)(a[0]).size();
    for (int b = 0; b < i - 1; ) {
        for (int k = b + 1; k < i; ) {
            double d = a[k][b] / a[b][b];
            for (int m = b + 1; m < i; ) {
                a[k][m] = a[k][m] - d * a[b][m];
                m++;
            }  y[k] = y[k] - d * y[b];
            k++;
        }  b++;
    }
    std::vector<double> x(i);
    for (int j = i - 1; j >= 0; ) {
        double d = y[j];
        for (int k = j + 1; k < i; ) {
            d -= a[j][k] * x[k];
            k++;
        }  x[j] = d / a[j][j];
        j--;
    }
    return x;
}

std::vector<float> MPolynomialRadialDistortion::getCoeficients()const {
    return coefficients_;
}

std::string MPolynomialRadialDistortion::toString()const {
    std::stringstream ss;
    ss<<"PolynomialRadialDistortion:: k1..kn(";
    for(const float coefficient : coefficients_){
        ss<<coefficient<<",";
    }
    ss<<")";
    return ss.str();
}

float MPolynomialRadialDistortion::calculateDeviation(float radius,const MPolynomialRadialDistortion &distortion,const MPolynomialRadialDistortion &inverseDistortion) {
    const auto v1=distortion.DistortRadiusInverse(radius);
    const auto v2=inverseDistortion.DistortRadius(radius);
    const auto deviation=std::abs(v2-v1);
    return deviation;
}

float MPolynomialRadialDistortion::calculateMaxDeviation(const MPolynomialRadialDistortion &distortion,const MPolynomialRadialDistortion &inverse,
        const float maxRadSq, const float stepSize) {
    float maxDeviation=0.0F;
    for(float r2=0;r2<=maxRadSq;r2+=stepSize) {
        const float deviation = MPolynomialRadialDistortion::calculateDeviation(r2,distortion,inverse);
        if (deviation > maxDeviation) {
            maxDeviation=deviation;
        }
    }
    return maxDeviation;
}
