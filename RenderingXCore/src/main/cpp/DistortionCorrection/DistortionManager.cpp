//
// Created by Consti10 on 31/10/2019.
//

#include "DistortionManager.h"

DistortionManager::UndistortionHandles
DistortionManager::getUndistortionUniformHandles(const GLuint program) const {
    UndistortionHandles ret{};
    if(distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        ret.uMaxRadSq=(GLuint)glGetUniformLocation(program,"uMaxRadSq");
        ret.uKN=(GLuint)glGetUniformLocation(program,"uKN");
    }else if(distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        ret.uMaxRadSq=(GLuint)glGetUniformLocation(program,"uMaxRadSq");
        ret.uKN=(GLuint)glGetUniformLocation(program,"uKN");
        ret.uScreenParams_w=(GLuint)glGetUniformLocation(program,"uScreenParams.width");
        ret.uScreenParams_h=(GLuint)glGetUniformLocation(program,"uScreenParams.height");
        ret.uScreenParams_x_off=(GLuint)glGetUniformLocation(program,"uScreenParams.x_eye_offset");
        ret.uScreenParams_y_off=(GLuint)glGetUniformLocation(program,"uScreenParams.y_eye_offset");
        ret.uTextureParams_w=(GLuint)glGetUniformLocation(program,"uTextureParams.width");
        ret.uTextureParams_h=(GLuint)glGetUniformLocation(program,"uTextureParams.height");
        ret.uTextureParams_x_off=(GLuint)glGetUniformLocation(program,"uTextureParams.x_eye_offset");
        ret.uTextureParams_y_off=(GLuint)glGetUniformLocation(program,"uTextureParams.y_eye_offset");
    }
    return ret;
}

void DistortionManager::beforeDraw(
        const DistortionManager::UndistortionHandles& undistortionHandles) const {
    if(distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        glUniform1f(undistortionHandles.uMaxRadSq,radialDistortionCoefficients.maxRadSquared);
        glUniform1fv(undistortionHandles.uKN,N_RADIAL_UNDISTORTION_COEFICIENTS,radialDistortionCoefficients.kN.data());
    }else if(distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD) {
        glUniform1f(undistortionHandles.uMaxRadSq,radialDistortionCoefficients.maxRadSquared);
        glUniform1fv(undistortionHandles.uKN,N_RADIAL_UNDISTORTION_COEFICIENTS,radialDistortionCoefficients.kN.data());
        const int i=leftEye ? 0 : 1;
        glUniform1f(undistortionHandles.uScreenParams_w,screen_params[i].width);
        glUniform1f(undistortionHandles.uScreenParams_h,screen_params[i].height);
        glUniform1f(undistortionHandles.uScreenParams_x_off,screen_params[i].x_eye_offset);
        glUniform1f(undistortionHandles.uScreenParams_y_off,screen_params[i].y_eye_offset);

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
        const DistortionManager& distortionManager) {
    std::stringstream s;
    if(distortionManager.distortionMode==NONE){
        return "";
    }
    const int N_COEFICIENTS=DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS;

    //All GLSL functions (declare before main in vertex shader, then use anywhere)
    //
    //same as PolynomialRadialDistortion::DistortionFactor but unrolled loop for easier optimization by compiler
    s<<"float PolynomialDistortionFactor(const in float r_squared,const in float["<<N_COEFICIENTS<<"] coefficients){\n";
    //manually unrolled loop
    s<<"float ret = 0.0;\n";
    for(int i=N_COEFICIENTS-1;i>=0;i--){
        s<<"ret = r_squared * (ret + coefficients["<<i<<"]);\n";
    }
    s<<"return 1.0+ret;\n";
    s<<"}\n";

    //s<<"varying float invisibleFragment;";

    //same as PolynomialRadialDistortion::Distort
    //But with maxRadSq as limit
    s<<"vec2 PolynomialDistort(const in float["<<N_COEFICIENTS<<"] coefficients,const in vec2 in_pos,const in float maxRadSq){\n";
    s<<"float r2=dot(in_pos.xy,in_pos.xy);\n";
    //s<<"invisibleFragment=-1.0;";
    //s<<"if(r2>1.0){";
    //s<<"invisibleFragment=1.0;";
    //s<<"}";
    s<<"r2=clamp(r2,0.0,maxRadSq);";
    s<<"float dist_factor=PolynomialDistortionFactor(r2,coefficients);";
    s<<"vec2 ret=in_pos.xy*dist_factor;\n";
    s<<"return ret;\n";
    s<<"}";

    //Same as MLensDistortion::ViewportParams
    s<<"struct ViewportParams\n"
       "{\n"
       "  float width;\n"
       "  float height;\n"
       "  float x_eye_offset;\n"
       "  float y_eye_offset;\n"
       "};";
    //Same as MLensDistortion::UndistortedNDCForDistortedNDC
    s<<"vec2 UndistortedNDCForDistortedNDC(";
    s<<"const in float["<<N_COEFICIENTS<<"] coefficients,";
    s<<"const in ViewportParams screen_params,const in ViewportParams texture_params,const in vec2 in_ndc,const in float maxRadSq){\n";
    s<<"vec2 distorted_ndc_tanangle=vec2(";
    s<<"in_ndc.x * texture_params.width - texture_params.x_eye_offset,";
    s<<"in_ndc.y * texture_params.height - texture_params.y_eye_offset);";
    s<<"vec2 undistorted_ndc_tanangle = PolynomialDistort(coefficients,distorted_ndc_tanangle,maxRadSq);";
    s<<"vec2 ret=vec2(undistorted_ndc_tanangle.x*screen_params.width+screen_params.x_eye_offset,";
    s<<"undistorted_ndc_tanangle.y*screen_params.height+screen_params.y_eye_offset);";
    s<<"return ret;";
    s<<"}\n";

    //The uniforms needed for vddc
    if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        s<<"uniform float uMaxRadSq;";
        s<<"uniform float uKN["<<N_COEFICIENTS<<"];";
    }else if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        s<<"uniform float uMaxRadSq;";
        s<<"uniform float uKN["<<N_COEFICIENTS<<"];";
        s<<"uniform ViewportParams uScreenParams;";
        s<<"uniform ViewportParams uTextureParams;";
    }
    return s.str();
}

