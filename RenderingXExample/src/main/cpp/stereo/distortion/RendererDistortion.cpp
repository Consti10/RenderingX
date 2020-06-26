//
// Created by Consti10 on 15/05/2019.
//
#include "RendererDistortion.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include <GLBuffer.hpp>
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
    glProgramTextureProj=new GLPTextureProj();
    glProgramTexture=new GLProgramTexture(false);
    GLProgramTexture::loadTexture(mTextureMonaLisa, env, context, "ExampleTexture/grid_2000px.png");
    const float wh=2.0f;
    glBufferTextured.uploadGL(TexturedGeometry::makeTesselatedVideoCanvas2(10,glm::vec3(0,0,0),{wh,wh},0.0f,1.0f));

    //mBasicGLPrograms=std::make_unique<BasicGLPrograms>();
    //mBasicGLPrograms->text.loadTextRenderingData(env,context,TextAssetsHelper::ARIAL_PLAIN);
    mGLProgramVC=std::make_unique<GLProgramVC>(&distortionManager);
    //create the green and blue mesh
    float tesselatedRectSize=2.0; //6.2f
    blueMeshB=GLProgramVC::Mesh(
            ColoredGeometry::makeTessellatedColoredRectWireframe(LINE_MESH_TESSELATION_FACTOR, {0,0,-2}, {tesselatedRectSize,tesselatedRectSize},
                                                                 TrueColor2::BLUE), GL_LINES);
    greenMeshB=GLProgramVC::Mesh(
            ColoredGeometry::makeTessellatedColoredRectWireframe(LINE_MESH_TESSELATION_FACTOR, {0,0,-2}, {tesselatedRectSize,tesselatedRectSize},
                                                                 TrueColor2::GREEN), GL_LINES);
    //create the occlusion mesh, left and right viewport
    //use a slightly different color than clear color to make mesh visible
    const auto color=TrueColor(glm::vec4{1.0f, 0.1, 0.1, 1.0});
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
    const auto& tmp=meshColorGreen ? greenMeshB : blueMeshB;
    mGLProgramVC->drawX(viewM,projM,tmp);

    //if(occlusion){
    //    mBasicGLPrograms->vc2D.drawX(glm::mat4(1.0f),glm::mat4(1.0f),mOcclusionMesh[eye==GVR_LEFT_EYE ? 0 : 1]);
    //}

    // Dev proj texturing
    /*glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
    /*{
        glProgramTexture->beforeDraw(glBufferTextured.vertexB,mTextureMonaLisa);
        glm::mat4 modelM=glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-3));

        glProgramTexture->draw(viewM*modelM,projM,0,glBufferTextured.nVertices,glBufferTextured.mMode);
        glProgramTexture->afterDraw();
    }*/
    /*{
        constexpr float DEFAULT_CAMERA_Z=10.0f;
        constexpr auto DEFAULT_CAMERA_POSITION=glm::vec3(0,0,DEFAULT_CAMERA_Z);
        constexpr auto DEFAULT_CAMERA_LOOK_AT=glm::vec3(0,0,0);
        constexpr auto DEFAULT_CAMERA_UP=glm::vec3(0,1,0);
        const auto DEFAULT_EYE_VIEW=glm::lookAt(DEFAULT_CAMERA_POSITION,DEFAULT_CAMERA_LOOK_AT,DEFAULT_CAMERA_UP);
        glProgramTextureProj->beforeDraw(glBufferTextured.vertexB, mTextureMonaLisa);
        glProgramTextureProj->draw(glm::scale(glm::mat4(1.0),glm::vec3(3.0f,3.0f,3.0f)),DEFAULT_EYE_VIEW,projM,0,glBufferTextured.nVertices,glBufferTextured.mMode);

        glm::mat4 scaleBiasMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(0.5f));
        // Construct the combined matrix to transform the vertex position to its projective
        // texture coordiantes.
        glm::mat4 modelM=glm::translate(glm::vec3(0,0,0.2));
        //glm::mat4 modelM=glm::scale(glm::mat4(1.0f),glm::vec3(0.1,0.1,0.1));
        //glm::mat4 modelM=glm::rotate(glm::radians(20.0f),glm::vec3(0,1,0));
        //glm::mat4 modelM=glm::mat4(1.0);

        //glm::mat4 viewM2=glm::lookAt()
        glm::mat4 projM2=glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);

        glm::mat4 projectorTransformMatrix = scaleBiasMatrix * projM2 *(viewM*modelM);

        glProgramTextureProj->updateTexMatrix(projectorTransformMatrix);
        glProgramTextureProj->afterDraw();
    }


    mBasicGLPrograms->vc.drawX(viewM,projM,tmp);*/

    GLHelper::checkGlError("RendererDistortion::drawEye");
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

JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instancePointer,jobject instanceMyVrHeadsetParams) {
    const MVrHeadsetParams deviceParams=createFromJava(env, instanceMyVrHeadsetParams);
    native(instancePointer)->vrHeadsetParams.updateHeadsetParams(deviceParams);
}

}
