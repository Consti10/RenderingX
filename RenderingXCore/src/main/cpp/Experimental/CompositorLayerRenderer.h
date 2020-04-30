//
// Created by geier on 29/04/2020.
//

#ifndef RENDERINGX_COMPOSITORLAYERRENDERER_H
#define RENDERINGX_COMPOSITORLAYERRENDERER_H

#include <GLProgramTexture.h>
#include <variant>

// read https://developer.oculus.com/documentation/unity/unity-ovroverlay/ for terminology of VR layer(s)

enum TEXTURE_TYPE{
    TEXTURE_2D,
    // The Android 'video' texture
    TEXTURE_2D_EXTERNAL_EOS
};
enum LAYER_TYPE{
    // Most commonly used as a panel to display text (UI) or 2D video in the scene
    PLANE_2D,
    // 360° images or video
    EQUIRECT_360,
    // These are all for live streaming of not really common special 360° video formats
    // 2 different modes - mapping in shader or before uploading
    RM_360_DUAL_FISHEYE_INSTA360_1,
    RM_360_DUAL_FISHEYE_INSTA360_2,
    //Rest of the stuff added by webbn
    RM_360_KODAK_SP360_4K_DUAL,RM_360_KODAK_SP360_4K_SINGLE,RM_360_FIREFLY_SPLIT_4K,RM_360_1080P_USB,RM_360_STEREO_PI
};

class VDDCLayer{
public:
    VDDCLayer(const GLuint texture,const TEXTURE_TYPE textureType,const VertexBuffer mesh):
    texture(texture),textureType(textureType),mesh(mesh){}
    // Texture this layer samples from
    const GLuint texture;
    const TEXTURE_TYPE textureType;
    // Rendering this mesh in 3D space (with VDDC) determines the final position of the layer.
    // Here we assume the position of the layer doesn't change and can therefore be baked into a mesh
    const VertexBuffer mesh;
};

class CompositorLayerRenderer {
public:
    /**
     * @param layerTransform Matrix that places the Layer in the right Position in 3D Space.( modelMatrix in other words)
     *  For example: If the camera is positioned at (0,0,0) you can use a translation matrix of (0,0,-3) to make a 2D layer visible
     * @param ViewM view matrix of the current scene (See DistortionEngine::Get ... )
     * @param ProjM proj matrix of the current scene (See DistortionEngine::Get ... )
     * @param layerType how the texture data should be interpreted (even a 360° video sphere uses a 2D texture )
     * @param texture a valid OpenGL texture
     * @param textureType the external texture needs a special sampler (e.g. slightly different OpenGL shader)
     */
    void drawLayer(const glm::mat4x4& layerTransform,const glm::mat4x4& ViewM,const glm::mat4x4& ProjM,const LAYER_TYPE layerType,const GLuint texture,const TEXTURE_TYPE textureType);

    void drawLayer(const VDDCLayer& layer,const glm::mat4x4& ViewM,const glm::mat4x4& ProjM){
        if(layer.textureType==TEXTURE_TYPE::TEXTURE_2D){
            glProgramTexture->drawX(layer.texture,ViewM,ProjM,layer.mesh);
        }else{
            glProgramTextureExt->drawX(layer.texture,ViewM,ProjM,layer.mesh);
        }
    }
    GLProgramTexture* glProgramTexture;
    GLProgramTextureExt* glProgramTextureExt;
    GLProgramTextureExt* glProgramTextureExtMapping;
};


#endif //RENDERINGX_COMPOSITORLAYERRENDERER_H
