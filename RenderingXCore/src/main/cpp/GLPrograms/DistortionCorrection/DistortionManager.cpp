//
// Created by Consti10 on 31/10/2019.
//

#include "DistortionManager.h"

DistortionManager::DistortionManager(gvr_context *gvrContext) {
    const Distortion mDistortionLeftEye(200,gvrContext,GVR_LEFT_EYE);
    const Distortion mDistortionRightEye(200,gvrContext,GVR_RIGHT_EYE);
    const Distortion inverseLeftEye=mDistortionLeftEye.calculateInverse(RESOLUTION_XY-1);
    const Distortion inverseRightEye=mDistortionRightEye.calculateInverse(RESOLUTION_XY-1);

    inverseLeftEye.extractData(leftEyeUndistortionData);
    inverseRightEye.extractData(rightEyeUndistortionData);

    //coefficients: [0.34, 0.55]
    radialDistortionCoefficients.maxRadSquared=10.0f;
    radialDistortionCoefficients.kN[0]=0.34f;
    radialDistortionCoefficients.kN[1]=0.55f;
}

DistortionManager::DistortionManager(JNIEnv *env, jfloatArray undistData) {
    jfloat *arrayP=env->GetFloatArrayElements(undistData, nullptr);
    radialDistortionCoefficients.maxRadSquared=arrayP[0];
    std::memcpy(radialDistortionCoefficients.kN.data(),&arrayP[1],radialDistortionCoefficients.kN.size()*sizeof(float));
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
        const DistortionManager::UndistortionHandles& undistortionHandles) const {
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
    GLuint *texture=&(leftEye ? mDistortionCorrectionTextureLeftEye : mDistortionCorrectionTextureRightEye);
    glGenTextures(1,texture);
    glBindTexture(GL_TEXTURE_2D,*texture);

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

std::string DistortionManager::writeGLPositionWithDistortion(const DistortionManager &distortionManager,
                                                 const std::string &positionAttribute) {
    std::stringstream s;
    if(distortionManager.MY_VERSION==0){
        s<<"vec4 pos=uMVMatrix*"+positionAttribute+";\n";
        s<<"float r2=dot(pos.xy,pos.xy)/(pos.z*pos.z);\n";
        s<<"r2=clamp(r2,0.0,_MaxRadSq);\n";
        s<<"float ret = 0.0;\n";
        s<<"ret = r2 * (ret + _Undistortion2.y);\n";
        s<<"ret = r2 * (ret + _Undistortion2.x);\n";
        s<<"ret = r2 * (ret + _Undistortion.w);\n";
        s<<"ret = r2 * (ret + _Undistortion.z);\n";
        s<<"ret = r2 * (ret + _Undistortion.y);\n";
        s<<"ret = r2 * (ret + _Undistortion.x);\n";
        s<<"pos.xy*=1.0+ret;\n";
        s<<"gl_Position=pos;\n";
    }else if(distortionManager.MY_VERSION==1){
        s<<std::fixed;
        //s<<"vec4 pos=vec4("+positionAttribute+".xy*2.0, 0, 1);";
        s<<"vec4  pos=(uPMatrix*uMVMatrix)*"+positionAttribute+";\n";
        s<<"vec2 ndc=pos.xy/pos.w;";///(-pos.z);";
        s<<"int idx1=my_round(ndc.x*"<<(DistortionManager::RESOLUTION_XY/2.0f)<<")+"<<DistortionManager::RESOLUTION_XY/2<<";";
        s<<"int idx2=my_round(ndc.y*"<<(DistortionManager::RESOLUTION_XY/2.0f)<<")+"<<DistortionManager::RESOLUTION_XY/2<<";";
        s<<"idx1=my_clamp(idx1,0,"<<DistortionManager::RESOLUTION_XY-1<<");";
        s<<"idx2=my_clamp(idx2,0,"<<DistortionManager::RESOLUTION_XY-1<<");";
        //We do not have support for 2-dimensional arrays- simple pointer arithmetic ( array[i][j]==array[i*n+j] )
        s<<"int idx=idx1*"<<DistortionManager::RESOLUTION_XY<<"+idx2;";
        s<<"pos.x+=LOL[idx].x;";
        s<<"pos.y+=LOL[idx].y;";
        s<<"gl_Position=pos;\n";
    }else{
        s<<"vec4  pos=(uPMatrix*uMVMatrix)*"+positionAttribute+";\n";
        //s<<"vec4 pos=vec4("+positionAttribute+".xy*2.0, 0, 1);";
        s<<"vec2 ndc=pos.xy/pos.w;";///(-pos.z);";
        s<<"vec2 uvFromNDC=(ndc+vec2(1.0,1.0))/2.0;";
        s<<"vec4 value=texture2D(sTextureDistCorrection,uvFromNDC );";
        s<<"pos.x+=value.x*pos.w;";
        s<<"pos.y+=value.y*pos.w;";
        s<<"gl_Position=pos;\n";
    }
    return s.str();
}

std::string DistortionManager::writeDistortionParams(
        const DistortionManager *distortionManager) {
    std::stringstream s;
    if(distortionManager==nullptr)return "";
    if(distortionManager->MY_VERSION==0){
        const auto coeficients=distortionManager->radialDistortionCoefficients.kN;
        s<<std::fixed;
        s<<"const float _MaxRadSq="<<distortionManager->radialDistortionCoefficients.maxRadSquared<<";\n";
        //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
        s<<"const vec4 _Undistortion=vec4("<<coeficients[0]<<","<<coeficients[1]<<","<<coeficients[2]<<","<<coeficients[3]<<");\n";
        s<<"const vec2 _Undistortion2=vec2("<<coeficients[4]<<","<<coeficients[5]<<");\n";
    }else if(distortionManager->MY_VERSION==1){
        s<<"uniform highp vec2 LOL["<< DistortionManager::ARRAY_SIZE<<"];";
        s<<"int my_clamp(in int x,in int minVal,in int maxVal){";
        s<<"if(x<minVal){return minVal;}";
        s<<"if(x>maxVal){return maxVal;}";
        s<<"return x;}";
        s<<"int my_round(in float x){";
        s<<"float afterCome=x-float(int(x));";
        s<<"if(afterCome>=0.5){return int(x+0.5);}";
        s<<"return int(x);}";
    }else if(distortionManager->MY_VERSION==2){
        s<<"uniform sampler2D sTextureDistCorrection;";
    }
    return s.str();
}

std::string DistortionManager::writeGLPosition(const DistortionManager *distortionManager,
                                                     const std::string &positionAttribute) {
    if(distortionManager!= nullptr)return writeGLPositionWithDistortion(*distortionManager,positionAttribute);
    //return"vec4 lul=uMVMatrix * "+positionAttribute+";\n"+"";
    return "gl_Position = (uPMatrix*uMVMatrix)* "+positionAttribute+";\n";
    //return "gl_Position = vec4("+positionAttribute+".xy*2.0, 0, 1);";
}

std::string DistortionManager::createDistortionFilesIfNotYetExisting(
        const std::string &distortionFilesDirectory, const std::string &viewerModel,
        gvr_context *gvrContext) {
    const std::string directory=distortionFilesDirectory+viewerModel+"/";

    int result=mkdir(directory.c_str(), 0777);

    if(!(FileHelper::fileExists(directory+std::string("dist_left.bin"))||
         FileHelper::fileExists(directory+std::string("dist_right.bin")))){
        //TODO
    }
    if(result==0){
        const Distortion distortionLeftEye(200,gvrContext,GVR_LEFT_EYE);
        const Distortion inverseLeftEye=distortionLeftEye.calculateInverse(RESOLUTION_XY-1);
        const Distortion distortionRightEye(200,gvrContext,GVR_RIGHT_EYE);
        const Distortion inverseRightEye=distortionRightEye.calculateInverse(RESOLUTION_XY-1);

        inverseLeftEye.saveAsBinary(directory,"dist_left.bin");
        inverseRightEye.saveAsBinary(directory,"dist_right.bin");
    }
    return directory;
}

DistortionManager *
DistortionManager::createFromFileIfAlreadyExisting(const std::string &externalStorageDirectory,
                                                   gvr_context *gvrContext) {
    //MDebug::log("Curr selected device is,Model:"+std::string(gvr_api_->GetViewerModel())+" Vendor:"+std::string(gvr_api_->GetViewerVendor()),TAG);
    FileHelper::createRenderingXCoreDistortionDirectoryIfNotAlreadyExisting(externalStorageDirectory);
    const std::string distortionDirectory=FileHelper::renderingXCoreDistortionDirectory(externalStorageDirectory);

    const std::string model=std::string(gvr_get_viewer_model(gvrContext));
    //const std::string vendor=std::string(gvr_get_viewer_vendor(gvrContext));
    const std::string distortionDirectoryForModel=DistortionManager::createDistortionFilesIfNotYetExisting(distortionDirectory,model,gvrContext);
    return new DistortionManager(distortionDirectoryForModel+std::string("dist_left.bin"),distortionDirectoryForModel+std::string("dist_right.bin"));
}
