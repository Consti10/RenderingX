//
// Created by geier on 26/06/2020.
//

#ifndef FPV_VR_OS_GLBUFFER_HPP
#define FPV_VR_OS_GLBUFFER_HPP

#include <GLHelper.hpp>
#include <vector>
#include <array>
#include <iterator>

// Provides convenient functions to upload cpp
// data types to GPU using OpenGL c style api
namespace GLBufferHelper {
    //c-style function that takes a data pointer and the data size in bytes
    //binds and un-binds gl buffer for data upload
    static void uploadGLBuffer(const GLuint buff,const void *array,GLsizeiptr arraySizeBytes,GLenum usage=GL_STATIC_DRAW) {
        glBindBuffer(GL_ARRAY_BUFFER, buff);
        glBufferData(GL_ARRAY_BUFFER, arraySizeBytes,
                     array,usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    //wrap std::vector<>
    //returns the n of elements inside the vector (NOT the n of bytes)
    //since they often are the n of triangles/indices that need to be drawn
    template<class T>
    static int uploadGLBuffer(const GLuint buff, const std::vector<T> &data,GLenum usage=GL_STATIC_DRAW) {
        const auto size = data.size();
        uploadGLBuffer(buff, (void *) data.data(), size * sizeof(T));
        return (int) size;
    }
    //wrap std::array, similar to std::vector<>
    template<class T,std::size_t S>
    static int uploadGLBuffer(const GLuint buff, const std::array<T,S> &data,GLenum usage=GL_STATIC_DRAW) {
        uploadGLBuffer(buff, (void *)data.data(), S * sizeof(T));
        return (int) S;
    }
};

// Wrapper around an OpenGL buffer ID
// Since the creation of the OpenGL buffer is delayed until the first call to uploadGL()
// It is possible to create an instance of this class without a valid OpenGL context
// The OpenGL buffer is automatically created with the first call to uploadGL
template<typename T>
class GLBuffer{
public:
    GLuint glBufferId;
    // N of elements of type T stored inside OpenGL buffer.
    int count=0;
    bool alreadyCreatedGLBuffer=false;
    bool alreadyUploaded=false;
    GLBuffer()=default;
    // An OpenGL Buffer is not Copy-constructable, since the intention doing so could be either of 2 following:
    // a) create a new OpenGL buffer, then duplicate the data from the source GL buffer
    // b) just copy the OpenGL buffer id - now the same OpenGL buffer id is duplicated, which leads to error-prone code
    GLBuffer(const GLBuffer&)=delete;
    // Moving an OpenGL buffer is no problem - The new OpenGL buffer just becomes the old one
    GLBuffer(GLBuffer&&)=default;
private:
    const std::string getTAG()const{
        return "GLBuffer"+std::to_string(glBufferId);
    }
    // We have to 'delay' the creation of the buffer until we have a OpenGL context
    // Do nothing if buffer was already created
    void createGLBufferIfNeeded(){
        if(alreadyCreatedGLBuffer)
            return;
        glGenBuffers(1,&glBufferId);
        alreadyCreatedGLBuffer=true;
        GLHelper::checkGlError(getTAG()+"::createGL");
    }
    // Calling uploadGL on the same OpenGL buffer multiple times is not a bug
    // but might be an accident. Log debug message in this case;
    void checkSetAlreadyUploaded(){
        if(alreadyUploaded){
            MLOGD2(getTAG())<<"uploadGL called twice";
        }
        alreadyUploaded=true;
    }
public:
    void uploadGL(const std::vector<T> &vertices){
        createGLBufferIfNeeded();
        checkSetAlreadyUploaded();
        count = GLBufferHelper::uploadGLBuffer(glBufferId, vertices);
        GLHelper::checkGlError(getTAG()+"uploadGL");
        //MDebug::log("N vertices is "+std::to_string(nVertices));
    }
    template<size_t s>
    void uploadGL(const std::array<T,s> &vertices){
        const auto tmp=std::vector<T>(vertices.begin(),vertices.end());
        GLBuffer::uploadGL(tmp);
    }
    void deleteGL() {
        glDeleteBuffers(1, &glBufferId);
    }
};

#endif //FPV_VR_OS_GLBUFFER_HPP
