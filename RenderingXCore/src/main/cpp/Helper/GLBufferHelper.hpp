
#ifndef RENDERINGX_CORE_HELPER_GLBufferHelper
#define RENDERINGX_CORE_HELPER_GLBufferHelper

#include <GLES2/gl2.h>
#include "GeometryBuilder/TexturedGeometry.hpp"

class GLBufferHelper {
public:
    static void uploadGLBufferStatic(GLuint buff, void *array, GLsizeiptr arraySizeBytes) {
        glBindBuffer(GL_ARRAY_BUFFER, buff);
        glBufferData(GL_ARRAY_BUFFER, arraySizeBytes,
                     array, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    static void uploadGLBufferDynamic(GLuint buff, void *array, GLsizeiptr arraySizeBytes) {
        glBindBuffer(GL_ARRAY_BUFFER, buff);
        glBufferData(GL_ARRAY_BUFFER, arraySizeBytes,
                     array, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    template<typename T>
    static int uploadGLBufferStatic(const GLuint buff, const std::vector<T> &data) {
        const auto size = data.size();
        const GLsizeiptr sizeBytes = size * sizeof(T);
        uploadGLBufferStatic(buff, (void *) data.data(), sizeBytes);
        return (int) size;
    }

    template<typename T>
    static int uploadGLBufferDynamic(const GLuint buff, const std::vector<T> &data) {
        const auto size = data.size();
        const GLsizeiptr sizeBytes = size * sizeof(T);
        uploadGLBufferDynamic(buff, (void *) data.data(), sizeBytes);
        return (int) size;
    }

    static void uploadGLBufferStatic(const GLuint vertexB, const GLuint indexB,
                                     const TexturedGeometry::IndicesVertices &data) {
        uploadGLBufferStatic(vertexB, data.vertices);
        uploadGLBufferStatic(indexB, data.indices);
    }

    template<typename T>
    static int createUploadGLBufferStatic(GLuint &buff, const std::vector<T> &data) {
        glGenBuffers(1, &buff);
        const auto size = data.size();
        const GLsizeiptr sizeBytes = size * sizeof(T);
        uploadGLBufferStatic(buff, (void *) data.data(), sizeBytes);
        return (int) size;
    }

    template<typename T>
    static int createUploadGLBufferDynamic(GLuint &buff, const std::vector<T> &data) {
        glGenBuffers(1, &buff);
        const auto size = data.size();
        const GLsizeiptr sizeBytes = size * sizeof(T);
        uploadGLBufferDynamic(buff, (void *) data.data(), sizeBytes);
        return (int) size;
    }
};

//Helps Generalizing the following re-occuring pattern:
//1) create vertices on stack
//2) upload vertex data to gpu, then object on stack is freed
//3) render all vertices. The nVertices has to be stored,too since it gets lost when object on stack is deleted
class VertexBuffer{
public:
    GLuint vertexB;
    int nVertices=0;

public:
    void initializeGL(){
        glGenBuffers(1,&vertexB);
    }

    template<typename T>
    void uploadGL(const std::vector<T> &vertices){
        nVertices = GLBufferHelper::uploadGLBufferStatic(vertexB, vertices);
    }

    template<typename T,size_t s>
    void uploadGL(const std::array<T,s> &vertices){
        const auto tmp=std::vector<T>(vertices.begin(),vertices.end());
        VertexBuffer::uploadGL(tmp);
    }

    template<typename T>
    void initializeAndUploadGL(const std::vector<T> &vertices){
        initializeGL();
        nVertices = GLBufferHelper::uploadGLBufferStatic(vertexB, vertices);
    }

    void deleteGL() {
        glDeleteBuffers(1, &vertexB);
    }
};

//Helps Generalizing the following re-occuring pattern:
//1) create vertices/indices on stack
//2) upload vertex/index data to gpu
//3) render all vertices using indices - the nIndices has to be stored, too
class VertexIndexBuffer {
public:
    GLuint vertexB;
    GLuint indexB;
    int nIndices=0;

    void initializeGL(){
        glGenBuffers(1,&vertexB);
        glGenBuffers(1,&indexB);
    }

    template<typename T, typename T2>
    void uploadGL(const std::vector<T> &vertices,const std::vector<T2> &indices){
        GLBufferHelper::uploadGLBufferStatic(vertexB, vertices);
        nIndices = GLBufferHelper::uploadGLBufferStatic(indexB, indices);
    }

    template<typename T, typename T2>
    void initializeAndUploadGL(const std::vector<T> &vertices,const std::vector<T2> &indices){
        initializeGL();
        GLBufferHelper::uploadGLBufferStatic(vertexB, vertices);
        nIndices = GLBufferHelper::uploadGLBufferStatic(indexB, indices);
    }

    void deleteGL() {
        glDeleteBuffers(1, &vertexB);
        glDeleteBuffers(1,&indexB);
    }
};

#endif //RENDERINGX_CORE_HELPER_GLBufferHelper