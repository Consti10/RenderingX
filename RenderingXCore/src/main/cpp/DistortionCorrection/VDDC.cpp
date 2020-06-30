//
// Created by Consti10 on 31/10/2019.
//

#include "VDDC.h"
constexpr auto TAG="VDDCManager";

std::string VDDC::writeDistortionUtilFunctionsAndUniforms() {
    std::stringstream s;
    //Write all shader function(s) needed for VDDC
    const int N_COEFICIENTS=VDDC::N_RADIAL_UNDISTORTION_COEFICIENTS;
    s<< glsl_struct_PolynomialRadialInverse(N_COEFICIENTS);
    s<<glsl_PolynomialDistortionFactor(N_COEFICIENTS);
    s<<glsl_PolynomialDistort();
    s<<glsl_ViewportParams();
    s<<glsl_UndistortedNDCForDistortedNDC();
    //The uniforms needed for vddc
    s<<"uniform PolynomialRadialInverse uPolynomialRadialInverse;";
    s<<"uniform ViewportParams uScreenParams;\n";
    s<<"uniform ViewportParams uTextureParams;\n";
    return s.str();
}

std::string VDDC::writeDistortedGLPosition() {
    std::stringstream s;
    s<<"vec4 pos_view=uMVMatrix*aPosition;\n";
    s<<"vec4 pos_clip=uPMatrix*pos_view;\n";
    s<<"vec3 ndc=pos_clip.xyz/pos_clip.w;\n";
    s<<"vec2 dist_p=UndistortedNDCForDistortedNDC(uPolynomialRadialInverse,uScreenParams,uTextureParams,ndc.xy);\n";
    s<<"gl_Position=vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);\n";
    return s.str();
}

VDDC::UnDistortionUniformHandles*
VDDC::getUndistortionUniformHandles(const GLuint program){
    auto* ret=new UnDistortionUniformHandles();
    ret->uPolynomialRadialInverse_coefficients=GLHelper::GlGetUniformLocation(program,"uPolynomialRadialInverse.coefficients");
    ret->uPolynomialRadialInverse_maxRadSq=GLHelper::GlGetUniformLocation(program,"uPolynomialRadialInverse.maxRadSq");
    ret->uScreenParams_w=GLHelper::GlGetUniformLocation(program,"uScreenParams.width");
    ret->uScreenParams_h=GLHelper::GlGetUniformLocation(program,"uScreenParams.height");
    ret->uScreenParams_x_off=GLHelper::GlGetUniformLocation(program,"uScreenParams.x_eye_offset");
    ret->uScreenParams_y_off=GLHelper::GlGetUniformLocation(program,"uScreenParams.y_eye_offset");
    ret->uTextureParams_w=GLHelper::GlGetUniformLocation(program,"uTextureParams.width");
    ret->uTextureParams_h=GLHelper::GlGetUniformLocation(program,"uTextureParams.height");
    ret->uTextureParams_x_off=GLHelper::GlGetUniformLocation(program,"uTextureParams.x_eye_offset");
    ret->uTextureParams_y_off=GLHelper::GlGetUniformLocation(program,"uTextureParams.y_eye_offset");
    return ret;
}


void VDDC::updateUnDistortionUniforms(const bool leftEye, const UnDistortionUniformHandles& undistortionHandles, const DataUnDistortion& dataUnDistortion){
    glUniform1f(undistortionHandles.uPolynomialRadialInverse_maxRadSq,dataUnDistortion.radialDistortionCoefficients.maxRadSquared);
    glUniform1fv(undistortionHandles.uPolynomialRadialInverse_coefficients,N_RADIAL_UNDISTORTION_COEFICIENTS,dataUnDistortion.radialDistortionCoefficients.kN.data());
    const int i=leftEye ? 0 : 1; //update screen params
    glUniform1f(undistortionHandles.uScreenParams_w,dataUnDistortion.screen_params[i].width);
    glUniform1f(undistortionHandles.uScreenParams_h,dataUnDistortion.screen_params[i].height);
    glUniform1f(undistortionHandles.uScreenParams_x_off,dataUnDistortion.screen_params[i].x_eye_offset);
    glUniform1f(undistortionHandles.uScreenParams_y_off,dataUnDistortion.screen_params[i].y_eye_offset);
    //same for texture params
    glUniform1f(undistortionHandles.uTextureParams_w,dataUnDistortion.texture_params[i].width);
    glUniform1f(undistortionHandles.uTextureParams_h,dataUnDistortion.texture_params[i].height);
    glUniform1f(undistortionHandles.uTextureParams_x_off,dataUnDistortion.texture_params[i].x_eye_offset);
    glUniform1f(undistortionHandles.uTextureParams_y_off,dataUnDistortion.texture_params[i].y_eye_offset);
}

