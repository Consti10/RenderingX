//
// Created by Consti10 on 15/05/2019.
//
#include "Renderer360Video.h"
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
#define MLOG LOG2(TAG)

Renderer360Video::Renderer360Video(JNIEnv *env, jobject androidContext, gvr_context *gvr_context, bool RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                       bool RENDER_SCENE_USING_VERTEX_DISPLACEMENT, const int vSPHERE_MODE):
        RENDER_SCENE_USING_GVR_RENDERBUFFER(RENDER_SCENE_USING_GVR_RENDERBUFFER),
        RENDER_SCENE_USING_VERTEX_DISPLACEMENT(RENDER_SCENE_USING_VERTEX_DISPLACEMENT),
        M_SPHERE_MODE(static_cast<SPHERE_MODE>(vSPHERE_MODE)),
        distortionManager(VDDCManager::RADIAL_CARDBOARD), mFPSCalculator("OpenGL FPS", 2000){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
    buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    recommended_buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    scratch_viewport = gvr_api_->CreateBufferViewport();
}


void Renderer360Video::onSurfaceCreated(JNIEnv *env, jobject context, int videoTexture) {
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
    mGLProgramTextureExt=std::make_unique<GLProgramTextureExt>(&distortionManager);
    mGLProgramTextureExt2=std::make_unique<GLProgramTextureExt>(&distortionManager,true);
    mVideoTexture=(GLuint)videoTexture;
    /*glGenTextures(1,&mTexture360Image);
    glGenTextures(1,&mTexture360ImageInsta360);
    GLProgramTexture::loadTexture(mTexture360Image,env,context,"360DegreeImages/gvr_testroom_mono.png");
    GLProgramTexture::loadTexture(mTexture360ImageInsta360,env,context,"360DegreeImages/insta_360_equirectangular.png");*/
    //create the insta360 sphere
    mSphereDualFisheye1.uploadGL(
            SphereBuilder::createSphereDualFisheyeInsta360(),GL_TRIANGLE_STRIP);
    //second insta360
    DualFisheyeSphere::uploadSphereGL(mSphereDualFisheye2,2560,1280);
    //create the gvr sphere
    mGvrSphereB.uploadGL(
            SphereBuilder::createSphereEquirectangularMonoscopic(1.0,72,36),GL_TRIANGLE_STRIP);
    //create the occlusion mesh, left and right viewport
    //use a slightly different color than clear color to make mesh visible
    const auto color=TrueColor(glm::vec4{0.1f, 0.1, 0.1, 1.0});
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(vrHeadsetParams,color,mOcclusionMesh);
    GLHelper::checkGlError("Renderer360Video::onSurfaceCreated");
}

void Renderer360Video::updateBufferViewports() {
    recommended_buffer_viewports.SetToRecommendedBufferViewports();
    for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        buffer_viewports.SetBufferViewport(eye,scratch_viewport);
    }
}

void Renderer360Video::onDrawFrame() {
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
    GLHelper::checkGlError("Renderer360Video::onDrawFrame");
}

void Renderer360Video::drawEyeGvrRenderbuffer(gvr::Eye eye) {
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
    GLHelper::checkGlError("Renderer360Video::drawEyeGvr");
}

void Renderer360Video::drawEyeVDDC(gvr::Eye eye) {
    vrHeadsetParams.setOpenGLViewport(eye);
    distortionManager.setEye(eye==0);
    const auto rotM=vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation();
    auto viewM=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotM;
    auto projM=vrHeadsetParams.GetProjectionMatrix(eye);
    glLineWidth(3.0f);
    drawEye(eye,viewM,projM,true);
    GLHelper::checkGlError("Renderer360Video::drawEyeVDDC2");
}

void Renderer360Video::drawEye(gvr::Eye eye, glm::mat4 viewM, glm::mat4 projM, bool occlusion) {
    switch(M_SPHERE_MODE){
        case SPHERE_MODE_EQUIRECTANGULAR_TEST:{
            mGLProgramTextureExt->drawX(mVideoTexture,viewM,projM,mGvrSphereB);
        }break;
        case SPHERE_MODE_INSTA360_TEST1:{
            glm::mat4x4 modelMatrix=glm::rotate(glm::mat4(1.0F),glm::radians(90.0F), glm::vec3(0,0,-1));
            mGLProgramTextureExt2->drawX(mVideoTexture,viewM*modelMatrix,projM,mGvrSphereB);
        }break;
        case SPHERE_MODE_INSTA360_TEST2:{
            glm::mat4x4 modelMatrix=glm::rotate(glm::mat4(1.0F),glm::radians(90.0F), glm::vec3(0,0,-1));
            mGLProgramTextureExt->drawX(mVideoTexture,viewM*modelMatrix, projM, mSphereDualFisheye2);
        }break;
        default:
            MLOG<<"Unknown sphere mode";
    }
    if(occlusion){
        mBasicGLPrograms->vc2D.drawX(glm::mat4(1.0f),glm::mat4(1.0f),mOcclusionMesh[eye==GVR_LEFT_EYE ? 0 : 1]);
    }
    GLHelper::checkGlError("Renderer360Video::drawEye");
}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_example_stereo_video360degree_Renderer360Video_##method_name

inline jlong jptr(Renderer360Video *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline Renderer360Video *native(jlong ptr) {
    return reinterpret_cast<Renderer360Video*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong native_gvr_api,jboolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
 jboolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,jint SPHERE_MODE) {
    return jptr(new Renderer360Video(env, androidContext, reinterpret_cast<gvr_context *>(native_gvr_api), RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                       RENDER_SCENE_USING_VERTEX_DISPLACEMENT,SPHERE_MODE));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
    delete native(p);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong p,jobject androidContext,jint videoTexture) {
    native(p)->onSurfaceCreated(env,androidContext,videoTexture);
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
