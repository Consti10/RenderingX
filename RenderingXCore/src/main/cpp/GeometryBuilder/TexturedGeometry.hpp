//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
#define FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H

#include <IndicesHelper.hpp>
#include "GLProgramTexture.h"
#include "VerticalPlane.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

#include <utility>
#include <AbstractMesh.hpp>

//Create vertex/index buffers for drawing textured geometry
namespace TexturedGeometry {
    static const GLProgramTexture::Mesh makeTesselatedVideoCanvas(const unsigned int tessellation, const glm::vec3& translation,const glm::vec2& scale, const float uOffset,
                                                           const float uRange,const bool invertUCoordinates=false,const bool invertVCoordinates=true){
        const auto vertices=VerticalPlane::createVerticesPlaneTextured(tessellation,translation,scale,uOffset,uRange,invertUCoordinates,invertVCoordinates);
        const auto indices=VerticalPlane::createIndicesPlane(tessellation);
        return GLProgramTexture::Mesh(vertices,indices,GL_TRIANGLES);
    }
    //Sometimes we want no indices for simplicity over performance
    static const GLProgramTexture::Mesh makeTesselatedVideoCanvas2(const unsigned int tessellation, const glm::vec3& translation,const glm::vec2& scale, const float uOffset,
                                                                                  const float uRange){
        const auto tmp=makeTesselatedVideoCanvas(tessellation,translation,scale,uOffset,uRange);
        const auto verticesOnly=IndicesHelper::mergeIndicesIntoVertices(tmp.vertices,*tmp.indices);
        return GLProgramTexture::Mesh(verticesOnly,GL_TRIANGLES);
    }
    static const std::vector<GLProgramTexture::Vertex> makeTesselatedVideoCanvas2(const unsigned int tessellation,
                                                                                  const glm::mat4 modelMatrix=glm::mat4(1.0f),const glm::mat4 textureMatrix=glm::mat4(1.0f)){
        const auto tmp= VerticalPlane::createVerticesPlaneTextured(tessellation, modelMatrix,textureMatrix,true);
        const auto tmp2=VerticalPlane::createIndicesPlane(tessellation);
        return IndicesHelper::mergeIndicesIntoVertices(tmp, tmp2);
    }
    static const std::vector<GLProgramTexture::Vertex> makePyramid(){
        return {
                { 1.0f, -1.0f, 1.0f, 0.0319, 0.4192 }, // face 1
                { -1.0f, -1.0f, -1.0f, 0.3546, 0.0966 },
                { 1.0f, -1.0f, -1.0f, 0.3546, 0.4192 },
                { 1.0f, -1.0f, -1.0f, 0.4223, 0.5177 }, // face 2
                { 0.0f, 1.0f, 0.0f, 0.2541, 0.8753 },
                { 1.0f, -1.0f, 1.0f, 0.0996, 0.5116 },
                { 1.0f, -1.0f, 1.0f, 0.8047, 0.5250 }, // face 3
                { 0.0f, 1.0f, 0.0f, 0.6434, 0.8857 },
                { -1.0f, -1.0f, 1.0f, 0.4820, 0.5250 },
                { -1.0f, -1.0f, 1.0f, 0.6637, 0.0981 }, // face 4
                { 0.0f, 1.0f, 0.0f, 0.5130, 0.4184 },
                { -1.0f, -1.0f, -1.0f,0.3748, 0.0926 },
                { 0.0f, 1.0f, 0.0f, 0.8416, 0.4227  }, // face 5
                { 1.0f, -1.0f, -1.0f, 0.6922, 0.0988 },
                { -1.0f, -1.0f, -1.0f,0.9834, 0.0954 },
                { 1.0f, -1.0f, 1.0f,  0.0319, 0.4192 }, // face 6
                { -1.0f, -1.0f, 1.0f, 0.0319, 0.0966 },
                { -1.0f, -1.0f, -1.0f, 0.3546, 0.0966 }
        };
    }
};


#endif //FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
