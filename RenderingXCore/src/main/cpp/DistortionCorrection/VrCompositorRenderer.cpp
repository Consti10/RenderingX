//
// Created by geier on 22/06/2020.
//

#include <CardboardViewportOcclusion.hpp>
#include "VrCompositorRenderer.h"

VrCompositorRenderer::VrCompositorRenderer(const VDDCManager::DISTORTION_MODE distortionMode):
        vddcManager(distortionMode){
}

void VrCompositorRenderer::initializeGL() {
    mGLProgramVC2D=std::make_unique<GLProgramVC2D>();
    mGLProgramVC=std::make_unique<GLProgramVC>(&vddcManager);
    mGLProgramTexture=std::make_unique<GLProgramTexture>(false,&vddcManager);
    mGLProgramTextureExt=std::make_unique<GLProgramTextureExt>(&vddcManager,false);
    const auto color=TrueColor2::BLACK;
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(distortionEngine, color, mOcclusionMesh);
    distortionEngine.updateDistortionManager(vddcManager);
}

void VrCompositorRenderer::addLayer(GLProgramTexture::TexturedMesh mesh, GLuint textureId, bool isExternalTexture, HEAD_TRACKING headTracking) {
    mesh.uploadGL();
    VRLayer vrLayer{std::move(mesh),textureId,isExternalTexture,headTracking};
    mVrLayerList.push_back(std::move(vrLayer));
}

void VrCompositorRenderer::drawLayers(gvr::Eye eye) {
    distortionEngine.setOpenGLViewport(eye);
    //TODO once
    vddcManager.updateDistortion(distortionEngine.mInverse,distortionEngine.screen_params,distortionEngine.texture_params);

    vddcManager.setEye(eye==GVR_LEFT_EYE);
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
    for(int i=0;i<debug.size();i++){
        const auto& layer=debug[i];
        const glm::mat4 viewM=distortionEngine.GetEyeFromHeadMatrix(eye) * rotation;
        mGLProgramVC->drawX(viewM,distortionEngine.GetProjectionMatrix(eye),layer.mesh);
    }
    //
    //Render the mesh that occludes everything except the part actually visible inside the headset
    if (true) {
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

