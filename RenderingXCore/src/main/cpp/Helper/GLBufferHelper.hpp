
#ifndef RENDERINGX_CORE_HELPER_GLBufferHelper
#define RENDERINGX_CORE_HELPER_GLBufferHelper

#include <GLES2/gl2.h>
#include "GeometryBuilder/TexturedGeometry.hpp"

class GLBufferHelper{
public:
    static void allocateGLBufferStatic(GLuint buff,void* array,GLsizeiptr arraySizeBytes){
        glBindBuffer(GL_ARRAY_BUFFER,buff);
        glBufferData(GL_ARRAY_BUFFER, arraySizeBytes,
                     array, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    static void allocateGLBufferDynamic(GLuint buff,void* array,GLsizeiptr arraySizeBytes){
        glBindBuffer(GL_ARRAY_BUFFER,buff);
        glBufferData(GL_ARRAY_BUFFER, arraySizeBytes,
                     array, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    template<typename T> static int allocateGLBufferStatic(const GLuint buff,const std::vector<T>& data){
        const auto size=data.size();
        const GLsizeiptr sizeBytes=size*sizeof(T);
        allocateGLBufferStatic(buff,(void*)data.data(),sizeBytes);
        return (int)size;
    }
    template<typename T> static int allocateGLBufferDynamic(const GLuint buff,const std::vector<T>& data){
        const auto size=data.size();
        const GLsizeiptr sizeBytes=size*sizeof(T);
        allocateGLBufferDynamic(buff,(void*)data.data(),sizeBytes);
        return (int)size;
    }
    static void allocateGLBufferStatic(const GLuint vertexB,const GLuint indexB,const TexturedGeometry::IndicesVertices& data){
        allocateGLBufferStatic(vertexB,data.vertices);
        allocateGLBufferStatic(indexB,data.indices);
    }

    template<typename T> static int createAllocateGLBufferStatic(GLuint& buff,const std::vector<T>& data){
        glGenBuffers(1,&buff);
        const auto size=data.size();
        const GLsizeiptr sizeBytes=size*sizeof(T);
        allocateGLBufferStatic(buff,(void*)data.data(),sizeBytes);
        return (int)size;
    }
    template<typename T> static int createAllocateGLBufferDynamic(GLuint& buff,const std::vector<T>& data){
        glGenBuffers(1,&buff);
        const auto size=data.size();
        const GLsizeiptr sizeBytes=size*sizeof(T);
        allocateGLBufferDynamic(buff,(void*)data.data(),sizeBytes);
        return (int)size;
    }
public:
    struct VertexIndexBuffer{
        GLuint vertexB;
        GLuint indexB;
        int nIndices;
    };
    template <typename T,typename T2>
    static void createAllocateVertexIndexBuffer(const std::vector<T> &vertices,
                                                const std::vector<T2> &indices,
                                                VertexIndexBuffer &vertexIndexBuffer){
        createAllocateGLBufferStatic(vertexIndexBuffer.vertexB,vertices);
        vertexIndexBuffer.nIndices=createAllocateGLBufferStatic(vertexIndexBuffer.indexB,indices);
    }
public:
    struct VertexBuffer{
            GLuint vertexB;
            int nVertices;
    };
    template <typename T>
    static void createAllocateVertexBuffer(const std::vector<T> &vertices,
                                                VertexBuffer &vertexBuffer){
        vertexBuffer.nVertices=createAllocateGLBufferStatic(vertexBuffer.vertexB,vertices);
    }
};

#endif //RENDERINGX_CORE_HELPER_GLBufferHelper