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
#include "LensDistortion/MLensDistortion.h"


/*
 * V.D.D.C stands for Vertex Displacement Distortion Correction.
 * This file contains everything needed to write a shader with V.D.D.C
 * See GLProgramTexture for an example OpenGL Shader using V.D.D.C
 * For technical details,see https://www.youtube.com/watch?v=yJVkdsZc9YA
 * Update 30.06.2020 : Creates exact same distorted position(s) as in google cardboard https://github.com/googlevr/cardboard
 */
// TODO use namespace instead of class but something goes wrong with linkage when doing so
class VDDC{
public:
    // Number of coefficients for the inverse distortion. This value needs to be present at compile time
    static constexpr const int N_RADIAL_UNDISTORTION_COEFICIENTS=8;
    /**
     * These values make up a Polynomial radial distortion with input inside interval [0..maxRadSq].
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
        std::array<MLensDistortion::ViewportParamsHSNDC,2> screen_params;
        std::array<MLensDistortion::ViewportParamsHSNDC,2> texture_params;
        static DataUnDistortion identity(){
            const PolynomialRadialInverse identity(VDDC::N_RADIAL_UNDISTORTION_COEFICIENTS);
            const MLensDistortion::ViewportParamsHSNDC identityParams=MLensDistortion::ViewportParamsHSNDC::identity();
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
    static UnDistortionUniformHandles getUndistortionUniformHandles(const GLuint program);
    /**
     * Write shader utility functions and the uniforms needed for VDDC
     * @return String usable inside OpenGL vertex shader
     */
    static std::string writeDistortionUtilFunctionsAndUniforms();
    /**
     * Update the uniform values with the UnDistortion data
     * @param leftEye true if uniforms should be updated for rendering the left eye, false for right eye
     */
    static void updateUnDistortionUniforms(const bool leftEye, const UnDistortionUniformHandles& undistortionHandles, const DataUnDistortion& dataUnDistortion);
    /**
     * NOTE: Following functions all return GLSL shader code as a string
     */
    static float PolynomialDistortionFactor(const float r_squared,const std::array<float,N_RADIAL_UNDISTORTION_COEFICIENTS> coefficients){
        float ret=0.0f;
        for(int i=N_RADIAL_UNDISTORTION_COEFICIENTS-1;i>=0;i--){
            ret = r_squared * (ret + coefficients[i]);
        }
        return 1.0f+ret;
    }
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
    //PolynomialRadialInverse data for GLSL
    static std::string glsl_struct_PolynomialRadialInverse(const int N_COEFICIENTS){
        return "struct DataPolynomialRadialInverse{\n"
               "float coefficients["+std::to_string(N_COEFICIENTS)+"];\n"
                "float maxRadSq;\n"
                "};\n";
    }
    static glm::vec2 PolynomialDistort(const DataPolynomialRadialInverse data,const glm::vec2 in_pos){
        float r2=glm::dot(in_pos,in_pos);
        r2=glm::clamp(r2,0.0f,data.maxRadSquared);
        float dist_factor=PolynomialDistortionFactor(r2,data.kN);
        glm::vec2 ret=in_pos*dist_factor;
        return ret;
    }
    //same as PolynomialRadialInverse::Distort (e.g. [0...maxRadSq]) (have to use c-style for GLSL)
    static std::string glsl_PolynomialDistort(){
        return "vec2 PolynomialDistort(const in DataPolynomialRadialInverse inv,const in vec2 in_pos){\n"
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
    static glm::vec2 UndistortedNDCForDistortedNDC(const DataPolynomialRadialInverse inv,const ViewportParamsHSNDC screen_params,const ViewportParamsHSNDC texture_params,const glm::vec2 in_ndc){
        glm::vec2 distorted_ndc_tanangle=glm::vec2(
        in_ndc.x * texture_params.width+texture_params.x_eye_offset,
        in_ndc.y * texture_params.height+texture_params.y_eye_offset);
        glm::vec2 undistorted_ndc_tanangle = PolynomialDistort(inv,distorted_ndc_tanangle);
        glm::vec2 ret=glm::vec2(undistorted_ndc_tanangle.x*screen_params.width+screen_params.x_eye_offset,
        undistorted_ndc_tanangle.y*screen_params.height+screen_params.y_eye_offset);
        return ret;
    }
    //Same as MLensDistortion::UndistortedNDCForDistortedNDC but with maxRadSq
    static std::string glsl_UndistortedNDCForDistortedNDC(){
        return "vec2 UndistortedNDCForDistortedNDC("
               "const in DataPolynomialRadialInverse inv,const in ViewportParams screen_params,const in ViewportParams texture_params,const in vec2 in_ndc){\n"
               "vec2 distorted_ndc_tanangle=vec2("
               "in_ndc.x * texture_params.width+texture_params.x_eye_offset,"
               "in_ndc.y * texture_params.height+texture_params.y_eye_offset);\n"
               "vec2 undistorted_ndc_tanangle = PolynomialDistort(inv,distorted_ndc_tanangle);\n"
               "vec2 ret=vec2(undistorted_ndc_tanangle.x*screen_params.width+screen_params.x_eye_offset,"
               "undistorted_ndc_tanangle.y*screen_params.height+screen_params.y_eye_offset);\n"
               "return ret;\n"
               "}\n";
    }
    /**
     *
     * @param in_polynomialRadialInverse the polynomial radial inverse, same for left and right eye
     * @param in_screen_params left or right eye screen params, as obtained by LensDistortion
     * @param in_texture_params left or right eye texture params, as obtained by LensDistortion
     * @param in_MVMatrix model view matrix
     * @param in_PMatrix projection matrix, as obtained by LensDistortion
     * @param in_vertex  position in 3d space
     * @return can be used diretcly for gl_Position - transformed, projected and undistorted position of the input vector
     */
    static glm::vec4 CalculateVertexPosition(const DataPolynomialRadialInverse in_polynomialRadialInverse,const ViewportParamsHSNDC in_screen_params,const ViewportParamsHSNDC in_texture_params,
        const glm::mat4 in_MVMatrix,const glm::mat4 in_PMatrix,const glm::vec4 in_vertex){
        glm::vec4 pos_view=in_MVMatrix*in_vertex;
        glm::vec4 pos_clip=in_PMatrix*pos_view;
        glm::vec3 ndc=glm::vec3(pos_clip)/pos_clip.w;
        glm::vec2 dist_p=UndistortedNDCForDistortedNDC(in_polynomialRadialInverse,in_screen_params,in_texture_params,ndc);
        glm::vec4 lola=glm::vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);
        return glm::vec4(glm::vec3(lola)/lola.w,1.0);
    }
    static std::string glsl_CalculateVertexPosition(){
        return "vec4 CalculateVertexPosition(const in DataPolynomialRadialInverse in_polynomialRadialInverse,const in ViewportParams in_screen_params,const in ViewportParams in_texture_params,"
               "const in mat4 in_MVMatrix,const in mat4 in_PMatrix,const in vec4 in_vertex){\n"
        "vec4 pos_view=in_MVMatrix*in_vertex;\n"
        "vec4 pos_clip=in_PMatrix*pos_view;\n"
        "vec3 ndc=pos_clip.xyz/pos_clip.w;\n"
        "vec2 dist_p=UndistortedNDCForDistortedNDC(in_polynomialRadialInverse,in_screen_params,in_texture_params,ndc.xy);\n"
        //s<<"gl_Position=vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);\n";
        "vec4 lola=vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);\n"
        "return vec4(lola.xyz/lola.w,1.0);\n"
        "}\n";
    }
};



#endif //RENDERINGX_VDDC_H
