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
    //Since blending is enabled, when selecting both rendering modes simultaneously the visual difference between them
    //can be observed when only rendering the 2D Mesh (the mesh is rendered with a smaller line width the second time)
    //When rendering the equirectangular sphere you should only use one of the two rendering methods since the first one will be overwritten
    const bool RENDER_SCENE_USING_GVR_RENDERBUFFER;
    const bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT;
    const bool ENABLE_SCENE_MESH_2D;
    const bool ENABLE_SCENE_360_SPHERE;
    const bool ENABLE_SCENE_360_SPHERE_EQUIRECTANGULAR;
public:
    ExampleRenderer(JNIEnv* env,jobject androidContext,gvr_context *gvr_context,
            bool RENDER_SCENE_USING_GVR_RENDERBUFFER=true,bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT=true,
            bool ENABLE_SCENE_MESH_2D=true,bool ENABLE_SCENE_360_SPHERE=false,bool ENABLE_SCENE_360_SPHERE_EQUIRECTANGULAR=false);
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
    std::unique_ptr<GLProgramVC> mGLProgramVC2D;
    GLuint mTexture360Image;
    GLuint mTexture360ImageEquirectangular;
    GLBufferHelper::VertexIndexBuffer mEquirecangularSphereB;
    GLBufferHelper::VertexBuffer mGvrSphereB;
    GLBufferHelper::VertexBuffer mOcclusionMesh[2];
    /*
     * draws into gvr renderbuffer which is then distorted into framebuffer
     */
    void drawEyeGvrRenderbuffer(gvr::Eye eye);
    /*
     * draw into framebuffer applying undistortion to the vertices
     * No intermediate renderbuffer needed
     */
    void drawEyeVDDC(gvr::Eye eye);
    /*
     * draw everything into what's currently bound (renderbuffer for gvr,framebuffer for vddc)
     */
    void drawEye(gvr::Eye eye,glm::mat4 viewM,glm::mat4 projM,bool meshColorGreen,bool vignette=false);
private:
    static constexpr int LINE_MESH_TESSELATION_FACTOR=10;
    GLBufferHelper::VertexBuffer blueMeshB;
    GLBufferHelper::VertexBuffer greenMeshB;
private:
    DistortionManager distortionManager;
public:
    VRHeadsetParams vrHeadsetParams;
};

#endif //RENDERINGX_EXAMPLE_DISTORTION_H
