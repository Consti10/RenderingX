//
// Created by Consti10 on 08/10/2019.
//

#ifndef RENDERINGX_EXAMPLE_DISTORTION_H
#define RENDERINGX_EXAMPLE_DISTORTION_H

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <GLProgramVC.h>
#include <GLProgramText.h>
#include <GLProgramTexture.h>
#include <ColoredGeometry.hpp>
#include <TexturedGeometry.hpp>
#include <TextAssetsHelper.hpp>

#include <GLProgramLine.h>
#include <Chronometer.h>
#include <FPSCalculator.h>
#include <Helper/BasicGLPrograms.hpp>
#include <DistortionCorrection/VRHeadsetParams.h>
#include <gvr_util/util.h>
#include <Helper/GLBufferHelper.hpp>

class ExampleRenderer{
public:
   ExampleRenderer(JNIEnv* env,jobject androidContext,gvr_context *gvr_context);
    void onSurfaceCreated(JNIEnv* env,jobject context);
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
private:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    gvr::BufferViewportList buffer_viewports;
    gvr::BufferViewportList recommended_buffer_viewports;
    gvr::BufferViewport scratch_viewport;
    std::unique_ptr<gvr::SwapChain> swap_chain;
    gvr::Sizei framebuffer_size;
    void updateBufferViewports();
    FPSCalculator mFPSCalculator;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<GLProgramTexture> mGLProgramTexture=nullptr;
    GLuint mTextureEquirectangularImage;
    GLBufferHelper::VertexIndexBuffer mEquirecangularSphereB;
    void drawEyeGvr(gvr::Eye eye);
    void drawEyeVDDC(gvr::Eye eye);
private:
    static constexpr int LINE_MESH_TESSELATION_FACTOR=10;
    GLuint glBufferVC;
    GLuint glBufferVCGreen;
    int nColoredVertices;
private:
    DistortionManager distortionManager;
public:
    VRHeadsetParams vrHeadsetParams;
};

#endif //RENDERINGX_EXAMPLE_DISTORTION_H
