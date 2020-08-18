//
// Created by geier on 22/06/2020.
//

#include <CardboardViewportOcclusion.hpp>
#include <Sphere/SphereBuilder.hpp>
#include <android/trace.h>
#include <ColoredGeometry.hpp>
#include "VrCompositorRenderer.h"

VrCompositorRenderer::VrCompositorRenderer(JNIEnv* env,jobject androidContext,gvr::GvrApi *gvr_api,const bool ENABLE_VDDC,const bool ENABLE_DEBUG1,const bool ENABLE_VIGNETTE):
        ENABLE_DEBUG(ENABLE_DEBUG1),
        ENABLE_VIGNETTE(ENABLE_VIGNETTE),
        gvr_api(gvr_api),
        ENABLE_VDDC(ENABLE_VDDC){
    const MVrHeadsetParams deviceParams=createFromJava2(env,androidContext);
    updateHeadsetParams(deviceParams);
}

void VrCompositorRenderer::initializeGL() {
    mGLProgramVC2D=std::make_unique<GLProgramVC2D>();
    mGLProgramTexture2D=std::make_unique<GLProgramTexture>(false,false,true);
    mGLProgramTextureExt2D=std::make_unique<GLProgramTextureExt>(false,true,false);
    mGLProgramTextureVDDC=std::make_unique<GLProgramTexture>(false, true);
    mGLProgramTextureExtVDDC=std::make_unique<GLProgramTextureExt>(true, false);
    const TrueColor occlusionMeshColor=ENABLE_DEBUG ? TrueColor2::RED : TrueColor2::BLACK;
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(*this, occlusionMeshColor, mOcclusionMesh);
    //
    solidRectangleBlack.setData(
            ColoredGeometry::makeTessellatedColoredRect(10, {0,0,0}, {2,2}, TrueColor2::BLACK));
    solidRectangleYellow.setData(
            ColoredGeometry::makeTessellatedColoredRect(10, {0,0,0}, {2,2}, TrueColor2::YELLOW));
}

void VrCompositorRenderer::updateLatestHeadSpaceFromStartSpaceRotation() {
    if(gvr_api!=nullptr){
        auto tmp=gvr_api->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
        latestHeadSpaceFromStartSpaceRotation=toGLM(tmp);
    }else{
        latestHeadSpaceFromStartSpaceRotation=glm::mat4(1.0f);
    }
}

glm::mat4 VrCompositorRenderer::GetLatestHeadSpaceFromStartSpaceRotation()const{
    return latestHeadSpaceFromStartSpaceRotation;
}

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
    if(!ENABLE_VDDC){
        mDataUnDistortion=VDDC::DataUnDistortion::identity();
        mInverse=PolynomialRadialInverse(VDDC::N_RADIAL_UNDISTORTION_COEFICIENTS);
        mDistortion=PolynomialRadialDistortion();
    }else{
        mDataUnDistortion=VDDC::DataUnDistortion{{mInverse},screen_params,texture_params};
    }
}

void VrCompositorRenderer::addLayer(const TexturedStereoMeshData &meshData,VrContentProvider vrContentProvider,HEAD_TRACKING headTracking) {
    //MLOGD<<"Add layer";
    VRLayer vrLayer;
    if(headTracking==HEAD_TRACKING::NONE){
        TexturedMeshData distortedMeshData1=distortMesh(GVR_LEFT_EYE,GLProgramTexture::convert(meshData,true));
        TexturedMeshData distortedMeshData2=distortMesh(GVR_RIGHT_EYE,GLProgramTexture::convert(meshData,false));
        vrLayer.meshLeftAndRightEye=nullptr;
        vrLayer.optionalLeftEyeDistortedMesh=std::make_unique<TexturedGLMeshBuffer>(distortedMeshData1);
        vrLayer.optionalRightEyeDistortedMesh=std::make_unique<TexturedGLMeshBuffer>(distortedMeshData2);
    }else{
        vrLayer.meshLeftAndRightEye=std::make_unique<TexturedStereoGLMeshBuffer>(meshData);
    }
    vrLayer.contentProvider=vrContentProvider;
    vrLayer.headTracking=headTracking;
    mVrLayerList.push_back(std::move(vrLayer));
}

