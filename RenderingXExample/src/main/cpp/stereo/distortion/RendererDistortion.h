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
#include <DistortionEngine.h>
#include <gvr_util/util.h>
#include <GLBuffer.hpp>
#include <ProjTex/GLPTextureProj.h>
#include <VrCompositorRenderer.h>

// Example that compares the distortion created by VDDC and google cardboard with Gvr (google vr)
// Renders a 2D Mesh for testing

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
    * draws into gvr renderbuffer which is then distorted into framebuffer
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
    static constexpr int LINE_MESH_TESSELATION_FACTOR=12;
    GLProgramVC::ColoredMesh blueMeshB;
    GLProgramVC::ColoredMesh greenMeshB;
public:
    VrCompositorRenderer vrCompositorRenderer;
private:
    /*GLPTextureProj* glProgramTextureProj;
    GLProgramTexture* glProgramTexture;
    GLuint mTextureMonaLisa;
    VertexBuffer glBufferTextured;*/
};

#endif //RENDERINGX_EXAMPLE_DISTORTION_H
