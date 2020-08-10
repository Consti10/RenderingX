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
#include <FPSCalculator.h>
#include <BasicGLPrograms.hpp>
#include <gvr_util/util.h>
#include <GLBuffer.hpp>
#include <ProjTex/GLPTextureProj.h>
#include <VrCompositorRenderer.h>

// Example that compares the distortion created by VDDC and google cardboard with Gvr (google vr)
// Renders the wireframe of a 2D rectangular surface

class RendererDistortion{
public:
    //Since blending is enabled, when selecting both rendering modes simultaneously the visual difference between them
    //can be observed when rendering the 2D Mesh (the mesh is rendered with a smaller line width the second time)
    const bool RENDER_SCENE_USING_GVR_RENDERBUFFER= true;
    const bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT=true;
public:
    RendererDistortion(JNIEnv* env, jobject androidContext, gvr_context *gvr_context);
    void onSurfaceCreated(JNIEnv* env,jobject context);
    void onDrawFrame();
private:
    /*
    * draws the grid of a 2D surface into gvr renderbuffer which is then distorted into framebuffer by the gvr api
    */
    void drawEyeGvrRenderbuffer(gvr::Eye eye);
    /*
     * draw into framebuffer applying undistortion to the vertices
     * No intermediate renderbuffer needed
     */
    void drawEyeVDDC(gvr::Eye eye);
private:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    gvr::BufferViewportList buffer_viewports;
    gvr::BufferViewportList recommended_buffer_viewports;
    gvr::BufferViewport scratch_viewport;
    std::unique_ptr<gvr::SwapChain> swap_chain;
    gvr::Sizei framebuffer_size;
    void updateBufferViewports();
    FPSCalculator mFPSCalculator;
    std::unique_ptr<GLProgramVC> mGLProgramVC=nullptr;
    std::unique_ptr<GLProgramTexture> mGLProgramTexture=nullptr;
    static constexpr int LINE_MESH_TESSELATION_FACTOR=12;
    static constexpr float LINE_WIDTH_BIG=8.0f;
    static constexpr float LINE_WIDTH_SMALL=4.0f;
    GLProgramTexture::TexturedGLMeshBuffer mTestMesh2DWireframe;
public:
    VrCompositorRenderer vrCompositorRenderer;
    GLuint mGreenTexture;
    GLuint mBlueTexture;
};

#endif //RENDERINGX_EXAMPLE_DISTORTION_H
