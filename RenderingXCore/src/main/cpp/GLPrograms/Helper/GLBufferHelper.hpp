
#ifndef RENDERINGX_CORE_HELPER_GLBufferHelper
#define RENDERINGX_CORE_HELPER_GLBufferHelper

#include <GLES2/gl2.h>
#include "../GeometryBuilder/TexturedGeometry.hpp"

class GLBufferHelper{
public:
    static void allocateGLBufferStatic(GLuint buff,void* array,int arraySize){
        glBindBuffer(GL_ARRAY_BUFFER,buff);
        glBufferData(GL_ARRAY_BUFFER, arraySize,
                     array, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    static void allocateGLBufferDynamic(GLuint buff,void* array,int arraySize){
        glBindBuffer(GL_ARRAY_BUFFER,buff);
        glBufferData(GL_ARRAY_BUFFER, arraySize,
                     array, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    template<typename T> static void allocateGLBufferStatic(const GLuint buff,const std::vector<T>& data){
        const int size=data.size()*sizeof(T);
        allocateGLBufferStatic(buff,(void*)data.data(),size);
    }
    template<typename T> static void allocateGLBufferDynamic(const GLuint buff,const std::vector<T>& data){
        const int size=data.size()*sizeof(T);
        allocateGLBufferDynamic(buff,(void*)data.data(),size);
    }
    static void allocateGLBufferStatic(const GLuint vertexB,const GLuint indexB,const TexturedGeometry::IndicesVertices& data){
        allocateGLBufferStatic(vertexB,data.vertices);
        allocateGLBufferStatic(indexB,data.indices);
    }
};

#endif //RENDERINGX_CORE_HELPER_GLBufferHelper