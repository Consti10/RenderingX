//
// Created by geier on 22/06/2020.
//

#include <CardboardViewportOcclusion.hpp>
#include "VrCompositorRenderer.h"

VrCompositorRenderer::VrCompositorRenderer(const bool ENABLE_VDDC,const TrueColor occlusionMeshColor1):
        ENABLE_VDDC(ENABLE_VDDC),
        occlusionMeshColor(occlusionMeshColor1){
}

void VrCompositorRenderer::initializeGL() {
    mGLProgramVC2D=std::make_unique<GLProgramVC2D>();
    mGLProgramTexture=std::make_unique<GLProgramTexture>(false,true);
    mGLProgramTextureExt=std::make_unique<GLProgramTextureExt>(true,false);
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(distortionEngine, occlusionMeshColor, mOcclusionMesh);
    //distortionEngine.updateDistortionManager(vddcManager);
}

void VrCompositorRenderer::addLayer(GLProgramTexture::TexturedMesh mesh, GLuint textureId, bool isExternalTexture, HEAD_TRACKING headTracking) {
    mesh.uploadGL();
    VRLayer vrLayer{std::move(mesh),textureId,isExternalTexture,headTracking};
    mVrLayerList.push_back(std::move(vrLayer));
}

void VrCompositorRenderer::drawLayers(gvr::Eye eye) {
    const auto dataUnDistortion=getDataUnDistortion();
    const bool leftEye=eye==GVR_LEFT_EYE;
    mGLProgramTexture->updateUnDistortionUniforms(leftEye, dataUnDistortion);
    mGLProgramTextureExt->updateUnDistortionUniforms(leftEye, dataUnDistortion);

    distortionEngine.setOpenGLViewport(eye);
    const auto rotation = distortionEngine.GetLatestHeadSpaceFromStartSpaceRotation();

    for(int i=0;i<mVrLayerList.size();i++){
        const auto& layer=mVrLayerList[i];
        // Calculate the view matrix for this layer.
        const glm::mat4 viewM= layer.headTracking==NONE ? distortionEngine.GetEyeFromHeadMatrix(eye) : distortionEngine.GetEyeFromHeadMatrix(eye) * rotation;
        //
        GLProgramTexture* glProgramTexture=layer.isExternalTexture ? mGLProgramTextureExt.get() : mGLProgramTexture.get();
        /*if(layer.geometry.index()==0){
            const VertexBuffer& vb=std::get<0>(layer.geometry);
            glProgramTexture->drawX(layer.textureId, viewM, distortionEngine.GetProjectionMatrix(eye), vb);
        }else{
            const VertexIndexBuffer& vib=std::get<1>(layer.geometry);
            glProgramTexture->drawX(layer.textureId, viewM, distortionEngine.GetProjectionMatrix(eye), vib);
        }*/
        glProgramTexture->drawX(layer.textureId,viewM,distortionEngine.GetProjectionMatrix(eye),layer.mesh);
    }
    //
    //Render the mesh that occludes everything except the part actually visible inside the headset
    if (ENABLE_OCCLUSION_MESH) {
        int idx = eye == GVR_LEFT_EYE ? 0 : 1;
        mGLProgramVC2D->drawX(glm::mat4(1.0f), glm::mat4(1.0f), mOcclusionMesh[idx]);
    }
    GLHelper::checkGlError("VrCompositorRenderer::drawLayers");
}

void VrCompositorRenderer::removeLayers() {
    for(auto& layer:mVrLayerList){
        //layer.geometry.deleteGL();
    }
    mVrLayerList.resize(0);
}

void VrCompositorRenderer::updateHeadsetParams(const MVrHeadsetParams &mDP) {
    distortionEngine.updateHeadsetParams(mDP);
}

void VrCompositorRenderer::createVrRenderbuffer(VrCompositorRenderer::VrRenderbuffer &rb,int W, int H) {
    GLHelper::checkGlError("createVrRenderbuffer1");
    // Create render texture.
    glGenTextures(1, &rb.texture);
    glBindTexture(GL_TEXTURE_2D, rb.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //  GL_RGBA8_OES
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W,H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Create render target.
    glGenFramebuffers(1, &rb.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, rb.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           rb.texture, 0);
    auto status=glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status!=GL_FRAMEBUFFER_COMPLETE){
        MLOGE<<"Framebuffer not complete "<<status;
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    rb.WIDTH_PX=W;
    rb.HEIGH_PX=H;
    GLHelper::checkGlError("createVrRenderbuffer2");
}

VDDC::DataUnDistortion VrCompositorRenderer::getDataUnDistortion()const {
    if(!ENABLE_VDDC){
        return VDDC::DataUnDistortion::identity();
    }
    return VDDC::DataUnDistortion{{distortionEngine.mInverse},distortionEngine.screen_params,distortionEngine.texture_params};
}

/*void VrCompositorRenderer::drawLayersMono(glm::mat4 ViewM, glm::mat4 ProjM) {
    const float scale=100.0f;
    const glm::mat4 scaleM=glm::scale(glm::vec3(scale,scale,scale));
    const glm::mat4 modelMatrix=glm::rotate(glm::mat4(1.0F),glm::radians(90.0F), glm::vec3(0,0,-1))*scaleM;
    for(const auto& layer : mVrLayerList){
        GLProgramTexture* glProgramTexture=layer.isExternalTexture ? mGLProgramTextureExt.get() : mGLProgramTexture.get();
        if(layer.geometry.index()==0){
            const VertexBuffer& vb=std::get<0>(layer.geometry);
            glProgramTexture->drawX(layer.textureId,ViewM,ProjM,vb);
        }else{
            const VertexIndexBuffer& vib=std::get<1>(layer.geometry);
            glProgramTexture->drawX(layer.textureId,ViewM,ProjM,vib);
        }
    }
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas360");
}*/

