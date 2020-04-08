
#ifndef RENDERINGX_CORE_HELPER_GLBufferHelper
#define RENDERINGX_CORE_HELPER_GLBufferHelper

#include <GLES2/gl2.h>
#include <vector>
#include <array>
#include <iterator>

//Provides convenient functions to upload cpp
//std::vector<T> / std::array<T,size> data type to GPU using OpenGL c style api
class GLBufferHelper {
public:
    //c-style function that takes a data pointer and the data size in bytes
    //binds and un-binds gl buffer for data upload
    static void uploadGLBuffer(const GLuint buff,const void *array,GLsizeiptr arraySizeBytes,GLenum usage=GL_STATIC_DRAW) {
        glBindBuffer(GL_ARRAY_BUFFER, buff);
        glBufferData(GL_ARRAY_BUFFER, arraySizeBytes,
                     array,usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    //template<typename T,typename Container>
    //static void uploadGLBuffer(const GLuint buff,const Container& data,GLenum usage=GL_STATIC_DRAW){
    //    uploadGLBuffer(buff,data.data(),data.size()*sizeof(T) );
    //}
    //wrap std::vector<>
    //returns the n of elements inside the vector (NOT the n of bytes)
    //since they often are the n of triangles/indices that need to be drawn
    template<typename T>
    static int uploadGLBuffer(const GLuint buff, const std::vector<T> &data,GLenum usage=GL_STATIC_DRAW) {
        const auto size = data.size();
        uploadGLBuffer(buff, (void *) data.data(), size * sizeof(T));
        return (int) size;
    }
    //wrap std::array, similar to std::vector<>
    template<typename T,std::size_t S>
    static int uploadGLBuffer(const GLuint buff, const std::array<T,S> &data,GLenum usage=GL_STATIC_DRAW) {
        uploadGLBuffer(buff, (void *)data.data(), S * sizeof(T));
        return (int) S;
    }
    //Also create the GL Buffer, both for array and vector
    template<typename T>
    static int createUploadGLBuffer(GLuint &buff, const std::vector<T> &data,GLenum usage=GL_STATIC_DRAW) {
        glGenBuffers(1, &buff);
        return uploadGLBuffer(buff,data,usage);
    }
    template<typename T,std::size_t S>
    static int createUploadGLBuffer(GLuint &buff, const std::array<T,S> &data,GLenum usage=GL_STATIC_DRAW) {
        glGenBuffers(1,&buff);
        return uploadGLBuffer(buff,data,usage);
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
    GLenum mMode=GL_TRIANGLES;
public:
    void initializeGL(){
        glGenBuffers(1,&vertexB);
        GLHelper::checkGlError("VertexBuffer::initializeGL");
    }

    template<typename T>
    void uploadGL(const std::vector<T> &vertices,GLenum mode=GL_TRIANGLES){
        nVertices = GLBufferHelper::uploadGLBuffer(vertexB, vertices);
        mMode=mode;
        GLHelper::checkGlError("VertexBuffer::uploadGL");
        //MDebug::log("N vertices is "+std::to_string(nVertices));
    }

    template<typename T,size_t s>
    void uploadGL(const std::array<T,s> &vertices,GLenum mode=GL_TRIANGLES){
        const auto tmp=std::vector<T>(vertices.begin(),vertices.end());
        VertexBuffer::uploadGL(tmp,mode);
    }

    template<typename T>
    void initializeAndUploadGL(const std::vector<T> &vertices,GLenum mode=GL_TRIANGLES){
        initializeGL();
        uploadGL(vertices,mode);
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
    GLenum mMode=GL_TRIANGLES;
    void initializeGL(){
        glGenBuffers(1,&vertexB);
        glGenBuffers(1,&indexB);
        GLHelper::checkGlError("VertexIndexBuffer::initializeGL");
    }

    template<typename T, typename T2>
    void uploadGL(const std::vector<T> &vertices,const std::vector<T2> &indices,GLenum mode=GL_TRIANGLES){
        GLBufferHelper::uploadGLBuffer(vertexB, vertices);
        nIndices = GLBufferHelper::uploadGLBuffer(indexB, indices);
        mMode=mode;
        GLHelper::checkGlError("VertexIndexBuffer::uploadGL");
    }

    template<typename T, typename T2>
    void initializeAndUploadGL(const std::vector<T> &vertices,const std::vector<T2> &indices,GLenum mode=GL_TRIANGLES){
        initializeGL();
        uploadGL(vertices,indices,mode);
    }

    void deleteGL() {
        glDeleteBuffers(1, &vertexB);
        glDeleteBuffers(1,&indexB);
    }
};

#endif //RENDERINGX_CORE_HELPER_GLBufferHelper