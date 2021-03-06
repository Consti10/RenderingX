//
// Created by Consti10 on 15/05/2019.
//
#include "RendererDistortion.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include <GLBuffer.hpp>
#include <MatrixHelper.h>
#include <Sphere/DualFisheyeSphere.hpp>
#include <Sphere/UvSphere.hpp>
#include <Sphere/SphereBuilder.hpp>
#include <Extensions.h>

constexpr auto TAG="DistortionExample";

RendererDistortion::RendererDistortion(JNIEnv *env, jobject androidContext, gvr_context *gvr_context):
        vrSettings(env,androidContext),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
        vrCompositorRenderer(env,androidContext,gvr_api_.get(),true,true),
        mFPSCalculator("OpenGL FPS",std::chrono::seconds(2)){
    buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    recommended_buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    scratch_viewport = gvr_api_->CreateBufferViewport();
}

void RendererDistortion::onSurfaceCreated(JNIEnv *env, jobject context) {
    vrCompositorRenderer.initializeGL();
    gvr_api_->InitializeGl();
    std::vector<gvr::BufferSpec> specs;
    specs.push_back(gvr_api_->CreateBufferSpec());
    framebuffer_size = gvr_api_->GetMaximumEffectiveRenderTargetSize();
    //MLOGD<<"GVR MaximumEffectiveRenderTargetSize"<<framebuffer_size.width<<" "<<framebuffer_size.height;
    specs[0].SetSize(framebuffer_size);
    specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    swap_chain = std::make_unique<gvr::SwapChain>(gvr_api_->CreateSwapChain(specs));
    //MLOGD<<"N buff in swap chain"<<swap_chain->GetBufferCount(); //returns 1 ?!

    mGLProgramVC=std::make_unique<GLProgramVC>();
    mGLProgramTexture=std::make_unique<GLProgramTexture>(false);
    //create the green and blue mesh
    float tesselatedRectSize=2.0; //6.2f
    const auto tmpData=TexturedGeometry::makeTessellatedRectWireframe(LINE_MESH_TESSELATION_FACTOR, {0, 0, -2}, {tesselatedRectSize, tesselatedRectSize});
    mTestMesh2DWireframe.setData(tmpData);
    glGenTextures(1,&mBlueTexture);
    //glGenTextures(1,&mGreenTexture);
    GLProgramTexture::loadTexture(mBlueTexture,env,context,"ExampleTexture/blue.png");
    vrRenderBufferWithGreenTexture.loadDefaultTexture(env,context);
    vrCompositorRenderer.removeLayers();
    const auto headTracking=vrSettings.isHeadTrackingEnabled() ? VrCompositorRenderer::HEAD_TRACKING::FULL : VrCompositorRenderer::HEAD_TRACKING::NONE;

    vrCompositorRenderer.addLayer(tmpData,&vrRenderBufferWithGreenTexture,headTracking);
    GLHelper::checkGlError("example_renderer::onSurfaceCreated");
}

void RendererDistortion::updateBufferViewports() {
    recommended_buffer_viewports.SetToRecommendedBufferViewports();
    for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        buffer_viewports.SetBufferViewport(eye,scratch_viewport);
    }
}

void RendererDistortion::onDrawFrame() {
    mFPSCalculator.tick();
    //LOGD("FPS: %f",mFPSCalculator.getCurrentFPS());

    //Update the head position (rotation) then leave it untouched during the frame
    vrCompositorRenderer.updateLatestHeadSpaceFromStartSpaceRotation();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(RENDER_SCENE_USING_GVR_RENDERBUFFER){
        updateBufferViewports();
        gvr::Frame frame = swap_chain->AcquireFrame();
        frame.BindBuffer(0); //0 is the 0 from createSwapChain()
        glClearColor(0.25f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        for(uint32_t eye=0;eye<2;eye++){
            drawEyeGvrRenderbuffer(static_cast<gvr::Eye>(eye));
        }
        frame.Unbind();
        frame.Submit(buffer_viewports, toGVR(vrCompositorRenderer.GetLatestHeadSpaceFromStartSpaceRotation()));
    }
    if(RENDER_SCENE_USING_VERTEX_DISPLACEMENT){
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        for(int eye=0;eye<2;eye++){
            drawEyeVDDC(static_cast<gvr::Eye>(eye));
        }
    }
    GLHelper::checkGlError("RendererDistortion::onDrawFrame");
}

void RendererDistortion::drawEyeGvrRenderbuffer(gvr::Eye eye) {
    buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
    const gvr::Rectf& rect = scratch_viewport.GetSourceUv();
    int left = static_cast<int>(rect.left * framebuffer_size.width);
    int bottom = static_cast<int>(rect.bottom * framebuffer_size.width);
    int width = static_cast<int>((rect.right - rect.left) * framebuffer_size.width);
    int height = static_cast<int>((rect.top - rect.bottom) * framebuffer_size.height);
    glViewport(left, bottom, width, height);
    const gvr_rectf fov = scratch_viewport.GetSourceFov();
    const gvr::Mat4f perspective =ndk_hello_vr::PerspectiveMatrixFromView(fov, VrCompositorRenderer::MIN_Z_DISTANCE,VrCompositorRenderer::MAX_Z_DISTANCE);
    const auto eyeM=gvr_api_->GetEyeFromHeadMatrix(eye==0 ? GVR_LEFT_EYE : GVR_RIGHT_EYE);
    //const auto rotM=gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    const auto rotM=toGVR(vrCompositorRenderer.GetLatestHeadSpaceFromStartSpaceRotation());
    glm::mat4 viewM;
    if(vrSettings.isHeadTrackingEnabled()){
        viewM=toGLM(ndk_hello_vr::MatrixMul(eyeM,rotM));
    }else{
        viewM=toGLM(ndk_hello_vr::MatrixMul(eyeM,toGVR(glm::mat4(1.0f))));
    }
    const auto projectionM=toGLM(perspective);
    glLineWidth(LINE_WIDTH_BIG);
    // draw debug mesh:
    mGLProgramTexture->drawX(mBlueTexture, viewM, projectionM, mTestMesh2DWireframe);
    GLHelper::checkGlError("RendererDistortion::drawEyeGvrRenderbuffer");
}

void RendererDistortion::drawEyeVDDC(gvr::Eye eye) {
    glLineWidth(LINE_WIDTH_SMALL);
    vrCompositorRenderer.drawLayers(eye);
    GLHelper::checkGlError("RendererDistortion::drawEyeVDDC");
}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_stereo_distortion_RendererDistortion_##method_name

inline jlong jptr(RendererDistortion *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline RendererDistortion *native(jlong ptr) {
    return reinterpret_cast<RendererDistortion*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong native_gvr_api) {
    return jptr(new RendererDistortion(env, androidContext, reinterpret_cast<gvr_context *>(native_gvr_api)));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
    delete native(p);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong p,jobject androidContext) {
    native(p)->onSurfaceCreated(env,androidContext);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onDrawFrame();
}

}
