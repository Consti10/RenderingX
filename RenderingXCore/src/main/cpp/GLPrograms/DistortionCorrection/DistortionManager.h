//
// Created by Consti10 on 31/10/2019.
//

#ifndef RENDERINGX_DISTORTIONMANAGER_H
#define RENDERINGX_DISTORTIONMANAGER_H

#include <array>
#include <string>
#include <sstream>
#include <Helper/MDebug.hpp>
#include <vector>
#include "Helper/GLHelper.hpp"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Helper/NDKHelper.h>
#include "android/log.h"
#include "DistortionCorrection/Distortion.h"

class DistortionManager {
public:
    static constexpr const int N_RADIAL_UNDISTORTION_COEFICIENTS=7;
    std::array<float,N_RADIAL_UNDISTORTION_COEFICIENTS> RadialUndistortionData;

    static constexpr const int RESOLUTION_XY=32;
    static constexpr int ARRAY_SIZE=RESOLUTION_XY*RESOLUTION_XY;
    float lol[RESOLUTION_XY][RESOLUTION_XY][2];

    GLuint mDistortionCorrectionTexture;
    struct UndistortionHandles{
        GLuint lolHandle;
        GLuint samplerDistCorrectionHandle;
    };
    const int MY_VERSION=2;
public:
    DistortionManager(gvr_context* gvrContext);
    DistortionManager(JNIEnv *env,jfloatArray undistData);
    UndistortionHandles getUndistortionUniformHandles(const GLuint program)const;
    void beforeDraw(const UndistortionHandles undistortionHandles)const;
    void afterDraw()const;

    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT=1;
    void generateTexture();

    static const std::string writeGLPosition(const DistortionManager* distortionManager,const std::string &positionAttribute="aPosition"){
        if(distortionManager!= nullptr)return writeGLPositionWithVDDC(*distortionManager,positionAttribute);
        //return"vec4 lul=uMVMatrix * "+positionAttribute+";\n"+"";
        return "gl_Position = (uPMatrix*uMVMatrix)* "+positionAttribute+";\n";
        //return "gl_Position = vec4("+positionAttribute+".xy*2.0, 0, 1);";
    }
    static const std::string writeGLPositionWithVDDC(const DistortionManager& distortionManager,const std::string& positionAttribute){
        std::stringstream s;
        if(distortionManager.MY_VERSION==0){
            s<<"vec4 pos=uMVMatrix*"+positionAttribute+";\n";
            s<<"float r2=dot(pos.xy,pos.xy)/(pos.z*pos.z);\n";
            s<<"r2=clamp(r2,0.0,_MaxRadSq);\n";
            s<<"float ret = 0.0;\n";
            s<<"ret = r2 * (ret + _Undistortion2.y);\n";
            s<<"ret = r2 * (ret + _Undistortion2.x);\n";
            s<<"ret = r2 * (ret + _Undistortion.w);\n";
            s<<"ret = r2 * (ret + _Undistortion.z);\n";
            s<<"ret = r2 * (ret + _Undistortion.y);\n";
            s<<"ret = r2 * (ret + _Undistortion.x);\n";
            s<<"pos.xy*=1.0+ret;\n";
            s<<"gl_Position=pos;\n";
        }else if(distortionManager.MY_VERSION==1){
            s<<std::fixed;
            //s<<"vec4 pos=vec4("+positionAttribute+".xy*2.0, 0, 1);";
            s<<"vec4  pos=(uPMatrix*uMVMatrix)*"+positionAttribute+";\n";
            s<<"vec2 ndc=pos.xy/pos.w;";///(-pos.z);";
            s<<"int idx1=my_round(ndc.x*"<<(DistortionManager::RESOLUTION_XY/2.0f)<<")+"<<DistortionManager::RESOLUTION_XY/2<<";";
            s<<"int idx2=my_round(ndc.y*"<<(DistortionManager::RESOLUTION_XY/2.0f)<<")+"<<DistortionManager::RESOLUTION_XY/2<<";";
            s<<"idx1=my_clamp(idx1,0,"<<DistortionManager::RESOLUTION_XY-1<<");";
            s<<"idx2=my_clamp(idx2,0,"<<DistortionManager::RESOLUTION_XY-1<<");";
            //We do not have support for 2-dimensional arrays- simple pointer arithmetic ( array[i][j]==array[i*n+j] )
            s<<"int idx=idx1*"<<DistortionManager::RESOLUTION_XY<<"+idx2;";
            s<<"pos.x+=LOL[idx].x;";
            s<<"pos.y+=LOL[idx].y;";
            s<<"gl_Position=pos;\n";
        }else{
            s<<"vec4  pos=(uPMatrix*uMVMatrix)*"+positionAttribute+";\n";
            //s<<"vec4 pos=vec4("+positionAttribute+".xy*2.0, 0, 1);";
            s<<"vec2 ndc=pos.xy/pos.w;";///(-pos.z);";
            s<<"vec2 uvFromNDC=(ndc+vec2(1.0,1.0))/2.0;";
            s<<"vec4 value=texture2D(sTextureDistCorrection,uvFromNDC );";
            s<<"pos.x+=value.x*pos.w;";
            s<<"pos.y+=value.y*pos.w;";
            s<<"gl_Position=pos;\n";
        }
        return s.str();
    }

    static const std::string writeLOL(const DistortionManager* distortionManager){
        std::stringstream s;
        if(distortionManager==nullptr)return "";
        if(distortionManager->MY_VERSION==0){
            const auto coeficients=distortionManager->RadialUndistortionData;
            s<<std::fixed;
            s<<"const float _MaxRadSq="<<coeficients[0];s<<";\n";
            //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
            s<<"const vec4 _Undistortion=vec4("<<coeficients[1]<<","<<coeficients[2]<<","<<coeficients[3]<<","<<coeficients[4]<<");\n";
            s<<"const vec2 _Undistortion2=vec2("<<coeficients[5]<<","<<coeficients[6]<<");\n";
        }else if(distortionManager->MY_VERSION==1){
            s<<"uniform highp vec2 LOL["<< DistortionManager::ARRAY_SIZE<<"];";
            s<<"int my_clamp(in int x,in int minVal,in int maxVal){";
            s<<"if(x<minVal){return minVal;}";
            s<<"if(x>maxVal){return maxVal;}";
            s<<"return x;}";
            s<<"int my_round(in float x){";
            s<<"float afterCome=x-float(int(x));";
            s<<"if(afterCome>=0.5){return int(x+0.5);}";
            s<<"return int(x);}";
        }else if(distortionManager->MY_VERSION==2){
            s<<"uniform sampler2D sTextureDistCorrection;";
        }
        return s.str();
    }
};


#endif //RENDERINGX_DISTORTIONMANAGER_H
