//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_COLOR_H
#define FPV_VR_GLPROGRAMS_HELPER_COLOR_H


#include <cstdint>
#include <glm/vec4.hpp>
#include <string>
#include <sstream>


// True Color means 8 bit per channel . OpenGL Uses RGBA by default, so I also use that layout for the TrueColor class
// 8 bits per channel * 4 channels = 32 bit / 4 bytes total
static constexpr const auto TRUE_COLOR_SIZE_BYTES=4;
// It is possible to upload a TrueColor instance to OpenGL directly - see end of file
class TrueColor {
public:
    uint8_t r=0;
    uint8_t g=0;
    uint8_t b=0;
    uint8_t a=0;
    TrueColor()= default;
    // Create from 8 bit values in the range [0..255]
    TrueColor(const uint8_t r,const uint8_t g,const uint8_t b,const uint8_t a){
        this->r=r;
        this->g=g;
        this->b=a;
        this->a=a;
    }
    // Create from a 32 bit value where each channel takes 8 bits
    TrueColor(const uint32_t trueColorRGBA){
        uint8_t tmp[4];
        memcpy(&tmp,&trueColorRGBA,sizeof(uint32_t));
        r=tmp[0];
        g=tmp[1];
        b=tmp[2];
        a=tmp[3];
    }
    // Comparing two TrueColor values is straight forward
    bool operator==(const TrueColor& y)const{
        return (r==y.r && g==y.g && b==y.b && a==y.a);
    }
    bool operator!=(const TrueColor& y)const{
        return !(*this==y);
    }
public:
    /**
     * Methods to convert from / to GL_RGBA32F
     * Which refers to 4 float values in the range [0..1].
     */
    static TrueColor RGBA32F(const float r, const float g, const float b, const float a){
        return TrueColor((uint8_t)(r*255),(uint8_t)(g*255),(uint8_t)(b*255),(uint8_t)(a*255));
    }
    static TrueColor RGBA32F(const glm::vec4& rgba){
        return RGBA32F(rgba.r,rgba.g,rgba.b,rgba.a);
    }
    glm::vec4 RGBA32F()const{
        return glm::vec4(r/255.0f,g/255.0f,b/255.0f,a/255.0f);
    };
public:
    //In Androids JAVA code colors are stored in argb, not rgba
    static TrueColor ARGB(const int argb){
        const auto alpha=(uint8_t )((argb>>24) & 0xFF);
        const auto red=  (uint8_t )((argb>>16) & 0xFF);
        const auto green=(uint8_t )((argb>>8) & 0xFF);
        const auto blue= (uint8_t )((argb   ) & 0xFF);
        return TrueColor(red,green,blue,alpha);
    }
    uint32_t toUInt32()const{
        uint32_t tmp;
        auto tmp2={r,g,b,a};
        memcpy(&tmp,&tmp2,sizeof(uint32_t));
        return tmp;
    }
    std::string asString()const{
        glm::vec4 rgba= RGBA32F();
        std::stringstream ss;
        ss << "RGBA(" << rgba.r << "," << rgba.g << "," << rgba.b << "," << rgba.a << ")" << toUInt32();
        return ss.str();
    }
}__attribute__((packed));

// unfortunately I cannot just make them members of TrueColor class because of the
// definition of 'TrueColor' is not complete until the closing '}' Error. Should be fixed  with c++20
namespace TrueColor2{
    static const TrueColor BLACK=TrueColor::RGBA32F(0.0f, 0, 0, 1);
    static const TrueColor TRANSPARENT=TrueColor::RGBA32F(0.0f, 0, 0, 0);
    static const TrueColor WHITE=TrueColor::RGBA32F(1.0f, 1, 1, 1);
    static const TrueColor RED=TrueColor::RGBA32F(1.0f, 0, 0, 1);
    static const TrueColor GREEN=TrueColor::RGBA32F(0.0f, 1, 0, 1);
    static const TrueColor BLUE=TrueColor::RGBA32F(0.0f, 0, 1, 1);
    static const TrueColor YELLOW=TrueColor::RGBA32F(1.0f, 1.0f, 0.0f, 1);
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
}






#endif //FPV_VR_GLPROGRAMS_HELPER_COLOR_H
