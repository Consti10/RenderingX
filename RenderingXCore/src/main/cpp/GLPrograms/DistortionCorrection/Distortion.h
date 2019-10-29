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

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "android/log.h"
static const float calculateRadSquared(const glm::vec2 p);

class Distortion{
private:
    struct DistortedPoint{
        glm::vec2 p;
        glm::vec2 distortedP;
    };
    struct Link{
        float searchValue;
        DistortedPoint* p;
    };
public:
    const int RESOLUTION;
    std::vector<std::vector<DistortedPoint>> distortedPoints;
    std::vector<Link> list;
    Distortion(const int RESOLUTION,const gvr_context* gvrContext):
            RESOLUTION(RESOLUTION),distortedPoints((RESOLUTION+1),std::vector<DistortedPoint>((RESOLUTION+1))){
        if(gvrContext== nullptr)return;
        //Creates a 2d array of size (RESOULTION+1)
        //array at position [x][y] contains distorted values for (x,y)==(i/RESOLUTION,j/RESOLUTION) and x bty in range [0..1] inclusive
        //RESOLUTION should be multiple of 2
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const float x=(float)i/(float)RESOLUTION;
                const float y=(float)j/(float)RESOLUTION;
                gvr_vec2f in{x,y};
                gvr_vec2f out[3];
                gvr_compute_distorted_point(gvrContext,GVR_LEFT_EYE,in,out);
                distortedPoints.at(i).at(j)={glm::vec2(x,y),glm::vec2(out[0].x,out[0].y)};
            }
        }
        //
        /*for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const int idx=i*(RESOLUTION+1)+j;
                auto& p=distortedPoints.at(i).at(j);
                const float searchValue=p.p.x*(RESOLUTION+1)+p.p.y;
                list.at(idx)={searchValue,&p};
                //__android_log_print(ANDROID_LOG_DEBUG,"VDDC","DPSV %f",searchValue);
            }
        }
        std::sort(list.begin(),list.end(),[](Link a,Link b)->bool{return a.searchValue > b.searchValue; });*/
        /*for(Link& link:list){
            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","Link %f -> (%f,%f)",link.searchValue,link.p->p.x,link.p->p.y);
        }*/
    }

    const void print()const{
        const auto seperator="--------------------------------------------------------------------";
        __android_log_print(ANDROID_LOG_DEBUG,"VDDC","%s",seperator);
        for(int i=0;i<=RESOLUTION;i++){
            const auto rowArray=distortedPoints.at(i);
            std::stringstream ss;
            ss<<"|";
            for(int j=0;j<=RESOLUTION;j++){
                ss<<"("<<rowArray.at(j).p.x<<","<<rowArray.at(j).p.y<<"),";
            }
            ss<<"|";
            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","%s",ss.str().c_str());
        }
        __android_log_print(ANDROID_LOG_DEBUG,"VDDC","%s",seperator);
    }
    const glm::vec2 distortPoint(const glm::vec2 in)const{
        const float indexX_=in.x*RESOLUTION;
        const float indexY_=in.y*RESOLUTION;
        const int indexX=(int)std::round(indexX_);
        const int indexY=(int)std::round(indexY_);
        const DistortedPoint& distortedPoint=distortedPoints.at(indexX).at(indexY);
        if(distortedPoint.p!=glm::vec2(in.x,in.y)){
            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","Wrong resolution. Want (%f,%f) Found (%f,%f)",in.x,in.y,
                                distortedPoint.p.x,distortedPoint.p.y);
        }
        return distortedPoint.distortedP;
        /*const float searchValue=(in.x*(RESOLUTION+1))+in.y;
        auto const it = std::lower_bound(list.begin(), list.end(), searchValue,[](Link a,float b)->bool{return a.searchValue > b; });
        __android_log_print(ANDROID_LOG_DEBUG,"VDDC","Looking for (%f,%f) -> Got (%f,%f)",in.x,in.y,it->p->p.x,it->p->p.y);
        return  it->p->distortedP;*/
    }

    const glm::vec2 findClosestMatchInverse(const glm::vec2 in)const{
        double lastBestDiff=std::numeric_limits<double>::max();
        glm::vec2 bestMatchUndistortionPoint=glm::vec2(-100.0f,-100.0f);
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const auto point=distortedPoints.at(i).at(j);
                const double diff1=abs(point.distortedP.x-in.x);
                const double diff2=abs(point.distortedP.y-in.y);
                const double diff=sqrt(diff1*diff1+diff2*diff2);//(diff1+diff2)/2.0f;
                if(diff<lastBestDiff){
                    lastBestDiff=diff;
                    bestMatchUndistortionPoint=point.p;
                }
            }
        }
        /*if(lastBestDiff>0.001f){
            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","Looking for (%f,%f) -> Found (%f,%f) -> Diff is %f",in.x,in.y,bestMatchUndistortionPoint.x,bestMatchUndistortionPoint.y,lastBestDiff);
        }*/
        return bestMatchUndistortionPoint;
    }

    const Distortion calculateInverse(const int NEW_RES)const{
        //create array with DistortedPoints, but one dimension to make searching
        /*std::vector<Link> list((RESOLUTION+1)*(RESOLUTION+1));
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const int idx=i*RESOLUTION+j;
                auto& p=distortedPoints.at(i).at(j);
                const float distortedPointSearchValue=p.distortedP.x*RESOLUTION+p.distortedP.y;
                list.at(idx)={distortedPointSearchValue,&p};
                //__android_log_print(ANDROID_LOG_DEBUG,"VDDC","DPSV %f",distortedPointSearchValue);
            }
        }
        std::sort(list.begin(),list.end(),[](Link a,Link b)->bool{return a.distortedPointSearchValue > b.distortedPointSearchValue; });*/
        Distortion distortion(NEW_RES,nullptr);
        for(int i=0;i<=NEW_RES;i++){
            for(int j=0;j<=NEW_RES;j++){
                const float x=(float)i/(float)NEW_RES;
                const float y=(float)j/(float)NEW_RES;
                const auto p=findClosestMatchInverse({x,y});
                distortion.distortedPoints.at(i).at(j)={{x,y},p};
                //const float searchValue=x*RESOLUTION+y;
                //auto const it = std::lower_bound(list.begin(), list.end(), searchValue,[](Link a,float b)->bool{return a.distortedPointSearchValue > b; });
                //distortion.distortedPoints.at(i).at(j)={{x,y},it.base()->distortedP};
            }
        }
        return distortion;
    }

    void radialDistortionOnly(){
        //find the point where the inverse is closest to (0.5/0.5)
        const auto middle=findClosestMatchInverse({0.5f,0.5f});
        LOGD("Middle is at (%f,%f)",middle.x,middle.y);
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++) {
                //use +/- 0.5f for coordinate system with (0,0) in the middle
                const glm::vec2 p1 = distortedPoints.at(i).at(j).p-glm::vec2(0.5f,0.5f);
                const glm::vec2 p2 = distortedPoints.at(i).at(j).distortedP-glm::vec2(0.5f,0.5f);
                const float p1_r2 = calculateRadSquared(p1);
                const float p2_r2 = calculateRadSquared(p2);
                const float radialDistortionFactor = 1.0f + p2_r2 - p1_r2;
                const glm::vec2 radialDistortedPoint = p1 * radialDistortionFactor;
                distortedPoints.at(i).at(j).distortedP=radialDistortedPoint+glm::vec2(0.5f,0.5f);
            }
        }
    }
    /*std::vector<std::vector<glm::vec2>> calculateVectorField(){
        std::vector<std::vector<glm::vec2>> ret((RESOLUTION+1)*(RESOLUTION+1));
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const auto p=distortedPoints.at(i).at(j);
                auto dir=p.distortedP-p.p;
                //dir=glm::ab
            }
        }
    }*/
    void lol(float (&arr) [32][32][2])const{
        //float ret[RESOLUTION][RESOLUTION][2];
        for(int i=0;i<RESOLUTION;i++){
            for(int j=0;j<RESOLUTION;j++){
                const auto value=distortedPoints.at(i).at(j);
                const auto p=value.p*2.0f-glm::vec2(1.0f,1.0f);
                const auto distortedP=value.distortedP*2.0f-glm::vec2(1.0f,1.0f);
                const auto dir=distortedP-p;
                arr[i][j][0]=dir.x;//p.distortedP.x;
                arr[i][j][1]=dir.y;//p.distortedP.y;
                //arr[i][j][0]=dir.x;
                //arr[i][j][1]=dir.y;
            }
        }
        //return (float*) ret;
    }

    const void save( const std::string filename){
        std::stringstream ss;
        for(int i=0;i<=RESOLUTION;i++){
            const auto rowArray=distortedPoints.at(i);
            for(int j=0;j<=RESOLUTION;j++){
                const auto point=rowArray.at(j);
                ss<<"("<<point.p.x<<","<<point.p.y<<"),";
            }
            ss<<"\n";
        }
        //write to file
        std::ofstream distortionFile;
        distortionFile.open (filename.c_str());
        distortionFile.write(ss.str().c_str(),ss.str().length());
        distortionFile.flush();
        distortionFile.close();
    }
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


static const void extractRadialTangentialDistortion(const int RESOLUTION,std::vector<std::vector<gvr_vec2f>>& distortedPoints){
    //const auto distortedPoints=calculateDistortedPoints(gvrContext,RESOLUTION);
    const auto middle=distortedPoints.at(0.5f*RESOLUTION).at(0.5f*RESOLUTION);
    __android_log_print(ANDROID_LOG_DEBUG,"VDDC","(0.5,0.5) links to -> (%f,%f)",middle.x,middle.y);
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

            /*const float r2=sqrt(p1.x*p1.x+p2.y*p2.y);
            const float radialDistortion=calculateBrownConrady(r2,0.34,0.55);
            const glm::vec2 radialDistortedPoint=p1*radialDistortion;

            const glm::vec2 tangentialDistortion=p2-radialDistortedPoint;

            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","(%f) -> (%f,%f)",p1.x,tangentialDistortion.x,tangentialDistortion.y);

            distortedPoints.at(i).at(j)={radialDistortedPoint.x+0.5f,radialDistortedPoint.y+0.5f};*/
            /*float a;
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
            distortedPoints.at(i).at(j).y=radialDistortedPoint.y+0.5f;*/
        }
    }
}



#endif //RENDERINGX_DISTORTION_H
