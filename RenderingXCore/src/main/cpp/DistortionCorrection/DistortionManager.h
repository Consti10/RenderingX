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
    //This is the inverse function to the kN distortion parameters from the headset
    //Even tough the distortion parameters are only up to 2 radial values,
    //We need more for the inverse for a good fit
    static constexpr const int N_RADIAL_UNDISTORTION_COEFICIENTS=8;
    //These make up a Polynomial radial distortion with input inside intervall [0..maxRadSq]
    struct RadialDistortionCoefficients{
        float maxRadSquared;
        std::array<float,N_RADIAL_UNDISTORTION_COEFICIENTS> kN;
    };
    struct UndistortionHandles{
        GLuint uMaxRadSq;
        GLuint uKN;
        //Only active if mode==2
        GLuint uScreenParams_w;
        GLuint uScreenParams_h;
        GLuint uScreenParams_x_off;
        GLuint uScreenParams_y_off;
        //
        GLuint uTextureParams_w;
        GLuint uTextureParams_h;
        GLuint uTextureParams_x_off;
        GLuint uTextureParams_y_off;
    };
    //NONE: Nothing is distorted,use a normal gl_Position=MVP*pos; multiplication
    //RADIAL: Distortion in view space as done in cardboard design lab. Deprectaed.
    //CARDBOARD: Distortion in ? space, creates exact same distorted position(s) as in google cardboard https://github.com/googlevr/cardboard
    enum DISTORTION_MODE{NONE,RADIAL_VIEW_SPACE,RADIAL_CARDBOARD};
private:
    RadialDistortionCoefficients radialDistortionCoefficients;
    const DISTORTION_MODE distortionMode;
    //Left and right eye each
    std::array<MLensDistortion::ViewportParams,2> screen_params;
    std::array<MLensDistortion::ViewportParams,2> texture_params;
public:
    bool leftEye=true;
    //Default: no distortion whatsoever
    DistortionManager():distortionMode(DISTORTION_MODE::NONE){};
    DistortionManager(const DISTORTION_MODE& distortionMode1):distortionMode(distortionMode1){updateDistortionWithIdentity();}

    UndistortionHandles getUndistortionUniformHandles(const GLuint program)const;
    void beforeDraw(const UndistortionHandles& undistortionHandles)const;
    void afterDraw()const;

    static std::string writeDistortionParams(const DistortionManager& distortionManager);
    static std::string writeGLPosition(const DistortionManager& distortionManager,const std::string &positionAttribute="aPosition");

    void updateDistortion(const MPolynomialRadialDistortion& distortion,float maxRadSq);
    void updateDistortion(const MPolynomialRadialDistortion& inverseDistortion,float maxRadSq,
                          const std::array<MLensDistortion::ViewportParams,2> screen_params,const std::array<MLensDistortion::ViewportParams,2> texture_params);
    //Identity leaves x,y values untouched (as if no vddc was enabled in the shader)
    void updateDistortionWithIdentity();
    //
    std::string getModeAsString()const{
        if(distortionMode==NONE)return "NONE";
        if(distortionMode==RADIAL_VIEW_SPACE)return "RADIAL_VIEW_SPACE";
        if(distortionMode==RADIAL_CARDBOARD)return "RADIAL_CARDBOARD";
        return "ERROR";
    }
};


#endif //RENDERINGX_DISTORTIONMANAGER_H
