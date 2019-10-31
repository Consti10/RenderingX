//
// Created by Consti10 on 31/10/2019.
//

#include "DistortionManager.h"

DistortionManager::DistortionManager(gvr_context *gvrContext) {
    Distortion mDistortion(200,gvrContext);
    //Distortion inverse=mDistortion.calculateInverse(RESOLUTION_XY);
    //inverse.saveAsBinary("/storage/emulated/0/Documents/RenderingX/","myfile3.bin");
    Distortion inverse=Distortion::createFromBinaryFile("/storage/emulated/0/Documents/RenderingX/myfile3.bin");

    //mDistortion.radialDistortionOnly();
    //Distortion inverse=mDistortion.calculateInverse(RESOLUTION_XY);
    //inverse.radialDistortionOnly();
    inverse.lol(lol);
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
        glUniform2fv(undistortionHandles.lolHandle,(GLsizei)(ARRAY_SIZE),(GLfloat*)lol);
    }else{
        glActiveTexture(DistortionManager::MY_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D,mDistortionCorrectionTexture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glUniform1i(undistortionHandles.samplerDistCorrectionHandle,DistortionManager::MY_SAMPLER_UNIT);
    }
}

void DistortionManager::afterDraw() const {
    glBindTexture(GL_TEXTURE_2D,0);
}

void DistortionManager::generateTexture() {
    glGenTextures(1,&mDistortionCorrectionTexture);

    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, mDistortionCorrectionTexture);

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

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    //NDKHelper::uploadAssetImageToGPU(env,androidContext,name,false);

    glBindTexture(GL_TEXTURE_2D,0);
}
