//
// Created by geier on 26/06/2020.
//

#ifndef FPV_VR_OS_ABSTRACTMESH_HPP
#define FPV_VR_OS_ABSTRACTMESH_HPP

#include <optional>
#include <GLBuffer.hpp>
#include <AndroidLogger.hpp>
#include <IndicesHelper.hpp>

// A mesh always has vertices, optionally also has indices.
// Mode is one of  GL_TRIANGLES, GL_TRIANGLE_STRIP ...
template<typename VERTEX,typename INDEX>
class AbstractMeshData{
    using VERTICES=std::vector<VERTEX>;
    using INDICES=std::vector<INDEX>;
public:
    VERTICES vertices;
    std::optional<INDICES> indices=std::nullopt;
    GLenum mode;
    AbstractMeshData()=default;
    AbstractMeshData(const AbstractMeshData&)=default;
    AbstractMeshData(AbstractMeshData&&)=default;
    AbstractMeshData(VERTICES vertices,INDICES indices,GLenum mode1):
    vertices(std::move(vertices)),
    indices(std::move(indices)),
    mode(std::move(mode1)){}
    AbstractMeshData(VERTICES vertices,GLenum mode1):
    vertices(std::move(vertices)),
    indices(std::nullopt),
    mode(std::move(mode1)){}
    // Sometimes we want no indices for simplicity over performance
    void mergeIndicesIntoVertexBuffer(){
        if(indices==std::nullopt || indices->size()==0){
            MLOGE<<"Called mergeIndicesIntoVertexBuffer but Mesh has no indices ";
            return;
        }
        const auto verticesOnly=IndicesHelper::mergeIndicesIntoVertices(vertices,*indices);
        vertices=verticesOnly;
        indices=std::nullopt;
    }
};

// An abstract GL Mesh (OpenGL Mesh) is a wrapper around
// One GL Vertex buffer and an optional GL Index buffer
template<typename VERTEX,typename INDEX>
class AbstractGLMesh{
public:
    GLBuffer<VERTEX> glBufferVertices;
    std::optional<GLBuffer<INDEX>> glBufferIndices=std::nullopt;
    GLenum mode;
    AbstractGLMesh()=default;
    // Delete copy
    AbstractGLMesh(const AbstractGLMesh&)=delete;
    AbstractGLMesh( AbstractGLMesh&&)=default;
    // Return self for Method chaining
    void setData(const AbstractMeshData<VERTEX,INDEX>& meshData){
        glBufferVertices.uploadGL(meshData.vertices);
        if(meshData.indices!=std::nullopt){
            glBufferIndices=GLBuffer<INDEX>();
            glBufferIndices->uploadGL(*meshData.indices);
        }else{
            glBufferIndices=std::nullopt;
        }
        mode=meshData.mode;
        //return *this;
    }
    bool hasIndices()const{
        return glBufferIndices!=std::nullopt;
    }
    int getCount()const{
        return hasIndices() ? glBufferIndices->count : glBufferVertices.count;
    }
    void logWarningWhenDrawnUninitialized()const{
        if(!glBufferVertices.alreadyUploaded || (glBufferIndices!=std::nullopt && !glBufferIndices->alreadyUploaded)){
            MLOGD<<"Mesh - not uploaded !";
        }
    }
};

#endif //FPV_VR_OS_ABSTRACTMESH_HPP
