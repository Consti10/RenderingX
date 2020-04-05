//
// Created by geier on 30/01/2020.
//

#ifndef RENDERINGX_LINEARALGEBRAHELPER_H
#define RENDERINGX_LINEARALGEBRAHELPER_H

#include <vector>

//based on @java Distortion from gvr (google vr) btw. cardboard library
//Used by PolynomialRadialDistortion to create inverse distortion

namespace LinearAlgebraHelper{
    //see @java Distortion.solveLinear()
    static std::vector<double> solveLinear(std::vector<std::vector<double>>& a,std::vector<double>& y){
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
    //see @java Distortion.solveLeastSquares()
    static std::vector<double> solveLeastSquares(const std::vector<std::vector<double>>& matA,const std::vector<double>& vecY){
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
};
#endif //RENDERINGX_LINEARALGEBRAHELPER_H
