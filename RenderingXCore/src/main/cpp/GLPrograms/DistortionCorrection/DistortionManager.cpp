//
// Created by Consti10 on 31/10/2019.
//

#include "DistortionManager.h"

DistortionManager::DistortionManager(gvr_context *gvrContext) {
    Distortion mDistortionLeftEye(200,gvrContext,GVR_LEFT_EYE);
    Distortion mDistortionRightEye(200,gvrContext,GVR_RIGHT_EYE);
    Distortion inverseLeftEye=mDistortionLeftEye.calculateInverse(RESOLUTION_XY-1);
    Distortion inverseRightEye=mDistortionRightEye.calculateInverse(RESOLUTION_XY-1);


    inverseLeftEye.extractData(leftEyeUndistortionData);
    inverseRightEye.extractData(rightEyeUndistortionData);

    //coefficients: [0.34, 0.55]
    RadialUndistortionData[0]=10.0f;
    RadialUndistortionData[1]=0.34f;
    RadialUndistortionData[2]=0.55f;
}

DistortionManager::DistortionManager(JNIEnv *env, jfloatArray undistData) {
    jfloat *arrayP=env->GetFloatArrayElements(undistData, nullptr);
    std::memcpy(RadialUndistortionData.data(),arrayP,RadialUndistortionData.size()*sizeof(float));
    env->ReleaseFloatArrayElements(undistData,arrayP,0);
}

DistortionManager::DistortionManager(const std::string &filenameLeftEye,const std::string &filenameRightEye) {
    Distortion inverseLeftEye=Distortion::createFromBinaryFile(filenameLeftEye);
    Distortion inverseRightEye=Distortion::createFromBinaryFile(filenameRightEye);
    inverseLeftEye.extractData(leftEyeUndistortionData);
    inverseRightEye.extractData(rightEyeUndistortionData);
}


DistortionManager::UndistortionHandles
DistortionManager::getUndistortionUniformHandles(const GLuint program) const {
    UndistortionHandles ret{};
    if(MY_VERSION==1)ret.lolHandle=(GLuint)glGetUniformLocation(program,"LOL");
    if(MY_VERSION==2)ret.samplerDistCorrectionHandle=(GLuint)glGetUniformLocation (program, "sTextureDistCorrection");
    return ret;
}

void DistortionManager::beforeDraw(
        const DistortionManager::UndistortionHandles undistortionHandles) const {
    if(MY_VERSION==0){
        //Nothing
    }else if(MY_VERSION==1){
        glUniform2fv(undistortionHandles.lolHandle,(GLsizei)(ARRAY_SIZE),(GLfloat*)(leftEye ? leftEyeUndistortionData : rightEyeUndistortionData));
    }else{
        auto texture=leftEye ? mDistortionCorrectionTextureLeftEye : mDistortionCorrectionTextureRightEye;
        GLenum sampler=leftEye ? MY_SAMPLER_UNIT_LEFT_EYE : MY_SAMPLER_UNIT_RIGHT_EYE;
        GLenum texUnit=leftEye ? MY_TEXTURE_UNIT_LEFT_EYE : MY_TEXTURE_UNIT_RIGHT_EYE;
        glActiveTexture(texUnit);
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glUniform1i(undistortionHandles.samplerDistCorrectionHandle,sampler);
    }
}

void DistortionManager::afterDraw() const {
    glBindTexture(GL_TEXTURE_2D,0);
}

void DistortionManager::generateTexture(bool leftEye) {
    glGenTextures(1,&(leftEye ? mDistortionCorrectionTextureLeftEye : mDistortionCorrectionTextureRightEye));
    glBindTexture(GL_TEXTURE_2D,(leftEye ? mDistortionCorrectionTextureLeftEye : mDistortionCorrectionTextureRightEye));

    const auto& lol=leftEye ? leftEyeUndistortionData : rightEyeUndistortionData;
    const int SIZE=DistortionManager::RESOLUTION_XY;
    GLfloat data[SIZE][SIZE][4];
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            data[i][j][0]=lol[j][i][0];
            data[i][j][1]=lol[j][i][1];
            //data[i][j][0]=0.0f;
            //data[i][j][1]=0.2f;
        }
    }
    //GL_RGBA32F
    constexpr auto RGBA32F_ARB=0x8814;
    constexpr auto RGBA16F_ARB=0x881A;
    glTexImage2D(GL_TEXTURE_2D, 0,RGBA16F_ARB, SIZE,SIZE, 0, GL_RGBA, GL_FLOAT,data);

    glBindTexture(GL_TEXTURE_2D,0);
}

void DistortionManager::generateTextures() {
    generateTexture(true);
    generateTexture(false);
}