/*if(isNullOrDisabled(distortionManager1))
        return "gl_Position = (uPMatrix*uMVMatrix)* aPosition;\n";;
    const VDDCManager& distortionManager=*distortionManager1;
    std::stringstream s;
    if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        s<<"vec4 pos=uMVMatrix*aPosition;\n";
        s<<"float r2=dot(pos.xy,pos.xy)/(pos.z*pos.z);\n";
        //s<<"r2=clamp(r2,0.0,uMaxRadSq);\n";
        s<<"float dist_factor=PolynomialDistortionFactor(r2,uPolynomialRadialInverse.coefficients);\n";
        s<<"pos.xy*=dist_factor;\n";
        //s<<"gl_Position=pos;\n";
        s<<"gl_Position=uPMatrix*pos;\n";
        //s<<"gl_Position.x=gl_Position.x*uScreenParams.width+uScreenParams.x_eye_offset;";
        //s<<"gl_Position.y=gl_Position.y*uScreenParams.height+uScreenParams.y_eye_offset;";
    }else if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        s<<"vec4 pos_view=uMVMatrix*aPosition;\n";
        s<<"vec4 pos_clip=uPMatrix*pos_view;\n";
        s<<"vec3 ndc=pos_clip.xyz/pos_clip.w;\n";
        //s<<"bool inside=isInsideViewPort(ndc.xy);";
        //s<<"ndc.xy=clamp(ndc.xy,-1.0,1.0);";
        //s<<"if(inside){";
        s<<"vec2 dist_p=UndistortedNDCForDistortedNDC(uPolynomialRadialInverse,uScreenParams,uTextureParams,ndc.xy);\n";
        s<<"gl_Position=vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);\n";
        //s<<"}else{";
        //s<<"gl_Position=pos_clip;";
        //s<<"}";
        //s<<"gl_Position=vec4(lol,0,1);";
        //s<<"gl_Position=pos_clip;";
        //s<<"vec3 ndc=pos.xyz/pos.w;";
        //s<<"vec2 lol=ndc.xy/ndc.z;";
        //s<<"pos.xy=UndistortedNDCForDistortedNDC(uKN,uScreenParams,uTextureParams,ndc);";
        //s<<"gl_Position.x+=1.0*gl_Position.w;";
        //s<<"gl_Position.y+=1.0*gl_Position.w;";
        //s<<"pos.y+=-0.18;";
        //s<<"pos.x+=0.1;";
    }
    return s.str();*/

/*if(distortionMode==NONE){
       return;
   }else if(distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
       const UndistortionHandles& undistortionHandles=*uh;
       glUniform1f(undistortionHandles.uPolynomialRadialInverse_maxRadSq,radialDistortionCoefficients.maxRadSquared);
       glUniform1fv(undistortionHandles.uPolynomialRadialInverse_coefficients,N_RADIAL_UNDISTORTION_COEFICIENTS,radialDistortionCoefficients.kN.data());
   }else if(distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD) {
       const UndistortionHandles& undistortionHandles=*uh;
       glUniform1f(undistortionHandles.uPolynomialRadialInverse_maxRadSq,radialDistortionCoefficients.maxRadSquared);
       glUniform1fv(undistortionHandles.uPolynomialRadialInverse_coefficients,N_RADIAL_UNDISTORTION_COEFICIENTS,radialDistortionCoefficients.kN.data());
       const int i=leftEye ? 0 : 1; //update screen params
       glUniform1f(undistortionHandles.uScreenParams_w,screen_params[i].width);
       glUniform1f(undistortionHandles.uScreenParams_h,screen_params[i].height);
       glUniform1f(undistortionHandles.uScreenParams_x_off,screen_params[i].x_eye_offset);
       glUniform1f(undistortionHandles.uScreenParams_y_off,screen_params[i].y_eye_offset);
       //same for texture params
       glUniform1f(undistortionHandles.uTextureParams_w,texture_params[i].width);
       glUniform1f(undistortionHandles.uTextureParams_h,texture_params[i].height);
       glUniform1f(undistortionHandles.uTextureParams_x_off,texture_params[i].x_eye_offset);
       glUniform1f(undistortionHandles.uTextureParams_y_off,texture_params[i].y_eye_offset);
   }*/



