//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_COLOR_H
#define FPV_VR_GLPROGRAMS_HELPER_COLOR_H


#include <cstdint>
#include <glm/vec4.hpp>
#include <string>
#include <sstream>

// To improve performance and also ease the upload of TrueColor values to OpenGL
// I do not create a class named TrueColor but rather declare a TrueColor namespace which holds
// The value in a simple struct - called Color.
namespace TrueColor {
    // since OpenGL uses RGBA,I also use RGBA for TrueColor. Make sure to convert when using android ColorPreference (which uses ARGB)
    // True color is 8 bit per channel ,e.g 8*4 = 32 bit in total
    struct Color{
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
        // Comparing two TrueColor values is straight forward
        bool operator==(const Color& y)const{
            return (r==y.r && g==y.g && b==y.b && a==y.a);
        }
        bool operator!=(const Color& y)const{
            return !(*this==y);
        }

    }__attribute__((packed));
    static_assert(sizeof(Color)==4);
    // Create a TrueColor value from rgba values [0..1]
    static constexpr Color fromRGBA(const float r, const float g, const float b, const float a){
        return {
                (uint8_t)(r*255),
                (uint8_t)(g*255),
                (uint8_t)(b*255),
                (uint8_t)(a*255),
        };
    };
    //In Androids JAVA code colors are stored in argb, not rgba
    static constexpr Color fromAndroid(const int argb){
        auto alpha=(uint8_t )((argb>>24) & 0xFF);
        auto red=  (uint8_t )((argb>>16) & 0xFF);
        auto green=(uint8_t )((argb>>8) & 0xFF);
        auto blue= (uint8_t )((argb   ) & 0xFF);
        return {
            red,green,blue,alpha
        };
    }
    static glm::vec4 toRGBA(const Color trueColor){
        return glm::vec4(trueColor.r/255.0f,trueColor.g/255.0f,trueColor.b/255.0f,trueColor.a/255.0f);
    };
    static uint32_t asUInt32(const Color trueColor){
        uint32_t tmp;
        memcpy(&tmp,&trueColor,sizeof(uint32_t));
        return tmp;
    }
    static std::string asString(const Color trueColor){
        glm::vec4 rgba=toRGBA(trueColor);
        std::stringstream ss;
        ss<<"RGBA("<<rgba.r<<","<<rgba.g<<","<<rgba.b<<","<<rgba.a<<")"<<asUInt32(trueColor);
        return ss.str();
    }
    static constexpr Color BLACK=fromRGBA(0, 0, 0, 1);
    static constexpr Color TRANSPARENT=fromRGBA(0, 0, 0, 0);
    static constexpr Color WHITE=fromRGBA(1, 1, 1, 1);
    static constexpr Color RED=fromRGBA(1, 0, 0, 1);
    static constexpr Color GREEN=fromRGBA(0, 1, 0, 1);
    static constexpr Color BLUE=fromRGBA(0, 0, 1, 1);
    static constexpr Color YELLOW=fromRGBA(1.0f, 1.0f, 0.0f, 1);

    static bool lol(){
        Color c1,c2;
        return c1==c2;
    };
};

//  At the time, I do not use anything else than TrueColor
// In the RenderingXCore library so in the end I make the namespace public.
// So I do not have to write TrueColor::Color everywhere
// is this a bad habit - probably but I think the ease of use justifies it
using namespace TrueColor;





#endif //FPV_VR_GLPROGRAMS_HELPER_COLOR_H
