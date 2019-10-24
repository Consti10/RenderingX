//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_VDDC_H
#define FPV_VR_VDDC_H

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

/// VDDC Vertex displacement distortion correction
/// The distortion happens in view space
/// This space ranges from -1 ... 1, and maxRadSq would be therefore 1 without any distortion.
/// But since the distortion moves coordinate points 'inwarts' relative to the view space r2,
/// we have to distort maxRadSq before

static constexpr const int MY_VERSION=2;

class DistortionManager{
public:
    static constexpr const int N_UNDISTORTION_COEFICIENTS=7;
    std::array<float,N_UNDISTORTION_COEFICIENTS> VR_DC_UndistortionData;

    static constexpr const int RESOLUTION_XY=32;
    static constexpr int ARRAY_SIZE=RESOLUTION_XY*RESOLUTION_XY;
    float lol[RESOLUTION_XY][RESOLUTION_XY][2];

    GLuint mDistortionCorrectionTexture;
public:
    DistortionManager(gvr_context* gvrContext){
        Distortion mDistortion(400,gvrContext);
        Distortion inverse=mDistortion.calculateInverse(32);
        //mDistortion.radialDistortionOnly();
        //Distortion inverse=mDistortion.calculateInverse(RESOLUTION_XY);
        //inverse.radialDistortionOnly();
        inverse.lol(lol);
        //coefficients: [0.34, 0.55]
        VR_DC_UndistortionData.at(0)=10.0f;
        VR_DC_UndistortionData.at(1)=0.34f;
        VR_DC_UndistortionData.at(2)=0.55f;
        //VR_DC_UndistortionData.at(2)=1.4535f;
    }

    void beforeDraw(const GLuint lolHandle,const GLuint samplerDistCorrectionHandle)const{
        if(MY_VERSION==0){
            //Nothing
        }else if(MY_VERSION==1){
            glUniform2fv(lolHandle,(GLsizei)(ARRAY_SIZE),(GLfloat*)lol);
        }else{
            glActiveTexture(DistortionManager::MY_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D,mDistortionCorrectionTexture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glUniform1i(samplerDistCorrectionHandle,DistortionManager::MY_SAMPLER_UNIT);
        }
    }

    void afterDraw()const{
        glBindTexture(GL_TEXTURE_2D,0);
    }

    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE1;
    static constexpr auto MY_SAMPLER_UNIT=1;
    void generateTexture(){
        glGenTextures(1,&mDistortionCorrectionTexture);

        glActiveTexture(MY_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, mDistortionCorrectionTexture);

        const int SIZE=DistortionManager::RESOLUTION_XY;
        GLfloat data[SIZE][SIZE][4];
        for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                data[i][j][0]=lol[j][i][0];
                data[i][j][1]=lol[j][i][1];
                //data[i][j][0]=0.0f;
                //data[i][j][1]=0.2f;
            }
        }
        //GL_RGBA32F
        constexpr auto RGBA32F_ARB=0x8814;
        constexpr auto RGBA16F_ARB=0x881A;
        glTexImage2D(GL_TEXTURE_2D, 0,RGBA16F_ARB, SIZE,SIZE, 0, GL_RGBA, GL_FLOAT,data);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        //NDKHelper::uploadAssetImageToGPU(env,androidContext,name,false);

        glBindTexture(GL_TEXTURE_2D,0);

    }
};


class VDDC {
public:

    static const std::string writeGLPosition(const DistortionManager* distortionManager,const std::string &positionAttribute="aPosition"){
        if(distortionManager!= nullptr)return writeGLPositionWithVDDC(positionAttribute);
        //return"vec4 lul=uMVMatrix * "+positionAttribute+";\n"+"";
        return "gl_Position = (uPMatrix*uMVMatrix)* "+positionAttribute+";\n";
        //return "gl_Position = vec4("+positionAttribute+".xy*2.0, 0, 1);";
    }
    static const std::string writeGLPositionWithVDDC(const std::string& positionAttribute){
        std::stringstream s;
        if(MY_VERSION==0){
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
        }else if(MY_VERSION==1){
            s<<std::fixed;
            //s<<"  pos=vec4("+positionAttribute+".xy*2.0, 0, 1);";
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
            s<<"vec2 ndc=pos.xy/pos.w;";///(-pos.z);";
            s<<"vec2 uvFromNDC=(ndc+vec2(1.0,1.0))/2.0;";
            s<<"vec4 value=texture2D(sTextureDistCorrection, uvFromNDC );";
            s<<"pos.x+=value.x*pos.w;";
            s<<"pos.y+=value.y*pos.w;";
            s<<"gl_Position=pos;\n";
        }
        return s.str();
    }

    static const std::string writeLOL(const DistortionManager* distortionManager){
        std::stringstream s;
        //Write placeholders even if not needed
        s<<"uniform highp vec2 LOL["<<((distortionManager!= nullptr && MY_VERSION==1) ? DistortionManager::ARRAY_SIZE : 1)<<"];";
        s<<"uniform sampler2D sTextureDistCorrection;";
        if(distortionManager==nullptr)return s.str();
        if(MY_VERSION==0){
            const auto coeficients=distortionManager->VR_DC_UndistortionData;
            s<<std::fixed;
            s<<"const float _MaxRadSq="<<coeficients[0];s<<";\n";
            //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
            s<<"const vec4 _Undistortion=vec4("<<coeficients[1]<<","<<coeficients[2]<<","<<coeficients[3]<<","<<coeficients[4]<<");\n";
            s<<"const vec2 _Undistortion2=vec2("<<coeficients[5]<<","<<coeficients[6]<<");\n";
        }else if(MY_VERSION==1){
            s<<"int my_clamp(in int x,in int minVal,in int maxVal){";
            s<<"if(x<minVal){return minVal;}";
            s<<"if(x>maxVal){return maxVal;}";
            s<<"return x;}";
            s<<"int my_round(in float x){";
            s<<"float afterCome=x-float(int(x));";
            s<<"if(afterCome>=0.5){return int(x+0.5);}";
            s<<"return int(x);}";
        }else{
            //Nothing
        }
        return s.str();
    }

};




#endif //FPV_VR_VDDC_H
