//
// Created by geier on 26/06/2020.
//

#ifndef FPV_VR_OS_ABSTRACTMESH_HPP
#define FPV_VR_OS_ABSTRACTMESH_HPP

#include <optional>
#include <GLBuffer.hpp>
#include <AndroidLogger.hpp>

// A mesh always has vertices, optionally also has indices.
// Mode is one of  GL_TRIANGLES, GL_TRIANGLE_STRIP ...
template<typename VERTEX,typename INDEX>
class AbstractMesh{
    using VERTICES=std::vector<VERTEX>;
    using INDICES=std::vector<INDEX>;
public:
    VERTICES vertices;
    GLBuffer<VERTEX> glBufferVertices;
    std::optional<INDICES> indices=std::nullopt;
    std::optional<GLBuffer<INDEX>> glBufferIndices=std::nullopt;
    GLenum mode;
    // OpenGL
    AbstractMesh(){}
    AbstractMesh(VERTICES vertices,INDICES indices,GLenum mode1):
            vertices(std::move(vertices)),
            indices(std::move(indices)),
            mode(mode1){
        uploadGL(true);
    }
    AbstractMesh(VERTICES vertices,GLenum mode1):
            vertices(std::move(vertices)),
            mode(mode1){
        uploadGL(true);
    }
    bool hasIndices()const{
        return indices!=std::nullopt;
    }
    int getCount()const{
        return hasIndices() ? glBufferIndices->count : glBufferVertices.count;
    }
    void uploadGL(const bool deleteDataFromCPU=true){
        glBufferVertices.uploadGL(vertices);
        if(indices){
            glBufferIndices=GLBuffer<INDEX>();
            glBufferIndices->uploadGL(*indices);
        }
        if(deleteDataFromCPU){
            vertices.resize(0);
            if(indices!=std::nullopt){
                indices->resize(0);
            }
        }
    }
    void logWarningWhenDrawnUninitialized()const{
        if(!glBufferVertices.alreadyUploaded || (glBufferIndices!=std::nullopt && !glBufferIndices->alreadyUploaded)){
            MLOGD<<"Mesh - not uploaded !";
        }
    }
};

#endif //FPV_VR_OS_ABSTRACTMESH_HPP
