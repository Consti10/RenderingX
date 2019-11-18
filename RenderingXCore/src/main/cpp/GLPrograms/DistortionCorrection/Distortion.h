//
// Created by Consti10 on 17/10/2019.
//

#ifndef RENDERINGX_DISTORTION_H
#define RENDERINGX_DISTORTION_H

#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <fstream>
#include <filesystem>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const float calculateRadSquared(const glm::vec2 p);


//This wraps an 2d array that maps to each point in viewport space (e.g. 0...1)
//a distorted point in view space
//NOTE Messy, most likely deprecated with new open source cardboard

class Distortion{
private:
    struct DistortedPoint{
        glm::vec2 originalPoint;
        glm::vec2 distortedP;
    };
public:
    const int RESOLUTION;
    std::vector<std::vector<DistortedPoint>> distortedPoints;
public:
    Distortion(const int RESOLUTION);;
    Distortion(const int RESOLUTION,const gvr_context* gvrContext,const gvr_eye eye=GVR_LEFT_EYE);
    void print()const;
    glm::vec2 distortPoint(const glm::vec2 in)const;

    glm::vec2 findClosestMatchInverse(const glm::vec2 in)const;

    Distortion calculateInverse(const int NEW_RES)const;

    template <size_t rowColSize>
    void extractData(float (&arr)[rowColSize][rowColSize][2])const{
        assert(rowColSize==(RESOLUTION+1));
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const auto value=distortedPoints.at(i).at(j);
                const auto p=value.originalPoint*2.0f-glm::vec2(1.0f,1.0f);
                const auto distortedP=value.distortedP*2.0f-glm::vec2(1.0f,1.0f);
                const auto dir=distortedP-p;
                arr[i][j][0]=dir.x;//p.distortedP.x;
                arr[i][j][1]=dir.y;//p.distortedP.y;
            }
        }
    }

    void radialDistortionOnly();

    void saveAsText(const std::string& directory,const std::string& filename)const;

     void saveAsBinary(const std::string& directory,const std::string& filename)const;

    static Distortion createFromBinaryFile(const std::string& filename);
};

static float calculateBrownConrady(float r2,float k1,float k2){
    float ret=0.0f;
    ret=r2*(ret+k2);
    ret=r2*(ret+k1);
    return 1.0f+ret;
}
static const float calculateRadSquared(const glm::vec2 p){
    return sqrt(p.x*p.x+p.y*p.y);
}

static glm::vec2 distortPointBrownConrady(const glm::vec2 p,const float k1,const float k2){
    auto point=p-glm::vec2(0.5f,0.5f);
    const float r2=calculateRadSquared(point);
    const float dist=calculateBrownConrady(r2,k1,k2);
    point*=dist;
    return (p+glm::vec2(0.5f,0.5f));
}


/*static const void extractRadialTangentialDistortion(const int RESOLUTION,std::vector<std::vector<gvr_vec2f>>& distortedPoints){
    //const auto distortedPoints=calculateDistortedPoints(gvrContext,RESOLUTION);
    const auto middle=distortedPoints.at(0.5f*RESOLUTION).at(0.5f*RESOLUTION);
    LOGD("VDDC","(0.5,0.5) links to -> (%f,%f)",middle.x,middle.y);

    const glm::vec2 translation={middle.x-0.5f,middle.y-0.5f};

    for(int i=0;i<=RESOLUTION;i++){
        for(int j=0;j<=RESOLUTION;j++){
            //use +/- 0.5f for coordinate system with (0,0) in the middle
            const glm::vec2 p1=glm::vec2{(float)i/(float)RESOLUTION-0.5f,(float)j/(float)RESOLUTION-0.5f}+translation;
            const glm::vec2 p2=glm::vec2{distortedPoints.at(i).at(j).x-0.5f,distortedPoints.at(i).at(j).y-0.5f}+translation;

            const float p1_r2=calculateRadSquared(p1);
            const float p2_r2=calculateRadSquared(p2);
            const float radialDistortionFactor=1.0f+p2_r2-p1_r2;
            const  glm::vec2 radialDistortedPoint=p1*radialDistortionFactor;
            distortedPoints.at(i).at(j).x=radialDistortedPoint.x+0.5f-translation.x;
            distortedPoints.at(i).at(j).y=radialDistortedPoint.y+0.5f-translation.y;

            //const glm::vec2 tangentialDistortion=

            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","(%f) -> (%f) ->(%f)",p1_r2, radialDistortionFactor,calculateBrownConrady(p1_r2,0.34,0.55));

            const float r2=sqrt(p1.x*p1.x+p2.y*p2.y);
            const float radialDistortion=calculateBrownConrady(r2,0.34,0.55);
            const glm::vec2 radialDistortedPoint=p1*radialDistortion;

            const glm::vec2 tangentialDistortion=p2-radialDistortedPoint;

            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","(%f) -> (%f,%f)",p1.x,tangentialDistortion.x,tangentialDistortion.y);

            distortedPoints.at(i).at(j)={radialDistortedPoint.x+0.5f,radialDistortedPoint.y+0.5f};
            float a;
            if(p1.x==0.0f) {
                a = p2.x;
            }else if(m==1.0f){
                a=0.0f;
            }else{
                // a * m = p2.y
                const float m=p1.y/p1.x; //Steigung
                a=p2.y / m;
            }
            const  glm::vec2 radialDistortedPoint={a,p2.y};
            distortedPoints.at(i).at(j).x=radialDistortedPoint.x+0.5f;
            distortedPoints.at(i).at(j).y=radialDistortedPoint.y+0.5f;
 }
    }
}*/



#endif //RENDERINGX_DISTORTION_H
