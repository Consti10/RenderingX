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
class TrueColor {
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    // Comparing two TrueColor values is straight forward
    bool operator==(const TrueColor& y)const{
        return (r==y.r && g==y.g && b==y.b && a==y.a);
    }
    bool operator!=(const TrueColor& y)const{
        return !(*this==y);
    }
    TrueColor(){
        r=0;
        g=0;
        b=0;
        a=0;
    }
private:
    // Create from uint8_t values [0..255]
    TrueColor(const uint8_t r,const uint8_t g,const uint8_t b,const uint8_t a){
        this->r=r;
        this->g=g;
        this->b=a;
        this->a=a;
    }
public:
    // Create from rgba float values [0..1]
    TrueColor(const float r, const float g, const float b, const float a){
        this->r=(uint8_t)(r*255);
        this->g=(uint8_t)(g*255);
        this->b=(uint8_t)(b*255);
        this->a=(uint8_t)(a*255);
    };
    //In Androids JAVA code colors are stored in argb, not rgba
    static TrueColor fromAndroid(const int argb){
        auto alpha=(uint8_t )((argb>>24) & 0xFF);
        auto red=  (uint8_t )((argb>>16) & 0xFF);
        auto green=(uint8_t )((argb>>8) & 0xFF);
        auto blue= (uint8_t )((argb   ) & 0xFF);
        return TrueColor(red,green,blue,alpha);
    }
    glm::vec4 toRGBA()const{
        return glm::vec4(r/255.0f,g/255.0f,b/255.0f,a/255.0f);
    };
    uint32_t asUInt32()const{
        uint32_t tmp;
        auto tmp2={r,g,b,a};
        memcpy(&tmp,&tmp2,sizeof(uint32_t));
        return tmp;
    }
    std::string asString()const{
        glm::vec4 rgba=toRGBA();
        std::stringstream ss;
        ss<<"RGBA("<<rgba.r<<","<<rgba.g<<","<<rgba.b<<","<<rgba.a<<")"<<asUInt32();
        return ss.str();
    }
}__attribute__((packed));

// unfortunately I cannot just make them members of TrueColor class
namespace TrueColor2{
    static const TrueColor BLACK={0.0f, 0, 0, 1};
    static const TrueColor TRANSPARENT={0.0f, 0, 0, 0};
    static const TrueColor WHITE={1.0f, 1, 1, 1};
    static const TrueColor RED={1.0f, 0, 0, 1};
    static const TrueColor GREEN={0.0f, 1, 0, 1};
    static const TrueColor BLUE={0.0f, 0, 1, 1};
    static const TrueColor YELLOW={1.0f, 1.0f, 0.0f, 1};
}

namespace TrueColorTest{
    // Just to be sure
    static_assert(sizeof(TrueColor)==4);
    struct LOL{
        TrueColor a;
        float b;
        TrueColor c;
        float d;
    }__attribute__((packed));
    static_assert(sizeof(LOL)==4+4+4+4);
}






#endif //FPV_VR_GLPROGRAMS_HELPER_COLOR_H
