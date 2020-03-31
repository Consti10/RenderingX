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
#include <BasicGLPrograms.hpp>
#include <VRHeadsetParams.h>
#include <gvr_util/util.h>
#include <GLBufferHelper.hpp>

//Example that uses VDDC to render VR content

class ExampleRendererVR{
public:
    //Since blending is enabled, when selecting both rendering modes simultaneously the visual difference between them
    //can be observed when only rendering the 2D Mesh (the mesh is rendered with a smaller line width the second time)
    //When rendering any 360 sphere you should only use one of the two rendering methods since the first one will be overwritten
    const bool RENDER_SCENE_USING_GVR_RENDERBUFFER;
    const bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT;
    const bool ENABLE_SCENE_MESH_2D;
    enum SPHERE_MODE{SPHERE_MODE_NONE,SPHERE_MODE_EQUIRECTANGULAR_TEST,SPHERE_MODE_INSTA360_TEST1,SPHERE_MODE_INSTA360_TEST2};
    const SPHERE_MODE M_SPHERE_MODE;
public:
    ExampleRendererVR(JNIEnv* env,jobject androidContext,gvr_context *gvr_context,
            bool RENDER_SCENE_USING_GVR_RENDERBUFFER=true,bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT=true,
            bool ENABLE_SCENE_MESH_2D=true,const int SPHERE_MODE=0);
    void onSurfaceCreated(JNIEnv* env,jobject context,int videoTexture);
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
private:
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
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    gvr::BufferViewportList buffer_viewports;
    gvr::BufferViewportList recommended_buffer_viewports;
    gvr::BufferViewport scratch_viewport;
    std::unique_ptr<gvr::SwapChain> swap_chain;
    gvr::Sizei framebuffer_size;
    void updateBufferViewports();
    FPSCalculator mFPSCalculator;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt=nullptr;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt2=nullptr;
    //GLuint mTexture360Image;
    //GLuint mTexture360ImageInsta360;
    GLuint mVideoTexture;
    VertexBuffer mGvrSphereMappedB;
    VertexBuffer mGvrSphereB;
    VertexIndexBuffer mSphereDualFisheye2;
    std::array<VertexBuffer,2> mOcclusionMesh;
    static constexpr int LINE_MESH_TESSELATION_FACTOR=12;
    VertexBuffer blueMeshB;
    VertexBuffer greenMeshB;
    DistortionManager distortionManager;
public:
    VRHeadsetParams vrHeadsetParams;
};

#endif //RENDERINGX_EXAMPLE_DISTORTION_H
