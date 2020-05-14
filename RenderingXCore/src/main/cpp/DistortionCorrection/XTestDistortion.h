//
// Created by Constantin on 1/14/2020.
//

#ifndef RENDERINGX_XTESTDISTORTION_H
#define RENDERINGX_XTESTDISTORTION_H

#include "PolynomialRadialDistortion/PolynomialRadialDistortion.h"
#include <vector>
#include <array>
#include "AndroidLogger.hpp"


static void EXPECT_NEAR(float a,float b,float tolerance){
    LOGD()<<("Deviation:"+std::to_string(std::abs(a-b)));
    if(std::abs(a-b)>tolerance){
        LOGD()<<"Test Error";
    }else{
        LOGD()<<"Test OK";
    }
}


void test2(){
    const float kDefaultFloatTolerance = 1.0e-3f;
    std::vector<std::pair<float, std::vector<float>>> device_range_and_params = {
            // Cardboard v1:
            {1.57f, {0.441f, 0.156f}},
            // Cardboard v2:
            {1.7f, {0.34f, 0.55f}}};

    for (const auto& device : device_range_and_params) {
        PolynomialRadialDistortion distortion(device.second);
        for (float radius = 0.0f; radius < device.first; radius += 0.01f) {
            // Choose a point whose distance from zero is |radius|.  Rotate by the
            // radius so that we're testing a range of points that aren't on a line.
            std::array<float, 2> point = {std::cosf(radius) * radius,
                                          std::sinf(radius) * radius};
            std::array<float, 2> inverse_point = distortion.DistortInverse(point);
            std::array<float, 2> check = distortion.Distort(inverse_point);

            EXPECT_NEAR(point[0], check[0], kDefaultFloatTolerance);
            EXPECT_NEAR(point[1], check[1], kDefaultFloatTolerance);
        }
    }
}


void test3(){
    const float kDefaultFloatTolerance = 1.0e-3f;
    std::vector<std::pair<float, std::vector<float>>> device_range_and_params = {
            // Cardboard v1:
            {1.57f, {0.441f, 0.156f}}
    };

    for (const auto& device : device_range_and_params) {
        const PolynomialRadialDistortion distortion(device.second);
        const float radius=device.first;

        const PolynomialRadialInverse inverse(distortion,radius*radius,8);

        for (float radius = 0.0f; radius < device.first; radius += 0.01f) {
            // Choose a point whose distance from zero is |radius|.  Rotate by the
            // radius so that we're testing a range of points that aren't on a line.
            const std::array<float, 2> point = {std::cosf(radius) * radius,
                                          std::sinf(radius) * radius};
            //Compare the parent.distortInverse with the inverse.distort()
            const std::array<float, 2> distort_inverse_point = distortion.DistortInverse(point);
            const std::array<float, 2> inverse_distort_point = inverse.Distort(point);

            EXPECT_NEAR(distort_inverse_point[0], inverse_distort_point[0], kDefaultFloatTolerance);
            EXPECT_NEAR(distort_inverse_point[1], inverse_distort_point[1], kDefaultFloatTolerance);
        }
    }
}




/*for(float i=0;i<2;i+=0.1f){
        const auto p1=polynomialRadialDistortion.DistortInverse({i,0});
        const auto p2=polynomialRadialDistortion.DistortInverse2({i,0});
        LOGD("X %f , DistortInverse %f , DistortInverse2 %f", i,p1[0],p2[0] );
    }*/


/*
    MAX_RAD_SQ=1.0f;
    bool done=false;
    while(MAX_RAD_SQ<2.0f && !done){
        const auto& inverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,VDDCManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
        const float maxDeviation=calculateMaxDeviation(polynomialRadialDistortion,inverse,MAX_RAD_SQ);
        LOGD("Max Rad Sq%f has max. deviation of %f",MAX_RAD_SQ,maxDeviation);
        if(maxDeviation>0.001f){
            done = true;
            MAX_RAD_SQ-=0.01f;
        }
        MAX_RAD_SQ+=0.01f;
    }*/


//In the range of [1 ... 2] calculate the inverse distortion and the max
//deviation value for
/*for(int j=0;j<=11;j++){
    const int N=VDDCManager::N_RADIAL_UNDISTORTION_COEFICIENTS-11+j;
    MAX_RAD_SQ=1.0f;
    for(float i=1.0f;i<=2.0f;i+=0.01f){
        const auto& inverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,N);
        const float maxDeviation=calculateMaxDeviation(polynomialRadialDistortion,inverse,MAX_RAD_SQ);
        if(maxDeviation<=0.002f){
            MAX_RAD_SQ=i;
        }
    }
    LOGD("K %d %f",N,MAX_RAD_SQ);
}*/
#endif //RENDERINGX_XTESTDISTORTION_H
