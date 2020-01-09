//
// Created by Consti10 on 15/05/2019.
//
#include "example_distortion.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include "Helper/GLBufferHelper.hpp"
#include "../HelperX.h"
#include <MatrixHelper.h>
#include <array>

constexpr auto TAG="DistortionExample";

ExampleRenderer::ExampleRenderer(JNIEnv *env, jobject androidContext,gvr_context *gvr_context,int screenWidthP,int screenHeightP):
screenWidthP(screenWidthP),screenHeightP(screenHeightP),distortionManager(DistortionManager::RADIAL_2),mFPSCalculator("OpenGL FPS",2000){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
    buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    recommended_buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    scratch_viewport = gvr_api_->CreateBufferViewport();
}


void ExampleRenderer::onSurfaceCreated(JNIEnv *env, jobject context) {
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

    //
    float tesselatedRectSize=2.5; //6.2f
    const float offsetY=0.0f;
    auto tmp=ColoredGeometry::makeTesselatedColoredRectLines(LINE_MESH_TESSELATION_FACTOR,{-tesselatedRectSize/2.0f,-tesselatedRectSize/2.0f+offsetY,-2},tesselatedRectSize,tesselatedRectSize,Color::BLUE);
    nColoredVertices=GLBufferHelper::createAllocateGLBufferStatic(glBufferVC,tmp);
    tmp=ColoredGeometry::makeTesselatedColoredRectLines(LINE_MESH_TESSELATION_FACTOR,{-tesselatedRectSize/2.0f,-tesselatedRectSize/2.0f+offsetY,-2},tesselatedRectSize,tesselatedRectSize,Color::GREEN);
    GLBufferHelper::createAllocateGLBufferStatic(glBufferVCGreen,tmp);
    GLHelper::checkGlError("example_renderer::onSurfaceCreated");
}

void ExampleRenderer::onSurfaceChanged(int width, int height) {
    //Nothing
}

void ExampleRenderer::updateBufferViewports() {
    recommended_buffer_viewports.SetToRecommendedBufferViewports();
    for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        buffer_viewports.SetBufferViewport(eye,scratch_viewport);
    }
}

void ExampleRenderer::onDrawFrame() {
    mFPSCalculator.tick();
    //LOGD("FPS: %f",mFPSCalculator.getCurrentFPS());
    vrHeadsetParams.updateHeadView();

    updateBufferViewports();

    gvr::Frame frame = swap_chain->AcquireFrame();
    frame.BindBuffer(0); //0 is the 0 from createSwapChain()

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.3f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    distortionManager.updateDistortionWithIdentity();
    for(uint32_t eye=0;eye<2;eye++){
        drawEye(static_cast<gvr::Eye>(eye));
    }
    frame.Unbind();
    frame.Submit(buffer_viewports, vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation_());
    //
    //glClearColor(0.3f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    vrHeadsetParams.updateDistortionManager(distortionManager);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    for(int eye=0;eye<2;eye++){
        drawEyeVDDC(static_cast<gvr::Eye>(eye));
    }
    GLHelper::checkGlError("ExampleRenderer2::drawFrame");
}

void ExampleRenderer::drawEye(gvr::Eye eye) {
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
    const auto rotM=gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    const auto viewM=toGLM(ndk_hello_vr::MatrixMul(eyeM,rotM));
    //const auto viewM=toGLM(eyeM);
    const auto projectionM=toGLM(perspective);
    glLineWidth(6.0f);
    mBasicGLPrograms->vc.beforeDraw(glBufferVC);
    mBasicGLPrograms->vc.draw(viewM,projectionM,0,nColoredVertices,GL_LINES);
    mBasicGLPrograms->vc.afterDraw();
}

void ExampleRenderer::drawEyeVDDC(gvr::Eye eye) {
    const int ViewPortW=(int)(screenWidthP/2.0f);
    const int ViewPortH=(int)(screenHeightP);
    if(eye==0){
        glViewport(0,0,ViewPortW,ViewPortH);
    }else{
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
    }
    distortionManager.leftEye=eye==0;
    const auto rotM=toGLM(gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow()));
    auto viewM=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotM;
    auto projM=vrHeadsetParams.mProjectionM[eye];
    glLineWidth(3.0f);
    mBasicGLPrograms->vc.beforeDraw(glBufferVCGreen);
    mBasicGLPrograms->vc.draw(viewM,projM,0,nColoredVertices,GL_LINES);
    mBasicGLPrograms->vc.afterDraw();
}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_renderer2_GLRTest_##method_name

inline jlong jptr(ExampleRenderer *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline ExampleRenderer *native(jlong ptr) {
    return reinterpret_cast<ExampleRenderer*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong native_gvr_api,jint w,jint h) {
    return jptr(new ExampleRenderer(env,androidContext,reinterpret_cast<gvr_context *>(native_gvr_api),w,h));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
    delete native(p);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong p,jobject androidContext) {
    native(p)->onSurfaceCreated(env,androidContext);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj,jlong p,jint width,jint height) {
    native(p)->onSurfaceChanged((int)width,(int)height);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onDrawFrame();
}

JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong glRendererStereo,
 jfloat screen_width_meters,
 jfloat screen_height_meters,
 jfloat screen_to_lens_distance,
 jfloat inter_lens_distance,
 jint vertical_alignment,
 jfloat tray_to_lens_distance,
 jfloatArray device_fov_left,
 jfloatArray radial_distortion_params
) {
    std::array<float,4> device_fov_left1{};
    std::vector<float> radial_distortion_params1(2);

    jfloat *arrayP=env->GetFloatArrayElements(device_fov_left, nullptr);
    std::memcpy(device_fov_left1.data(),&arrayP[0],4*sizeof(float));
    env->ReleaseFloatArrayElements(device_fov_left,arrayP,0);
    arrayP=env->GetFloatArrayElements(radial_distortion_params, nullptr);
    std::memcpy(radial_distortion_params1.data(),&arrayP[0],2*sizeof(float));
    env->ReleaseFloatArrayElements(radial_distortion_params,arrayP,0);

    const MDeviceParams deviceParams{screen_width_meters,screen_height_meters,screen_to_lens_distance,inter_lens_distance,vertical_alignment,tray_to_lens_distance,
                                     device_fov_left1,radial_distortion_params1};
    native(glRendererStereo)->vrHeadsetParams.updateHeadsetParams(deviceParams);
}

}
