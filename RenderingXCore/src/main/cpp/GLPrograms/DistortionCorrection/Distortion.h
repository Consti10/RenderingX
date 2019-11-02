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

#include "android/log.h"
static const float calculateRadSquared(const glm::vec2 p);

class Distortion{
private:
    struct DistortedPoint{
        glm::vec2 originalPoint;
        glm::vec2 distortedP;
    };
public:
    const int RESOLUTION;
    std::vector<std::vector<DistortedPoint>> distortedPoints;
    Distortion(const int RESOLUTION):RESOLUTION(RESOLUTION),distortedPoints((unsigned int)(RESOLUTION+1),std::vector<DistortedPoint>((unsigned int)(RESOLUTION+1))){};
    Distortion(const int RESOLUTION,const gvr_context* gvrContext,const gvr_eye eye=GVR_LEFT_EYE):
            RESOLUTION(RESOLUTION),distortedPoints((unsigned int)(RESOLUTION+1),std::vector<DistortedPoint>((unsigned int)(RESOLUTION+1))){
        //Creates a 2d array of size (RESOULTION+1)
        //array at position [x][y] contains distorted values for (x,y)==(i/RESOLUTION,j/RESOLUTION) and x bty in range [0..1] inclusive
        //RESOLUTION should be multiple of 2
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++){
                const float x=(float)i/(float)RESOLUTION;
                const float y=(float)j/(float)RESOLUTION;
                gvr_vec2f in{x,y};
                gvr_vec2f out[3];
                gvr_compute_distorted_point(gvrContext,eye,in,out);
                //We do not correct for chromatic aberation, so take the average
                float avgX=(out[0].x+out[1].x+out[2].x)/3.0f;
                float avgY=(out[0].y+out[1].y+out[2].y)/3.0f;
                distortedPoints.at(i).at(j)={glm::vec2(x,y),glm::vec2(avgX,avgY)};//out[0].x,out[0].y
            }
        }
    }
    const void print()const{
        const auto seperator="--------------------------------------------------------------------";
        __android_log_print(ANDROID_LOG_DEBUG,"VDDC","%s",seperator);
        for(int i=0;i<=RESOLUTION;i++){
            const auto rowArray=distortedPoints.at(i);
            std::stringstream ss;
            ss<<"|";
            for(int j=0;j<=RESOLUTION;j++){
                ss<<"("<<rowArray.at(j).originalPoint.x<<","<<rowArray.at(j).originalPoint.y<<"),";
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
        if(distortedPoint.originalPoint!=glm::vec2(in.x,in.y)){
            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","Wrong resolution. Want (%f,%f) Found (%f,%f)",in.x,in.y,
                                distortedPoint.originalPoint.x,distortedPoint.originalPoint.y);
        }
        return distortedPoint.distortedP;
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
                    bestMatchUndistortionPoint=point.originalPoint;
                }
            }
        }
        /*if(lastBestDiff>0.001f){
            __android_log_print(ANDROID_LOG_DEBUG,"VDDC","Looking for (%f,%f) -> Found (%f,%f) -> Diff is %f",in.x,in.y,bestMatchUndistortionPoint.x,bestMatchUndistortionPoint.y,lastBestDiff);
        }*/
        return bestMatchUndistortionPoint;
    }

    const Distortion calculateInverse(const int NEW_RES)const{
        Distortion distortion(NEW_RES);
        for(int i=0;i<=NEW_RES;i++){
            for(int j=0;j<=NEW_RES;j++){
                const float x=(float)i/(float)NEW_RES;
                const float y=(float)j/(float)NEW_RES;
                const auto p=findClosestMatchInverse({x,y});
                distortion.distortedPoints.at(i).at(j)={{x,y},p};
            }
        }
        return distortion;
    }
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
    void radialDistortionOnly(){
        //find the point where the inverse is closest to (0.5/0.5)
        const auto middle=findClosestMatchInverse({0.5f,0.5f});
        LOGD("Middle is at (%f,%f)",middle.x,middle.y);
        for(int i=0;i<=RESOLUTION;i++){
            for(int j=0;j<=RESOLUTION;j++) {
                //use +/- 0.5f for coordinate system with (0,0) in the middle
                const glm::vec2 p1 = distortedPoints.at(i).at(j).originalPoint-glm::vec2(0.5f,0.5f);
                const glm::vec2 p2 = distortedPoints.at(i).at(j).distortedP-glm::vec2(0.5f,0.5f);
                const float p1_r2 = calculateRadSquared(p1);
                const float p2_r2 = calculateRadSquared(p2);
                const float radialDistortionFactor = 1.0f + p2_r2 - p1_r2;
                const glm::vec2 radialDistortedPoint = p1 * radialDistortionFactor;
                distortedPoints.at(i).at(j).distortedP=radialDistortedPoint+glm::vec2(0.5f,0.5f);
            }
        }
    }
    bool fileExists(const std::string filename)const{
        std::ifstream infile(filename.c_str());
        return infile.good();
    }
    const void saveAsText(const std::string directory,const std::string filename){
       std::stringstream ss;
        for(int i=0;i<=RESOLUTION;i++){
            const auto& rowArray=distortedPoints.at(i);
            for(int j=0;j<=RESOLUTION;j++){
                const auto& point=rowArray.at(j);
                ss<<"("<<point.originalPoint.x<<","<<point.originalPoint.y<<"),";
            }
            ss<<"\n";
        }
        //write to file
        //bool exist = std::filesystem::exists("Hi");
        //std::filesystem::create_directory(directory.c_str());
        //__android_log_print(ANDROID_LOG_DEBUG,"Distortion","File exists%d",fileExists(directory+filename));
        std::ofstream distortionFile;
        distortionFile.open((directory+filename).c_str());
        distortionFile.write(ss.str().c_str(),ss.str().length());
        distortionFile.flush();
        distortionFile.close();
    }

     const void saveAsBinary(const std::string directory,const std::string filename){
         std::ofstream distortionFile;
         distortionFile.open((directory+filename).c_str(),std::ofstream::binary);
        //In the first line, we write the size as int32_t value
        //Then we just write the bytes of the distortion points array
        int32_t size=RESOLUTION+1;
        distortionFile.write((char*)&size,sizeof(int32_t));
        for(int i=0;i<=RESOLUTION;i++){
            const auto& rowArray=distortedPoints.at(i);
            for(int j=0;j<=RESOLUTION;j++){
                const auto& point=rowArray.at(j);
                float x=point.distortedP.x;
                float y=point.distortedP.y;
                distortionFile.write((char*)&x,sizeof(float));
                distortionFile.write((char*)&y,sizeof(float));
            }
        }
         distortionFile.flush();
         distortionFile.close();
    }
    
    static Distortion createFromBinaryFile(const std::string& filename){
        std::ifstream file (filename.c_str(),std::ifstream::binary);
        file.seekg(0, file.end);
        const int fileSize = (int)file.tellg();
        file.seekg(0, file.beg);
        uint8_t data[fileSize];
        file.read ((char*)data,fileSize);
        file.close();
        //first 4 bytes are the size of one row
        int32_t size;
        memcpy(&size,data,sizeof(int32_t));
        Distortion distortion(size-1);
        int offset=4;
        for(int i=0;i<=distortion.RESOLUTION;i++){
            for(int j=0;j<=distortion.RESOLUTION;j++){
                const float x=(float)i/(float)distortion.RESOLUTION;
                const float y=(float)j/(float)distortion.RESOLUTION;
                float x2,y2;
                memcpy(&x2,&data[offset], sizeof(float));
                offset+=4;
                memcpy(&y2,&data[offset], sizeof(float));
                offset+=4;
                distortion.distortedPoints.at(i).at(j).originalPoint={x,y};
                distortion.distortedPoints.at(i).at(j).distortedP={x2,y2};
            }
        }
        return distortion;
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
