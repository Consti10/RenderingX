//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_VDDC_H
#define FPV_VR_VDDC_H

#include <array>
#include <string>
#include <sstream>
#include <Helper/MDebug.hpp>
#include "Helper/GLHelper.hpp"

/// VDDC Vertex displacement distortion correction
/// The distortion happens in view space
/// This space ranges from -1 ... 1, and maxRadSq would be therefore 1 without any distortion.
/// But since the distortion moves coordinate points 'inwarts' relative to the view space r2,
/// we have to distort maxRadSq before

class VDDC {
public:
    static constexpr const int N_UNDISTORTION_COEFICIENTS=7;
    //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
    static const std::string undistortCoeficientsToString(const std::array<float,N_UNDISTORTION_COEFICIENTS>& coeficients){
        std::stringstream s;
        s<<std::fixed;
        s<<"float r2;\n";
        s<<"vec4 pos;\n";
        s<<"float ret;\n";
        s<<"const float _MaxRadSq="<<coeficients[0];s<<";\n";
        //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
        s<<"const vec4 _Undistortion=vec4("<<coeficients[1]<<","<<coeficients[2]<<","<<coeficients[3]<<","<<coeficients[4]<<");\n";
        s<<"const vec2 _Undistortion2=vec2("<<coeficients[5]<<","<<coeficients[6]<<");\n";

        LOGD("%s",s.str().c_str());
        return s.str();
    }
    static const std::string writeGLPosition(const bool vdddc,const std::string &positionAttribute="aPosition"){
        if(vdddc)return writeGLPositionWithVDDC(positionAttribute);
        return "gl_Position = (uPMatrix*uMVMatrix)* "+positionAttribute+";\n";
    }
    static const std::string writeGLPositionWithVDDC(const std::string& positionAttribute){
        std::stringstream s;
        //s<<"  pos=(uPMatrix*uMVMatrix) * aPosition;\n";
        s<<"  pos=uMVMatrix * "+positionAttribute+";\n";
        //s<<"r2=1.0;";
        s<<"  r2=dot(pos.xy,pos.xy)/(pos.z*pos.z);\n";
        s<<" r2=clamp(r2,0.0,_MaxRadSq);";
        s<<"  ret = 0.0;\n";
        s<<"  ret = r2 * (ret + _Undistortion2.y);\n";
        s<<"  ret = r2 * (ret + _Undistortion2.x);\n";
        s<<"  ret = r2 * (ret + _Undistortion.w);\n";
        s<<"  ret = r2 * (ret + _Undistortion.z);\n";
        s<<"  ret = r2 * (ret + _Undistortion.y);\n";
        s<<"  ret = r2 * (ret + _Undistortion.x);\n";
        //s<<"if(ret < -1.0){";
        //s<<" ret=-1.0;";
        //s<<"}";
        //s<<" ret=0.0;";
        //s<<" if( sqrt(r2) > 1.0 ){";    //|| r2<__MaxRadSq
        //s<<" r2=_MaxRadSq;";
        //s<<" pos.x=0.0;";
        //s<<" pos.y=0.0;";
        //s<<" pos.z=pos.z*pos.z;";
        //s<<" ret=0.0;\n";
        //s<<" };";
        s<<"  pos.xy*=1.0+ret;\n";
        s<<"  gl_Position=uPMatrix*pos;\n";
        //s<<"  gl_Position=pos;\n";
        return s.str();
    }
};


#endif //FPV_VR_VDDC_H
