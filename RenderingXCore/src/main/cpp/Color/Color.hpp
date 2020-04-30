//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_COLOR_H
#define FPV_VR_GLPROGRAMS_HELPER_COLOR_H


#include <cstdint>
#include <glm/vec4.hpp>
#include <string>
#include <sstream>

// since OpenGL uses RGBA,I also use RGBA for TrueColor. Make sure to convert when using android ColorPreference (which uses ARGB)
// True color is 8 bit per channel ,e.g 8*4 = 32 bit in total
struct TrueColor{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
}__attribute__((packed));

// Workaround for
// http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#1626
// constexpr member functions in brace-or-equal-initializers
namespace HideMe{
    static constexpr TrueColor fromRGBA(const float r, const float g, const float b, const float a){
        return {
                (uint8_t)(r*255),
                (uint8_t)(g*255),
                (uint8_t)(b*255),
                (uint8_t)(a*255),
        };
    };
}

class Color {
public:
    static constexpr TrueColor fromRGBA(const float r, const float g, const float b, const float a){
        return {
                (uint8_t)(r*255),
                (uint8_t)(g*255),
                (uint8_t)(b*255),
                (uint8_t)(a*255),
        };
    };
    //In Androids JAVA code colors are stored in argb, not rgba
    static constexpr TrueColor fromAndroid(const int argb){
        auto alpha=(uint8_t )((argb>>24) & 0xFF);
        auto red=  (uint8_t )((argb>>16) & 0xFF);
        auto green=(uint8_t )((argb>>8) & 0xFF);
        auto blue= (uint8_t )((argb   ) & 0xFF);
        return {
            red,green,blue,alpha
        };
    }
    static glm::vec4 toRGBA(const TrueColor trueColor){
        return glm::vec4(trueColor.r/255.0f,trueColor.g/255.0f,trueColor.b/255.0f,trueColor.a/255.0f);
    };
    static uint32_t asUInt32(const TrueColor trueColor){
        uint32_t tmp;
        memcpy(&tmp,&trueColor,sizeof(uint32_t));
        return tmp;
    }
    static std::string asString(const TrueColor trueColor){
        glm::vec4 rgba=toRGBA(trueColor);
        std::stringstream ss;
        ss<<"RGBA("<<rgba.r<<","<<rgba.g<<","<<rgba.b<<","<<rgba.a<<")"<<asUInt32(trueColor);
        return ss.str();
    }
public:
    // Mustn't use class members for constexpr here
    static constexpr TrueColor BLACK=HideMe::fromRGBA(0,0,0,1);
    static constexpr TrueColor TRANSPARENT=HideMe::fromRGBA(0,0,0,0);
    static constexpr TrueColor WHITE=HideMe::fromRGBA(1,1,1,1);
    static constexpr TrueColor RED=HideMe::fromRGBA(1,0,0,1);
    static constexpr TrueColor GREEN=HideMe::fromRGBA(0,1,0,1);
    static constexpr TrueColor BLUE=HideMe::fromRGBA(0,0,1,1);
    static constexpr TrueColor YELLOW= HideMe::fromRGBA(1.0f, 1.0f, 0.0f, 1);
};

#endif //FPV_VR_GLPROGRAMS_HELPER_COLOR_H
