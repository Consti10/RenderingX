//
// Created by Consti10 on 31/10/2019.
//

#include "DistortionManager.h"

DistortionManager::UndistortionHandles*
DistortionManager::getUndistortionUniformHandles(const DistortionManager* dm,const GLuint program){
    if(isNullOrDisabled(dm)){
        return nullptr;
    }
    UndistortionHandles* ret=new UndistortionHandles();
    if(dm->distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        ret->uPolynomialRadialInverse_coefficients=(GLuint)glGetUniformLocation(program,"uPolynomialRadialInverse.coefficients");
        ret->uPolynomialRadialInverse_maxRadSq=(GLuint)glGetUniformLocation(program,"uPolynomialRadialInverse.maxRadSq");
    }else if(dm->distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        ret->uPolynomialRadialInverse_coefficients=(GLuint)glGetUniformLocation(program,"uPolynomialRadialInverse.coefficients");
        ret->uPolynomialRadialInverse_maxRadSq=(GLuint)glGetUniformLocation(program,"uPolynomialRadialInverse.maxRadSq");
        ret->uScreenParams_w=(GLuint)glGetUniformLocation(program,"uScreenParams.width");
        ret->uScreenParams_h=(GLuint)glGetUniformLocation(program,"uScreenParams.height");
        ret->uScreenParams_x_off=(GLuint)glGetUniformLocation(program,"uScreenParams.x_eye_offset");
        ret->uScreenParams_y_off=(GLuint)glGetUniformLocation(program,"uScreenParams.y_eye_offset");
        ret->uTextureParams_w=(GLuint)glGetUniformLocation(program,"uTextureParams.width");
        ret->uTextureParams_h=(GLuint)glGetUniformLocation(program,"uTextureParams.height");
        ret->uTextureParams_x_off=(GLuint)glGetUniformLocation(program,"uTextureParams.x_eye_offset");
        ret->uTextureParams_y_off=(GLuint)glGetUniformLocation(program,"uTextureParams.y_eye_offset");
    }
    return ret;
}

void DistortionManager::beforeDraw(
        const DistortionManager::UndistortionHandles* uh) const {
    if(distortionMode==NONE){
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
    }
}

void DistortionManager::afterDraw() const {
    //glBindTexture(GL_TEXTURE_2D,0);
}

std::string DistortionManager::writeDistortionParams(
        const DistortionManager* distortionManager) {
    if(isNullOrDisabled(distortionManager))
        return "";
    std::stringstream s;
    //Write all shader function(s) needed for VDDC
    const int N_COEFICIENTS=DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS;
    s<< glsl_struct_PolynomialRadialInverse(N_COEFICIENTS);
    s<<glsl_PolynomialDistortionFactor(N_COEFICIENTS);
    s<<glsl_PolynomialDistort();
    s<<glsl_ViewportParams();
    s<<glsl_UndistortedNDCForDistortedNDC();
    //The uniforms needed for vddc
    if(distortionManager->distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        s<<"uniform PolynomialRadialInverse uPolynomialRadialInverse;";
    }else if(distortionManager->distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        s<<"uniform PolynomialRadialInverse uPolynomialRadialInverse;";
        s<<"uniform ViewportParams uScreenParams;\n";
        s<<"uniform ViewportParams uTextureParams;\n";
    }
    return s.str();
}

std::string DistortionManager::writeGLPosition(const DistortionManager* distortionManager1,
                                                     const std::string &positionAttribute) {
    if(isNullOrDisabled(distortionManager1))
        return "gl_Position = (uPMatrix*uMVMatrix)* "+positionAttribute+";\n";;
    const DistortionManager& distortionManager=*distortionManager1;
    std::stringstream s;
    if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        s<<"vec4 pos=uMVMatrix*"+positionAttribute+";\n";
        s<<"float r2=dot(pos.xy,pos.xy)/(pos.z*pos.z);\n";
        //s<<"r2=clamp(r2,0.0,uMaxRadSq);\n";
        s<<"float dist_factor=PolynomialDistortionFactor(r2,uPolynomialRadialInverse.coefficients);\n";
        s<<"pos.xy*=dist_factor;\n";
        //s<<"gl_Position=pos;\n";
        s<<"gl_Position=uPMatrix*pos;\n";
        //s<<"gl_Position.x=gl_Position.x*uScreenParams.width+uScreenParams.x_eye_offset;";
        //s<<"gl_Position.y=gl_Position.y*uScreenParams.height+uScreenParams.y_eye_offset;";
    }else if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        s<<"vec4 pos_view=uMVMatrix*"+positionAttribute+";\n";
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
    return s.str();
}

void DistortionManager::updateDistortion(const PolynomialRadialInverse &inverseDistortion) {
    for(int i=0;i<inverseDistortion.getCoeficients().size();i++){
        radialDistortionCoefficients.kN[i]=inverseDistortion.getCoeficients()[i];
    }
    radialDistortionCoefficients.maxRadSquared=inverseDistortion.getMaxRadSq();
    //MDebug::log(inverseDistortion.toString());
}

void DistortionManager::updateDistortion(const PolynomialRadialInverse &inverseDistortion,
                                    const std::array<MLensDistortion::ViewportParams, 2> screen_params,
                                    const std::array<MLensDistortion::ViewportParams, 2> texture_params) {
    this->screen_params=screen_params;
    this->texture_params=texture_params;
    updateDistortion(inverseDistortion);
}

void DistortionManager::updateDistortionWithIdentity() {
    PolynomialRadialInverse identity(DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
    const MLensDistortion::ViewportParams identityParams{1,1,0,0};
    updateDistortion(identity,{identityParams,identityParams},{identityParams,identityParams});
}


