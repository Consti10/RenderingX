//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_COLOR_H
#define FPV_VR_GLPROGRAMS_HELPER_COLOR_H


#include <cstdint>
#include <glm/vec4.hpp>
#include <string>
#include <sstream>

//since OpenGL uses RGBA,we also use RGBA for TrueColor. Make sure to convert when using ColorPreference
using TrueColor=uint32_t;

class Color {
public:
    static TrueColor fromRGBA(const float r, const float g, const float b, const float a){
        uint8_t rgba[4];
        rgba[0]=(uint8_t)(r*255);
        rgba[1]=(uint8_t)(g*255);
        rgba[2]=(uint8_t)(b*255);
        rgba[3]=(uint8_t)(a*255);
        TrueColor trueColor;
        std::memcpy(&trueColor,rgba,sizeof(TrueColor));
        return trueColor;
    };
    static glm::vec4 toRGBA(const TrueColor trueColor){
        uint8_t rgba[4];
        std::memcpy(rgba,&trueColor,sizeof(rgba));
        return glm::vec4(rgba[0]/255.0f,rgba[1]/255.0f,rgba[2]/255.0f,rgba[3]/255.0f);
    };
    static std::string asString(const TrueColor trueColor){
        glm::vec4 rgba=toRGBA(trueColor);
        std::stringstream ss;
        ss<<"RGBA("<<rgba.r<<","<<rgba.g<<","<<rgba.b<<","<<rgba.a<<")"<<trueColor;
        return ss.str();
    }
    //In Androids JAVA code colors are stored in argb, not rgba
    static TrueColor fromAndroid(const int argb){
        auto alpha=(uint8_t )((argb>>24) & 0xFF);
        auto red=  (uint8_t )((argb>>16) & 0xFF);
        auto green=(uint8_t )((argb>>8) & 0xFF);
        auto blue= (uint8_t )((argb   ) & 0xFF);
        uint8_t rgba[4];
        rgba[0]=red;
        rgba[1]=green;
        rgba[2]=blue;
        rgba[3]=alpha;
        TrueColor ret;
        std::memcpy(&ret,rgba,sizeof(TrueColor));
        return ret;
    }
public:
    inline static const TrueColor BLACK=fromRGBA(0,0,0,1);
    inline static const TrueColor TRANSPARENT=fromRGBA(0,0,0,0);
    inline static const TrueColor WHITE=fromRGBA(1,1,1,1);
    inline static const TrueColor RED=fromRGBA(1,0,0,1);
    inline static const TrueColor GREEN=fromRGBA(0,1,0,1);
    inline static const TrueColor BLUE=fromRGBA(0,0,1,1);
    inline static const TrueColor YELLOW= fromRGBA(1.0f, 1.0f, 0.0f, 1);
};


/*static TrueColor fromAndroid(const int color){
        uint8_t argb[4];
        std::memcpy(argb,&color,sizeof(TrueColor));
        uint8_t rgba[4];
        rgba[0]=argb[1];
        rgba[1]=argb[2];
        rgba[2]=argb[3];
        rgba[3]=argb[0];
        TrueColor ret;
        std::memcpy(&ret,rgba,sizeof(TrueColor));
        return ret;
    }*/


#endif //FPV_VR_GLPROGRAMS_HELPER_COLOR_H
