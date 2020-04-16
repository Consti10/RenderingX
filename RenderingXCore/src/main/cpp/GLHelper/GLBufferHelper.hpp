
#ifndef RENDERINGX_CORE_HELPER_GLBufferHelper
#define RENDERINGX_CORE_HELPER_GLBufferHelper

#include <GLES2/gl2.h>
#include <vector>
#include <array>
#include <iterator>

// Provides convenient functions to upload cpp
// data types to GPU using OpenGL c style api
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
    //Also create the GL Buffer, both for array and vector
    template<class T>
    static int createUploadGLBuffer(GLuint &buff, const std::vector<T> &data,GLenum usage=GL_STATIC_DRAW) {
        glGenBuffers(1, &buff);
        return uploadGLBuffer(buff,data,usage);
    }
    template<class T,std::size_t S>
    static int createUploadGLBuffer(GLuint &buff, const std::array<T,S> &data,GLenum usage=GL_STATIC_DRAW) {
        glGenBuffers(1,&buff);
        return uploadGLBuffer(buff,data,usage);
    }
};

#endif //RENDERINGX_CORE_HELPER_GLBufferHelper