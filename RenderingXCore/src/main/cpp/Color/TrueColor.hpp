//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_COLOR_H
#define FPV_VR_GLPROGRAMS_HELPER_COLOR_H


#include <cstdint>
#include <string>
#include <sstream>
#include <cstring>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

// True Color means 8 bit per channel. OpenGL Uses RGBA by default, so I also use that layout for the TrueColor class
// 8 bits per channel * 4 channels = 32 bit / 4 bytes total
static constexpr const auto TRUE_COLOR_SIZE_BYTES=4;
// It is possible to upload a TrueColor instance to OpenGL buffers directly - see end of file
class TrueColor {
public:
    uint8_t r=0;
    uint8_t g=0;
    uint8_t b=0;
    uint8_t a=0;
    TrueColor()= default;
    /**
     * Create from 8 bit values in the range [0..255]
     */
    constexpr TrueColor(const uint8_t r2,const uint8_t g2,const uint8_t b2,const uint8_t a2):
    r(r2),g(g2),b(b2),a(a2){}
    /**
     *  Create from a 32 bit RGBA value where each channel takes 8 bits
     */
    constexpr TrueColor(const uint32_t rgba):
        r(static_cast<uint8_t>((rgba >> 24) & 0xFF)),
        g(static_cast<uint8_t>((rgba >> 16) & 0xFF)),
        b(static_cast<uint8_t>((rgba >> 8) & 0xFF)),
        a(static_cast<uint8_t>((rgba   ) & 0xFF)){}
    /**
    * Constructor that takes GL_RGBA32F
    * Which refers to 4 float values in the range [0..1].
    * glm::vec4 / vec3 are RGBA32F by default.
    */
    constexpr TrueColor(const glm::vec4& colorRGBA32F){
        r=(uint8_t)(colorRGBA32F.x*255); //read of member 'r' not allowed in constant expression
        g=(uint8_t)(colorRGBA32F.y*255);// .g
        b=(uint8_t)(colorRGBA32F.z*255);// .b
        a=(uint8_t)(colorRGBA32F.w*255);// .a
    }
    /**
     * Comparing two TrueColor values is straight forward
     */
    constexpr bool operator==(const TrueColor& y)const{
        return (r==y.r && g==y.g && b==y.b && a==y.a);
    }
    constexpr bool operator!=(const TrueColor& y)const{
        return !(*this==y);
    }
    /**
     * Automatic conversion to glm::vec4 and glm::vec3
     */
    constexpr operator glm::vec4()const{
        return glm::vec4(r/255.0f,g/255.0f,b/255.0f,a/255.0f);
    }
    constexpr operator glm::vec3()const{
        return glm::vec3(r/255.0f,g/255.0f,b/255.0f);
    }
    /**
     * In Androids JAVA code colors are stored in argb, not rgba
     * This is not a constructor for TrueColor to avoid confusion
     */
    static constexpr TrueColor ARGB(const int argb){
        const auto alpha=static_cast<uint8_t>((argb>>24) & 0xFF);
        const auto red=  static_cast<uint8_t>((argb>>16) & 0xFF);
        const auto green=static_cast<uint8_t>((argb>>8) & 0xFF);
        const auto blue= static_cast<uint8_t>((argb   ) & 0xFF);
        return TrueColor(red,green,blue,alpha);
    }
    uint32_t toUInt32()const{
        uint32_t tmp;
        auto tmp2={r,g,b,a};
        std::memcpy(&tmp,&tmp2,sizeof(uint32_t));
        return tmp;
    }
    std::string asString()const{
        glm::vec4 rgba32F= *this;
        std::stringstream ss;
        ss << "RGBA32F(" << rgba32F.r << "," << rgba32F.g << "," << rgba32F.b << "," << rgba32F.a << ")" << toUInt32();
        return ss.str();
    }
}__attribute__((packed));

// unfortunately I cannot just make them members of TrueColor class because of the
// definition of 'TrueColor' is not complete until the closing '}' Error. Should be fixed  with c++20
namespace TrueColor2{
    static constexpr const TrueColor BLACK=TrueColor(glm::vec4{0.0f, 0, 0, 1});
    static constexpr const TrueColor TRANSPARENT=TrueColor(glm::vec4{0.0f, 0, 0, 0});
    static constexpr const TrueColor WHITE=TrueColor(glm::vec4{1.0f, 1, 1, 1});
    static constexpr const TrueColor RED=TrueColor(glm::vec4{1.0f, 0, 0, 1});
    static constexpr const TrueColor GREEN=TrueColor(glm::vec4{0.0f, 1, 0, 1});
    static constexpr const TrueColor BLUE=TrueColor(glm::vec4{0.0f, 0, 1, 1});
    static constexpr const TrueColor YELLOW=TrueColor(glm::vec4{1.0f, 1.0f, 0.0f, 1});
    static constexpr const TrueColor ORANGE=TrueColor(glm::vec4{1,0.5f,0,1.0f});
    static constexpr const TrueColor GREY=TrueColor(glm::vec4{0.5f,0.5f,0.5f,1.0f});

    //error: conversion function must be a non-static member function
    //static constexpr operator TrueColor(const glm::vec4 colorRGBA32F){
    //    return TrueColor(0);
    //}
}

// Make sure the compiler does not add any padding AND
// That the size of a cpp class does not depend on how many member functions usw it has
// (Still having trouble wrapping my head around that)
namespace TrueColorTest{
    static_assert(sizeof(TrueColor)==TRUE_COLOR_SIZE_BYTES);
    struct ExampleVertex{
        float x;
        float y;
        TrueColor color1;
        TrueColor color2;
    }__attribute__((packed));
    static_assert(sizeof(ExampleVertex)==2*TRUE_COLOR_SIZE_BYTES+2*sizeof(float));
    static void example(){
        TrueColor c=TrueColor2::BLACK;
        glm::vec4 x=c;
    }
}






#endif //FPV_VR_GLPROGRAMS_HELPER_COLOR_H
