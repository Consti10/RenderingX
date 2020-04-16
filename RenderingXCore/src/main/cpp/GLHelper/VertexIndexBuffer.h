//
// Created by geier on 16/04/2020.
//

#ifndef RENDERINGX_VERTEXINDEXBUFFER_H
#define RENDERINGX_VERTEXINDEXBUFFER_H

#include <GLBufferHelper.hpp>

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

#endif //RENDERINGX_VERTEXINDEXBUFFER_H
