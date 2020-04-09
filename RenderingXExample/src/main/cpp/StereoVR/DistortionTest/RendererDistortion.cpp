//
// Created by Consti10 on 15/05/2019.
//
#include "RendererDistortion.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include <GLBufferHelper.hpp>
#include <MatrixHelper.h>
#include <array>
#include <Sphere/DualFisheyeSphere.hpp>
#include <Sphere/UvSphere.hpp>
#include <CardboardViewportOcclusion.hpp>
#include <Sphere/SphereBuilder.hpp>

constexpr auto TAG="DistortionExample";

RendererDistortion::RendererDistortion(JNIEnv *env, jobject androidContext, gvr_context *gvr_context):
        distortionManager(VDDCManager::RADIAL_CARDBOARD), mFPSCalculator("OpenGL FPS", 2000){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
    buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    recommended_buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    scratch_viewport = gvr_api_->CreateBufferViewport();
}


void RendererDistortion::onSurfaceCreated(JNIEnv *env, jobject context) {
//Instantiate all our OpenGL rendering 'Programs'
    gvr_api_->InitializeGl();
    std::vector<gvr::BufferSpec> specs;
    specs.push_back(gvr_api_->CreateBufferSpec());
    framebuffer_size = gvr_api_->GetMaximumEffectiveRenderTargetSize();
    specs[0].SetSize(framebuffer_size);
    specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_16);
    swap_chain = std::make_unique<gvr::SwapChain>(gvr_api_->CreateSwapChain(specs));

    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(&distortionManager);
    mBasicGLPrograms->text.loadTextRenderingData(env,context,TextAssetsHelper::ARIAL_PLAIN);
    //create the green and blue mesh
    float tesselatedRectSize=2.5; //6.2f
    const float offsetY=0.0f;
    blueMeshB.initializeAndUploadGL(
            ColoredGeometry::makeTessellatedColoredRectWireframe(LINE_MESH_TESSELATION_FACTOR,{-tesselatedRectSize/2.0f,-tesselatedRectSize/2.0f+offsetY,-2},tesselatedRectSize,tesselatedRectSize,
                    Color::BLUE),GL_LINES);
    greenMeshB.initializeAndUploadGL(
            ColoredGeometry::makeTessellatedColoredRectWireframe(LINE_MESH_TESSELATION_FACTOR,{-tesselatedRectSize/2.0f,-tesselatedRectSize/2.0f+offsetY,-2},tesselatedRectSize,tesselatedRectSize,
                    Color::GREEN),GL_LINES);
    //create the occlusion mesh, left and right viewport
    //use a slightly different color than clear color to make mesh visible
    const TrueColor color=Color::fromRGBA(0.1,0.1,0.1,1.0);
    mOcclusionMesh[0].initializeGL();
    mOcclusionMesh[1].initializeGL();
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(vrHeadsetParams,color,mOcclusionMesh);
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
    vrHeadsetParams.updateLatestHeadSpaceFromStartSpaceRotation();

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
        distortionManager.updateDistortionWithIdentity();
        for(uint32_t eye=0;eye<2;eye++){
            drawEyeGvrRenderbuffer(static_cast<gvr::Eye>(eye));
        }
        frame.Unbind();
        frame.Submit(buffer_viewports, vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation_());
    }
    if(RENDER_SCENE_USING_VERTEX_DISPLACEMENT){
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        vrHeadsetParams.updateDistortionManager(distortionManager);
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
    const gvr::Mat4f perspective =ndk_hello_vr::PerspectiveMatrixFromView(fov, vrHeadsetParams.MIN_Z_DISTANCE,vrHeadsetParams.MAX_Z_DISTANCE);
    const auto eyeM=gvr_api_->GetEyeFromHeadMatrix(eye==0 ? GVR_LEFT_EYE : GVR_RIGHT_EYE);
    //const auto rotM=gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    const auto rotM=vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation_();
    const auto viewM=toGLM(ndk_hello_vr::MatrixMul(eyeM,rotM));
    const auto projectionM=toGLM(perspective);
    glLineWidth(6.0f);
    drawEye(eye,viewM,projectionM,false);
    GLHelper::checkGlError("RendererDistortion::drawEyeGvr");
}

void RendererDistortion::drawEyeVDDC(gvr::Eye eye) {
    vrHeadsetParams.setOpenGLViewport(eye);
    distortionManager.setEye(eye==0);
    const auto rotM=vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation();
    auto viewM=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotM;
    auto projM=vrHeadsetParams.GetProjectionMatrix(eye);
    glLineWidth(3.0f);
    drawEye(eye,viewM,projM,true,true);
    GLHelper::checkGlError("RendererDistortion::drawEyeVDDC2");
}

void RendererDistortion::drawEye(gvr::Eye eye, glm::mat4 viewM, glm::mat4 projM, bool meshColorGreen, bool occlusion) {
    const VertexBuffer& tmp=meshColorGreen ? greenMeshB : blueMeshB;
    mBasicGLPrograms->vc.drawX(viewM,projM,tmp);

    if(occlusion){
        mBasicGLPrograms->vc2D.drawX(glm::mat4(1.0f),glm::mat4(1.0f),mOcclusionMesh[eye==GVR_LEFT_EYE ? 0 : 1]);
    }
    GLHelper::checkGlError("RendererDistortion::drawEye");
}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_StereoVR_DistortionTest_RendererDistortion_##method_name

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

JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instancePointer,jobject instanceMyVrHeadsetParams) {
    const MVrHeadsetParams deviceParams=createFromJava(env, instanceMyVrHeadsetParams);
    native(instancePointer)->vrHeadsetParams.updateHeadsetParams(deviceParams);
}

}
