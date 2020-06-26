//
// Created by geier on 16/04/2020.
//

#ifndef RENDERINGX_VERTEXBUFFER_H
#define RENDERINGX_VERTEXBUFFER_H

#include <GLHelper.hpp>
#include <GLBuffer.hpp>
//Helps Generalizing the following re-occuring pattern:
//1) create vertices on stack
//2) upload vertex data to gpu, then object on stack is freed
//3) render all vertices. The nVertices has to be stored,too since it gets lost when object on stack is deleted

class VertexBuffer{
public:
    GLuint vertexB;
    int nVertices=0;
    GLenum mMode=GL_TRIANGLES;
    bool alreadyCreatedGLBuffer=false;
    bool alreadyUploaded=false;
private:
    std::string getTAG(){
        return "VertexBuffer"+std::to_string(vertexB);
    }
    // We have to 'delay' the creation of the buffer until we have a OpenGL context
    void createGLBufferIfNeeded(){
        if(alreadyCreatedGLBuffer)
            return;
        glGenBuffers(1,&vertexB);
        alreadyCreatedGLBuffer=true;
        GLHelper::checkGlError(getTAG()+"::createGL");
    }
    void checkSetAlreadyUploaded(){
        if(alreadyUploaded){
            MLOGD2(getTAG())<<"uploadGL called twice";
        }
        alreadyUploaded=true;
    }
public:
    template<typename T>
    void uploadGL(const std::vector<T> &vertices,GLenum mode=GL_TRIANGLES){
        createGLBufferIfNeeded();
        checkSetAlreadyUploaded();
        nVertices = GLBufferHelper::uploadGLBuffer(vertexB, vertices);
        mMode=mode;
        GLHelper::checkGlError(getTAG()+"uploadGL");
        //MDebug::log("N vertices is "+std::to_string(nVertices));
    }
    template<typename T,size_t s>
    void uploadGL(const std::array<T,s> &vertices,GLenum mode=GL_TRIANGLES){
        createGLBufferIfNeeded();
        checkSetAlreadyUploaded();
        const auto tmp=std::vector<T>(vertices.begin(),vertices.end());
        VertexBuffer::uploadGL(tmp,mode);
    }
    /*template<typename T>
    void initializeAndUploadGL(const std::vector<T> &vertices,GLenum mode=GL_TRIANGLES){
        initializeGL();
        uploadGL(vertices,mode);
    }*/
    void deleteGL() {
        glDeleteBuffers(1, &vertexB);
    }
};

#endif //RENDERINGX_VERTEXBUFFER_H
