//
// Created by geier on 16/04/2020.
//

#ifndef RENDERINGX_VERTEXBUFFER_H
#define RENDERINGX_VERTEXBUFFER_H

#include <GLHelper.hpp>
#include <GLBufferHelper.hpp>

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

#endif //RENDERINGX_VERTEXBUFFER_H
