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

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Helper/NDKHelper.h>
#include "android/log.h"
#include "DistortionCorrection/Distortion.h"
#include "FileHelper.h"

class DistortionManager {
public:
    static constexpr const int N_RADIAL_UNDISTORTION_COEFICIENTS=7;
    struct RadialDistortionCoefficients{
        float maxRadSquared;
        std::array<float,N_RADIAL_UNDISTORTION_COEFICIENTS-1> kN;
    };
    RadialDistortionCoefficients radialDistortionCoefficients;

    static constexpr const int RESOLUTION_XY=32;
    static constexpr int ARRAY_SIZE=RESOLUTION_XY*RESOLUTION_XY;

    float leftEyeUndistortionData[RESOLUTION_XY][RESOLUTION_XY][2];
    float rightEyeUndistortionData[RESOLUTION_XY][RESOLUTION_XY][2];

    struct UndistortionHandles{
        GLuint lolHandle;
        GLuint samplerDistCorrectionHandle;
    };
    const int MY_VERSION=2;

    bool leftEye=true;
    static constexpr auto MY_TEXTURE_UNIT_LEFT_EYE=GL_TEXTURE2;
    static constexpr auto MY_SAMPLER_UNIT_LEFT_EYE=2;
    static constexpr auto MY_TEXTURE_UNIT_RIGHT_EYE=GL_TEXTURE3;
    static constexpr auto MY_SAMPLER_UNIT_RIGHT_EYE=3;

    GLuint mDistortionCorrectionTextureLeftEye;
    GLuint mDistortionCorrectionTextureRightEye;
public:
    DistortionManager(gvr_context* gvrContext);
    DistortionManager(JNIEnv *env,jfloatArray undistData);
    DistortionManager(const std::string& filenameLeftEye,const std::string& filenameRightEye);

    UndistortionHandles getUndistortionUniformHandles(const GLuint program)const;
    void beforeDraw(const UndistortionHandles& undistortionHandles)const;
    void afterDraw()const;

    void generateTextures();
    void generateTexture(bool leftEye);

    static std::string writeGLPosition(const DistortionManager* distortionManager,const std::string &positionAttribute="aPosition");
    static std::string writeGLPositionWithDistortion(const DistortionManager &distortionManager, const std::string &positionAttribute);

    static std::string writeDistortionParams(const DistortionManager *distortionManager);

    static std::string createDistortionFilesIfNotYetExisting(const std::string& distortionFilesDirectory,const std::string& viewerModel,gvr_context* gvrContext);

    static DistortionManager* createFromFileIfAlreadyExisting(const std::string& externalStorageDirectory,gvr_context* gvrContext);
};


#endif //RENDERINGX_DISTORTIONMANAGER_H
