/*
 * Copyright 2019 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "polynomial_radial_distortion.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <android/log.h>


namespace cardboard {

PolynomialRadialDistortion::PolynomialRadialDistortion(
    const std::vector<float>& coefficients)
    : coefficients_(coefficients) {
  //LOGD("N coefficients:%d k1 %f k2 %f",coefficients_.size(),coefficients_[0],coefficients_[1]);
}

float PolynomialRadialDistortion::DistortionFactor(float r_squared) const {
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

std::array<float, 2> PolynomialRadialDistortion::Distort(
    const std::array<float, 2>& p) const {
  float distortion_factor = DistortionFactor(p[0] * p[0] + p[1] * p[1]);
  return std::array<float, 2>{distortion_factor * p[0],
                              distortion_factor * p[1]};
}

std::array<float, 2> PolynomialRadialDistortion::DistortInverse(
    const std::array<float, 2>& p) const {
  const float radius = std::sqrt(p[0] * p[0] + p[1] * p[1]);
  if (std::fabs(radius - 0.0f) < std::numeric_limits<float>::epsilon()) {
    return std::array<float, 2>();
  }

  // Based on the shape of typical distortion curves, |radius| / 2 and
  // |radius| / 3 are good initial guesses for the Secant method that will
  // remain within the intended range of the polynomial.
  float r0 = radius / 2.0f;
  float r1 = radius / 3.0f;
  float r2;
  float dr0 = radius - DistortRadius(r0);
  float dr1;
  while (std::fabs(r1 - r0) > 0.0001f /** 0.1mm */) {
    dr1 = radius - DistortRadius(r1);
    r2 = r1 - dr1 * ((r1 - r0) / (dr1 - dr0));
    r0 = r1;
    r1 = r2;
    dr0 = dr1;
  }

  return std::array<float, 2>{(r1 / radius) * p[0], (r1 / radius) * p[1]};
}
PolynomialRadialDistortion PolynomialRadialDistortion::getApproximateInverseDistortion(float maxRadius,int numCoefficients) {
  std::vector<std::vector<double>> matA(100,std::vector<double>(numCoefficients));
  std::vector<double> vecY(100);

  for(int i = 0; i < 100; ++i) {
    float r = maxRadius * (float)(i + 1) / 100.0F;
    //double rp = (double)this.distort(r);
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
  return PolynomialRadialDistortion(coefficients);
}

std::vector<double> PolynomialRadialDistortion::solveLeastSquares(const std::vector<std::vector<double>>& matA,const std::vector<double>& vecY) {
  int numSamples = matA.size();
  const unsigned int numCoefficients = matA[0].size();
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

std::vector<double> PolynomialRadialDistortion::solveLinear(std::vector<std::vector<double>>& a,std::vector<double>& y) {
  const unsigned int i = (a[0]).size();
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

std::vector<float> PolynomialRadialDistortion::getCoeficients()const {
  return coefficients_;
}

std::string PolynomialRadialDistortion::toString()const {
  std::stringstream ss;
  ss<<"PolynomialRadialDistortion:: k1..kn(";
  for(float coefficient : coefficients_){
    ss<<coefficient<<",";
  }
  ss<<")";
  return ss.str();
}

void PolynomialRadialDistortion::calculateDeviation(float radius,const PolynomialRadialDistortion &distortion,const PolynomialRadialDistortion &inverseDistortion) {
  //TODO
}

}  // namespace cardboard
