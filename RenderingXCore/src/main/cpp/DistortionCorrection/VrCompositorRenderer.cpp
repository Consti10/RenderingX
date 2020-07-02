//
// Created by geier on 22/06/2020.
//

#include <CardboardViewportOcclusion.hpp>
#include "VrCompositorRenderer.h"

VrCompositorRenderer::VrCompositorRenderer(gvr::GvrApi *gvr_api,const bool ENABLE_VDDC,const TrueColor occlusionMeshColor1):
        gvr_api(gvr_api),
        ENABLE_VDDC(ENABLE_VDDC),
        occlusionMeshColor(occlusionMeshColor1){
}

void VrCompositorRenderer::initializeGL() {
    mGLProgramVC2D=std::make_unique<GLProgramVC2D>();
    mGLProgramTexture=std::make_unique<GLProgramTexture>(false,true);
    mGLProgramTextureExt=std::make_unique<GLProgramTextureExt>(true,false);
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(*this, occlusionMeshColor, mOcclusionMesh);
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

    setOpenGLViewport(eye);
    const auto rotation = GetLatestHeadSpaceFromStartSpaceRotation();

    for(int i=0;i<mVrLayerList.size();i++){
        const auto& layer=mVrLayerList[i];
        // Calculate the view matrix for this layer.
        const glm::mat4 viewM= layer.headTracking==NONE ? GetEyeFromHeadMatrix(eye) : GetEyeFromHeadMatrix(eye) * rotation;
        //
        GLProgramTexture* glProgramTexture=layer.isExternalTexture ? mGLProgramTextureExt.get() : mGLProgramTexture.get();
        /*if(layer.geometry.index()==0){
            const VertexBuffer& vb=std::get<0>(layer.geometry);
            glProgramTexture->drawX(layer.textureId, viewM, distortionEngine.GetProjectionMatrix(eye), vb);
        }else{
            const VertexIndexBuffer& vib=std::get<1>(layer.geometry);
            glProgramTexture->drawX(layer.textureId, viewM, distortionEngine.GetProjectionMatrix(eye), vib);
        }*/
        glProgramTexture->drawX(layer.textureId,viewM,GetProjectionMatrix(eye),layer.mesh);
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

VDDC::DataUnDistortion VrCompositorRenderer::getDataUnDistortion()const {
    if(!ENABLE_VDDC){
        return VDDC::DataUnDistortion::identity();
    }
    return VDDC::DataUnDistortion{{mInverse},screen_params,texture_params};
}

void VrCompositorRenderer::addLayer2DCanvas(float z, float width, float height, GLuint textureId,
                                      bool isExternalTexture) {
    auto mesh=TexturedGeometry::makeTesselatedVideoCanvas(12,{0, 0,z}, {width, height},0.0f,1.0f);
    addLayer(std::move(mesh),textureId,isExternalTexture);
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

void VrCompositorRenderer::updateHeadsetParams(const MVrHeadsetParams &mDP) {
    this->SCREEN_WIDTH_PX=mDP.screen_width_pixels;
    this->SCREEN_HEIGHT_PX=mDP.screen_height_pixels;
    EYE_VIEWPORT_W=SCREEN_WIDTH_PX/2;
    EYE_VIEWPORT_H=SCREEN_HEIGHT_PX;
    MLOGD<<MyVrHeadsetParamsAsString(mDP);
    mDistortion=PolynomialRadialDistortion(mDP.radial_distortion_params);

    const auto GetYEyeOffsetMeters= MLensDistortion::GetYEyeOffsetMeters(mDP.vertical_alignment,
                                                                         mDP.tray_to_lens_distance,
                                                                         mDP.screen_height_meters);
    const auto fovLeft= MLensDistortion::CalculateFov(mDP.device_fov_left, GetYEyeOffsetMeters,
                                                      mDP.screen_to_lens_distance,
                                                      mDP.inter_lens_distance,
                                                      mDistortion,
                                                      mDP.screen_width_meters, mDP.screen_height_meters);
    const auto fovRight=VrCompositorRenderer::reverseFOV(fovLeft);

    MLensDistortion::CalculateViewportParameters_NDC(0, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovLeft,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     screen_params[0], texture_params[0]);
    MLensDistortion::CalculateViewportParameters_NDC(1, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovRight,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     screen_params[1], texture_params[1]);
    MLOGD<<mDistortion.toString();
    MLOGD<<"Left Eye: "<<MLensDistortion::ViewportParamsNDCAsString(screen_params[0],texture_params[0]);
    MLOGD<<"Right Eye: "<<MLensDistortion::ViewportParamsNDCAsString(screen_params[1],texture_params[1]);

    //TODO calculate right maxRadSq
    /*const float maxX=1*texture_params[0].width+texture_params[0].x_eye_offset;
    const float maxY=1*texture_params[0].height+texture_params[0].y_eye_offset;
    const float maxR2=maxX*maxX+maxY*maxY;
    const float maxR=sqrt(maxR2);
    const float iMaxRad=mDistortion.DistortRadiusInverse(maxR);
    LOGD("Max X Y %f %f |  maxR2 %f maxR %f iMaxRad %f",maxX,maxY,maxR2,maxR,iMaxRad);*/

    //Find the maximum value we can use to create a inverse polynomial distortion that
    //never has a deviation higher from x in the range [0..maxRangeInverse]
    float maxRangeInverse=1.0f;
    for(float i=1.0f;i<=2.0f;i+=0.01f){
        const auto inverse=PolynomialRadialInverse(mDistortion, maxRangeInverse, VDDC::N_RADIAL_UNDISTORTION_COEFICIENTS);
        const float maxDeviation=PolynomialRadialInverse::calculateMaxDeviation(mDistortion,inverse,maxRangeInverse);
        if(maxDeviation<=0.001f){
            maxRangeInverse=i;
        }
    }
    MLOGD<<"Max value used for getApproximateInverseDistortion()"<<maxRangeInverse;
    mInverse=PolynomialRadialInverse(mDistortion, maxRangeInverse, VDDC::N_RADIAL_UNDISTORTION_COEFICIENTS);
    MLOGD<<"Inverse is:"<<mInverse.toStringX();

    //as long as the function is still strict monotonic increasing we can increase the value that will be used for
    //clamping later in the vertex shader.
    float MAX_RAD_SQ=0;
    float last=mInverse.DistortRadius(MAX_RAD_SQ);
    for(float i=MAX_RAD_SQ;i<3;i+=0.01f){
        const float d=mInverse.DistortRadius(i);
        if(d>=last){
            //LOGD("Increasing maxRadSq %f",MAX_RAD_SQ);
            MAX_RAD_SQ=i;
            last=d;
        }else{
            //LOGD("Cannot increase %f %f",last,d);
            last=d;
            break;
        }
    }
    mInverse.maxRadSq=MAX_RAD_SQ;

    mProjectionM[0]=perspective(fovLeft,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    mProjectionM[1]=perspective(fovRight,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    const float inter_lens_distance=mDP.inter_lens_distance;
    eyeFromHead[0]=glm::translate(glm::mat4(1.0f),glm::vec3(inter_lens_distance*0.5f,0,0));
    eyeFromHead[1]=glm::translate(glm::mat4(1.0f),glm::vec3(-inter_lens_distance*0.5f,0,0));

    //
    //test2();
    //test3();
}

void VrCompositorRenderer::updateLatestHeadSpaceFromStartSpaceRotation() {
    latestHeadSpaceFromStartSpaceRotation_=gvr_api->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    latestHeadSpaceFromStartSpaceRotation=toGLM(latestHeadSpaceFromStartSpaceRotation_);
}

glm::mat4 VrCompositorRenderer::GetLatestHeadSpaceFromStartSpaceRotation()const{
    return latestHeadSpaceFromStartSpaceRotation;
}

gvr::Mat4f VrCompositorRenderer::GetLatestHeadSpaceFromStartSpaceRotation_()const {
    return latestHeadSpaceFromStartSpaceRotation_;
}

glm::mat4 VrCompositorRenderer::GetEyeFromHeadMatrix(gvr::Eye eye)const{
    return eyeFromHead[eye];
}

glm::mat4 VrCompositorRenderer::GetProjectionMatrix(gvr::Eye eye)const{
    return mProjectionM[eye];
}

void VrCompositorRenderer::setOpenGLViewport(gvr::Eye eye)const {
    if(eye==GVR_LEFT_EYE){
        glViewport(0,0,EYE_VIEWPORT_W,EYE_VIEWPORT_H);
    }else{
        glViewport(EYE_VIEWPORT_W,0,EYE_VIEWPORT_W,EYE_VIEWPORT_H);
    }
}