std::string DistortionManager::writeGLPosition(const DistortionManager &distortionManager,
                                                     const std::string &positionAttribute) {
    std::stringstream s;
    if(distortionManager.distortionMode==NONE){
        s<<"gl_Position = (uPMatrix*uMVMatrix)* "<<positionAttribute<<";\n";
    }else if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_VIEW_SPACE){
        s<<"vec4 pos=uMVMatrix*"+positionAttribute+";\n";
        s<<"float r2=dot(pos.xy,pos.xy)/(pos.z*pos.z);\n";
        //s<<"r2=clamp(r2,0.0,uMaxRadSq);\n";
        s<<"float dist_factor=PolynomialDistortionFactor(r2,uKN);";
        s<<"pos.xy*=dist_factor;\n";
        //s<<"gl_Position=pos;\n";
        s<<"gl_Position=uPMatrix*pos;\n";
        //s<<"gl_Position.x=gl_Position.x*uScreenParams.width+uScreenParams.x_eye_offset;";
        //s<<"gl_Position.y=gl_Position.y*uScreenParams.height+uScreenParams.y_eye_offset;";
    }else if(distortionManager.distortionMode==DISTORTION_MODE::RADIAL_CARDBOARD){
        s<<"vec4 pos_view=uMVMatrix*"+positionAttribute+";\n";
        s<<"vec4 pos_clip=uPMatrix*pos_view;\n";
        s<<"vec3 ndc=pos_clip.xyz/pos_clip.w;";
        s<<"vec2 dist_p=UndistortedNDCForDistortedNDC(uKN,uScreenParams,uTextureParams,ndc.xy,uMaxRadSq);";
        s<<"gl_Position=vec4(dist_p*pos_clip.w,pos_clip.z,pos_clip.w);";
        //s<<"gl_Position=vec4(dist_p,0,1);";

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

void DistortionManager::updateDistortion(const MPolynomialRadialDistortion &inverseDistortion,
                                         float maxRadSq) {
    for(int i=0;i<inverseDistortion.getCoeficients().size();i++){
        radialDistortionCoefficients.kN[i]=inverseDistortion.getCoeficients()[i];
    }
    radialDistortionCoefficients.maxRadSquared=maxRadSq;
}

void DistortionManager::updateDistortion(const MPolynomialRadialDistortion &inverseDistortion,
                                    float maxRadSq,
                                    const std::array<MLensDistortion::ViewportParams, 2> screen_params,
                                    const std::array<MLensDistortion::ViewportParams, 2> texture_params) {
    this->screen_params=screen_params;
    this->texture_params=texture_params;
    updateDistortion(inverseDistortion,maxRadSq);
}

void DistortionManager::updateDistortionWithIdentity() {
    std::vector<float> identity;
    for(int i=0;i<DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS;i++){
        identity.push_back(0.0F);
    }
    MPolynomialRadialDistortion distortion(identity);
    const MLensDistortion::ViewportParams identityParams{1,1,0,0};
    updateDistortion(distortion,1000,{identityParams,identityParams},{identityParams,identityParams});
}


