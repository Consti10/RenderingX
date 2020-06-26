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
    std::optional<INDICES> indices=std::nullopt;
    GLenum mode;
    // If mesh has indices count==n of indices, n of vertices otherwise
    int count;
    // OpenGL
    GLBuffer glBufferVertices;
    std::optional<GLBuffer> glBufferIndices=std::nullopt;
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
    void uploadGL(const bool deleteDataFromCPU=true){
        glBufferVertices.uploadGL(vertices);
        if(indices){
            glBufferIndices=GLBuffer();
            glBufferIndices->uploadGL(*indices);
            count=indices->size();
        }else{
            count=vertices.size();
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
