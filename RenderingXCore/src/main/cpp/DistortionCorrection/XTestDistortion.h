//
// Created by Constantin on 1/14/2020.
//

#ifndef RENDERINGX_XTESTDISTORTION_H
#define RENDERINGX_XTESTDISTORTION_H


void test(){
/*for(float i=0;i<2;i+=0.1f){
        const auto p1=polynomialRadialDistortion.DistortInverse({i,0});
        const auto p2=polynomialRadialDistortion.DistortInverse2({i,0});
        LOGD("X %f , DistortInverse %f , DistortInverse2 %f", i,p1[0],p2[0] );
    }*/
    for(float i=0;i<2;i+=0.1f){
        LOGD("Radius:%f Inv1:%f Inv2:%f",i,mInverse.DistortRadius(i),polynomialRadialDistortion.DistortRadiusInverse(i));
    }
}

/*
    MAX_RAD_SQ=1.0f;
    bool done=false;
    while(MAX_RAD_SQ<2.0f && !done){
        const auto& inverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
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
    const int N=DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS-11+j;
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