void VrCompositorRenderer::addLayer2DCanvas(float z, float width, float height,VrContentProvider vrContentProvider,HEAD_TRACKING headTracking) {
    const auto mesh=TexturedGeometry::makeTesselatedVideoCanvas(12,{0, 0,z}, {width, height},0.0f,1.0f);
    addLayer(mesh,vrContentProvider,headTracking);
}

void VrCompositorRenderer::addLayerSphere360(float radius,UvSphere::MEDIA_FORMAT format,VrContentProvider vrContentProvider) {
    const auto sphere=SphereBuilder::createSphereEquirectangularMonoscopic(radius, 72, 36,format);
    addLayer(sphere,vrContentProvider,HEAD_TRACKING::FULL);
}

void VrCompositorRenderer::drawLayers(gvr::Eye eye) {
    ATrace_beginSection((eye==GVR_LEFT_EYE ? "VrCompositorRenderer::drawLayers LEFT" : "VrCompositorRenderer::drawLayers RIGHT"));
    const int EYE_IDX=eye==GVR_LEFT_EYE ? 0 : 1;
    cpuTime[EYE_IDX].start();
    const bool leftEye=eye==GVR_LEFT_EYE;
    mGLProgramTextureVDDC->updateUnDistortionUniforms(leftEye, mDataUnDistortion);
    mGLProgramTextureExtVDDC->updateUnDistortionUniforms(leftEye, mDataUnDistortion);
    const auto viewport=getViewportForEye(eye);
    glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    const auto rotation = GetLatestHeadSpaceFromStartSpaceRotation();
    for(int i=0;i<mVrLayerList.size();i++){
        const auto& layer=mVrLayerList[i];
        // Calculate the view matrix for this layer.
        const glm::mat4 viewM= layer.headTracking==NONE ? eyeFromHead[EYE_IDX] : eyeFromHead[EYE_IDX] * rotation;
        const bool isExternalTexture=std::holds_alternative<SurfaceTextureUpdate*>(layer.contentProvider);
        const GLint textureId=isExternalTexture ? std::get<SurfaceTextureUpdate*>(layer.contentProvider)->getTextureId() :
                              std::get<VrRenderBuffer2*>(layer.contentProvider)->getLatestRenderedTexture();
        if(layer.headTracking==HEAD_TRACKING::NONE){
            TexturedGLMeshBuffer* distortedMesh= eye == GVR_LEFT_EYE ? layer.optionalLeftEyeDistortedMesh.get() :
                    layer.optionalRightEyeDistortedMesh.get();
            GLProgramTexture* glProgramTexture2D=isExternalTexture ? mGLProgramTextureExt2D.get() : mGLProgramTexture2D.get();
            glProgramTexture2D->drawX(textureId,glm::mat4(1.0f),glm::mat4(1.0f),*distortedMesh);
        }else{
            GLProgramTexture* glProgramTexture= isExternalTexture ? mGLProgramTextureExtVDDC.get() : mGLProgramTextureVDDC.get();
            glProgramTexture->drawXStereoVertex(textureId,viewM,mProjectionM[EYE_IDX],*layer.meshLeftAndRightEye,eye==GVR_LEFT_EYE);
        }
    }
    // Render the mesh that occludes everything except the part actually visible inside the headset
    if (ENABLE_VIGNETTE) {
        int idx = eye == GVR_LEFT_EYE ? 0 : 1;
        mGLProgramVC2D->drawX( mOcclusionMesh[idx]);
    }
    GLHelper::checkGlError("VrCompositorRenderer::drawLayers");
    cpuTime[EYE_IDX].stop();
    ATrace_endSection();
}

void VrCompositorRenderer::removeLayers() {
    for(auto& layer:mVrLayerList){
        //layer.geometry.deleteGL();
    }
    mVrLayerList.resize(0);
}


