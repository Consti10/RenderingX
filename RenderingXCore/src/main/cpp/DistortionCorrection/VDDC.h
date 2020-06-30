//
// Created by Consti10 on 31/10/2019.
//

#ifndef RENDERINGX_VDDC_H
#define RENDERINGX_VDDC_H

#include <array>
#include <string>
#include <sstream>
#include <AndroidLogger.hpp>
#include <vector>
#include <sys/stat.h>
#include <GLHelper.hpp>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <NDKHelper.hpp>
#include "android/log.h"
#include "MLensDistortion.h"


/*
 * V.D.D.C stands for Vertex Displacement Distortion Correction.
 * See GLProgramTexture for an example OpenGL Shader using V.D.D.C
 * For technical details,see https://www.youtube.com/watch?v=yJVkdsZc9YA
 * Update 30.06.2020 : Creates exact same distorted position(s) as in google cardboard https://github.com/googlevr/cardboard
 */

class VDDC{
public:
    // Number of coefficients for the inverse distortion. This value needs to be present at compile time
    static constexpr const int N_RADIAL_UNDISTORTION_COEFICIENTS=8;
    /**
     * These make up a Polynomial radial distortion with input inside interval [0..maxRadSq].
     * Note that the N of coefficients is NOT dynamic !
     */
    struct DataPolynomialRadialInverse{
        float maxRadSquared;
        std::array<float,N_RADIAL_UNDISTORTION_COEFICIENTS> kN;
        DataPolynomialRadialInverse(const PolynomialRadialInverse &inverseDistortion){
            assert(inverseDistortion.getCoefficients().size()==N_RADIAL_UNDISTORTION_COEFICIENTS);
            for(int i=0;i<inverseDistortion.getCoefficients().size();i++){
                kN[i]=inverseDistortion.getCoefficients()[i];
            }
            maxRadSquared=inverseDistortion.getMaxRadSq();
        }
    };
    /**
     * This data is enough to calculate the undistorted vertices for both the left and right eye
     * The identity UnDistortion leaves the coordinates untouched  (usefully for debugging)
     */
    struct DataUnDistortion{
        // same for left and right eye
        DataPolynomialRadialInverse radialDistortionCoefficients;
        // one for Left and right eye each
        std::array<MLensDistortion::ViewportParams,2> screen_params;
        std::array<MLensDistortion::ViewportParams,2> texture_params;
        static DataUnDistortion identity(){
            const PolynomialRadialInverse identity(VDDC::N_RADIAL_UNDISTORTION_COEFICIENTS);
            const MLensDistortion::ViewportParams identityParams{1,1,0,0};
            return DataUnDistortion{{identity},{identityParams,identityParams},{identityParams,identityParams}};
        }
    };
    /**
     *  Each GLSL shader programs needs these uniform handles. See VDDC::getUnDistortionUniformHandles
     * They need to be updated in between rendering left and right eye
     */
    struct UnDistortionUniformHandles{
        GLuint uPolynomialRadialInverse_maxRadSq;
        GLuint uPolynomialRadialInverse_coefficients;
        GLuint uScreenParams_w;
        GLuint uScreenParams_h;
        GLuint uScreenParams_x_off;
        GLuint uScreenParams_y_off;
        GLuint uTextureParams_w;
        GLuint uTextureParams_h;
        GLuint uTextureParams_x_off;
        GLuint uTextureParams_y_off;
    };
    /**
     * Each GLSL program needs to bind its own distortion parameter uniforms.
     * @param program OpenGL shader program where the vertex shader contains the uniforms declared in VDDC::writeDistortionUtilFunctionsAndUniforms
     * TODO when using OpenGL ES 3.0 use uniform buffers for that
     */
    static UnDistortionUniformHandles* getUndistortionUniformHandles(const GLuint program);
    /**
     * Write shader utility functions and the uniforms needed for VDDC
     * @return String usable inside OpenGL vertex shader
     */
    static std::string writeDistortionUtilFunctionsAndUniforms();
    /**
     * Write "gl_Position" after applying undistortion
     * @return String usable inside OpenGL vertex shader
     */
    static std::string writeDistortedGLPosition();
    // update all the uniforms
    /**
     * Update the uniform values with the UnDistortion data
     * @param leftEye true if uniforms should be updated for rendering the left eye, false for right eye
     */
    static void updateUnDistortionUniforms(const bool leftEye, const UnDistortionUniformHandles& undistortionHandles, const DataUnDistortion& dataUnDistortion);
private:
    /**
     * NOTE: Following functions all return GLSL shader code as a string
     */
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



#endif //RENDERINGX_VDDC_H
