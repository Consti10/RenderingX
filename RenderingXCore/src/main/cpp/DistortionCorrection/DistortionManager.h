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
#include <sys/stat.h>
#include "Helper/GLHelper.hpp"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Helper/NDKHelper.h>
#include "android/log.h"
#include "MLensDistortion.h"


/*
 * Handles Vertex Displacement Distortion Correction. See GLPrograms for some example OpenGL Shader
 * with V.D.D.C
 */

class DistortionManager {
public:
    //NONE: Nothing is distorted,use a normal gl_Position=MVP*pos; multiplication
    //RADIAL: Distortion in view space as done in cardboard design lab. Deprecated.
    //CARDBOARD: Distortion in ?tan-angle? space, creates exact same distorted position(s) as in google cardboard https://github.com/googlevr/cardboard
    enum DISTORTION_MODE{NONE,RADIAL_VIEW_SPACE,RADIAL_CARDBOARD};
    //Default: no distortion whatsoever
    DistortionManager():distortionMode(DISTORTION_MODE::NONE){};
    DistortionManager(const DISTORTION_MODE& distortionMode1):distortionMode(distortionMode1){updateDistortionWithIdentity();}
    //Number of coefficients for the inverse distortion. This value needs to be present at compile time
    static constexpr const int N_RADIAL_UNDISTORTION_COEFICIENTS=8;
    //These make up a Polynomial radial distortion with input inside interval [0..maxRadSq].
    //Note that the N of coefficients is NOT dynamic !
    struct DataPolynomialRadialInverse{
        float maxRadSquared;
        std::array<float,N_RADIAL_UNDISTORTION_COEFICIENTS> kN;
    };
    //Each GLSL shader programs needs these uniform handles. See DistortionManager::getUnDistortionUniformHandles
    struct UndistortionHandles{
        GLuint uPolynomialRadialInverse_maxRadSq;
        GLuint uPolynomialRadialInverse_coefficients;
        //Only active if mode==RADIAL_CARDBOARD
        GLuint uScreenParams_w;
        GLuint uScreenParams_h;
        GLuint uScreenParams_x_off;
        GLuint uScreenParams_y_off;
        GLuint uTextureParams_w;
        GLuint uTextureParams_h;
        GLuint uTextureParams_x_off;
        GLuint uTextureParams_y_off;
    };
    //Each GLSL program needs to bind its own distortion parameter uniforms
    //TODO when using OpenGL ES 3.0 use uniform buffers for that
    //Returns null if DistortionManager is null or disabled
    static UndistortionHandles* getUndistortionUniformHandles(const DistortionManager* distortionManager,const GLuint program);
    void beforeDraw(const UndistortionHandles* undistortionHandles)const;
    void afterDraw()const;

    //Add this string to your GLSL vertex shader if enabled
    static std::string writeDistortionParams(const DistortionManager* distortionManager);
    //Write "gl_Position"  with or without Distortion
    static std::string writeGLPosition(const DistortionManager* distortionManager,const std::string &positionAttribute="aPosition");
    //successive calls to DistortionManager::beforeDraw will upload the new un-distortion parameters
    void updateDistortion(const PolynomialRadialInverse& distortion);
    void updateDistortion(const PolynomialRadialInverse& inverseDistortion,const std::array<MLensDistortion::ViewportParams,2> screen_params,const std::array<MLensDistortion::ViewportParams,2> texture_params);
    //Identity leaves x,y values untouched (as if no V.D.D.C was enabled in the shader)
    void updateDistortionWithIdentity();
    //Successive calls to DistortionManager::beforeDraw apply left or right eye distortion
    void setEye(bool leftEye){
        this->leftEye=leftEye;
    }
    //returns true if dm==nullptr or mode==NONE
    static bool isNullOrDisabled(const DistortionManager* dm){
        return dm==nullptr || dm->distortionMode==NONE;
    };
private:
    DataPolynomialRadialInverse radialDistortionCoefficients;
    const DISTORTION_MODE distortionMode;
    //Left and right eye each
    std::array<MLensDistortion::ViewportParams,2> screen_params;
    std::array<MLensDistortion::ViewportParams,2> texture_params;
    //Differentiate between left and right eye screen/texture params
    bool leftEye=true;
private:
    //All GLSL functions (declare before main in vertex shader, then use anywhere)
    //
    //same as PolynomialRadialDistortion::DistortionFactor but unrolled loop for easier optimization by compiler
    static std::string glsl_PolynomialDistortionFactor(const int N_COEFICIENTS){
        std::stringstream s;
        s<<"\nfloat PolynomialDistortionFactor(const in float r_squared,const in float coefficients["<<N_COEFICIENTS<<"]){\n";
        s<<"float ret = 0.0;\n";
        for(int i=N_COEFICIENTS-1;i>=0;i--){
            s<<"ret = r_squared * (ret + coefficients["<<i<<"]);\n";
        }
        s<<"return 1.0+ret;\n";
        s<<"}\n";
        return s.str();
    }
    //PolynomialRadialInverse
    static std::string glsl_struct_PolynomialRadialInverse(const int N_COEFICIENTS){
        return "struct PolynomialRadialInverse{\n"
               "float coefficients["+std::to_string(N_COEFICIENTS)+"];\n"
               "float maxRadSq;\n"
               "};\n";
    }
    //same as PolynomialRadialInverse::Distort (e.g. [0...maxRadSq])
    static std::string glsl_PolynomialDistort(){
        return "vec2 PolynomialDistort(const in PolynomialRadialInverse inv,const in vec2 in_pos){\n"
        "float r2=dot(in_pos.xy,in_pos.xy);\n"
        "r2=clamp(r2,0.0,inv.maxRadSq);\n"
        "float dist_factor=PolynomialDistortionFactor(r2,inv.coefficients);\n"
        "vec2 ret=in_pos.xy*dist_factor;\n"
        "return ret;\n"
        "}\n";
    }
    //Same as MLensDistortion::ViewportParams
    static std::string glsl_ViewportParams(){
        return "struct ViewportParams{\n"
           "  float width;\n"
           "  float height;\n"
           "  float x_eye_offset;\n"
           "  float y_eye_offset;\n"
           "};\n";
    }
    //Same as MLensDistortion::UndistortedNDCForDistortedNDC but with maxRadSq
    static std::string glsl_UndistortedNDCForDistortedNDC(){
        return "vec2 UndistortedNDCForDistortedNDC("
        "const in PolynomialRadialInverse inv,const in ViewportParams screen_params,const in ViewportParams texture_params,const in vec2 in_ndc){\n"
        "vec2 distorted_ndc_tanangle=vec2("
        "in_ndc.x * texture_params.width+texture_params.x_eye_offset,"
        "in_ndc.y * texture_params.height+texture_params.y_eye_offset);\n"
        "vec2 undistorted_ndc_tanangle = PolynomialDistort(inv,distorted_ndc_tanangle);\n"
        "vec2 ret=vec2(undistorted_ndc_tanangle.x*screen_params.width+screen_params.x_eye_offset,"
        "undistorted_ndc_tanangle.y*screen_params.height+screen_params.y_eye_offset);\n"
        "return ret;\n"
        "}\n";
    }
};


#endif //RENDERINGX_DISTORTIONMANAGER_H
