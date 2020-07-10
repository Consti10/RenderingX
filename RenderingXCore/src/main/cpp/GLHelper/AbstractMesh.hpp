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
// Use AbstractMeshData to create an AbstractGLMesh (which should be renderable via OpenGL)
template<class VERTEX,class INDEX>
class AbstractMeshData{
    using VERTICES=std::vector<VERTEX>;
    using INDICES=std::vector<INDEX>;
public:
    VERTICES vertices;
    // Second value holds true if mesh has indices
    std::optional<INDICES> indices=std::nullopt;
    GLenum mode;
    AbstractMeshData()=default;
    //AbstractMeshData(const AbstractMeshData&)=default;
    // a default constructor also counts as custom constructor
    // Therefore doing so deletes the move assignemnt operator
    // https://en.cppreference.com/w/cpp/language/copy_assignment#:~:text=A%20implicitly%2Ddeclared%20copy%20assignment,user%2Ddeclared%20move%20assignment%20operator.
    //AbstractMeshData(AbstractMeshData&&)=default;
    AbstractMeshData(VERTICES vertices,INDICES indices,GLenum mode1=GL_TRIANGLES):
    vertices(std::move(vertices)),
    indices(std::move(indices)),
    mode(std::move(mode1)){}
    AbstractMeshData(VERTICES vertices,GLenum mode1=GL_TRIANGLES):
    vertices(std::move(vertices)),
    mode(std::move(mode1)){}
    bool hasIndices()const{
        return indices!=std::nullopt;
    }
    // Sometimes we want no indices for simplicity over performance
    // create a new mesh without indices (vertices only)
    void mergeIndicesIntoVertices(){
        if(indices==std::nullopt || indices->size()==0){
            MLOGE<<"Called mergeIndicesIntoVertexBuffer but Mesh has no indices ";
            return;
        }
        const auto verticesOnly=IndicesHelper::mergeIndicesIntoVertices(vertices,*indices);
        vertices=verticesOnly;
        indices=std::nullopt;
    }
};

// An abstract GL Mesh buffer is a wrapper around
// One GL Vertex buffer and an optional GL Index buffer
// NOTE: Copy constructor and more have been deleted by purpose - use AbstractMeshData if you want to modify the mesh data
template<typename VERTEX,typename INDEX>
class AbstractGLMeshBuffer{
private:
    GLBuffer<VERTEX> glBufferVertices;
    //std::optional<GLBuffer<INDEX>> glBufferIndices=std::nullopt;
    // An OpenGL buffer is not expensive (and unless filled, the OpenGL counter-part is not even created)
    // True if index buffer is active,false otherwise
    std::pair<GLBuffer<INDEX>,bool> glBufferIndices;
    GLenum mode;
public:
    AbstractGLMeshBuffer()=default;
    // Same as GLBuffer
    AbstractGLMeshBuffer(const AbstractGLMeshBuffer&)=delete;
    AbstractGLMeshBuffer(AbstractGLMeshBuffer&&)=default;
    AbstractGLMeshBuffer(const AbstractMeshData<VERTEX,INDEX>& meshData){
        setData(meshData);
    }
    // Return self for Method chaining ?
    void setData(const AbstractMeshData<VERTEX,INDEX>& meshData){
        glBufferVertices.uploadGL(meshData.vertices);
        if(meshData.hasIndices()){
            //glBufferIndices=GLBuffer<INDEX>();
            glBufferIndices.first.uploadGL(*meshData.indices);
            glBufferIndices.second=true;
        }else{
            // If there exists an old index buffer, remove its content to free space for OpenGL
            if(glBufferIndices.second)glBufferIndices.first.freeDataGL();
            glBufferIndices.second=false;
        }
        mode=meshData.mode;
        //return *this;
    }
    bool hasIndices()const{
        return glBufferIndices.second;
    }
    int getCount()const{
        return hasIndices() ? glBufferIndices.first.count : glBufferVertices.count;
    }
    void logWarningWhenDrawingMeshWithoutData()const{
        if(glBufferVertices.count==0 || (glBufferIndices.second && glBufferIndices.first.count == 0)){
            MLOGD<<"Drawing Mesh without proper data";
        }
    }
    GLenum getMode()const{
        return mode;
    }
    GLuint getVertexBufferId()const{
        return glBufferVertices.glBufferId;
    }
    GLuint getIndexBufferId()const{
        return glBufferIndices.first.glBufferId;
    }
};

#endif //FPV_VR_OS_ABSTRACTMESH_HPP
