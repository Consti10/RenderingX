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
    bool alreadyCreatedGLBuffer=false;
    // uploading more than once is not a problem, but probably unintended
    bool alreadyUploaded=false;
private:
    std::string getTAG(){
        return "VertexIndexBuffer"+std::to_string(vertexB)+","+std::to_string(indexB)+"::";
    }
    // We have to 'delay' the creation of the buffer until we have a OpenGL context
    void createGLBufferIfNeeded(){
        if(alreadyCreatedGLBuffer)
            return;
        glGenBuffers(1,&vertexB);
        glGenBuffers(1,&indexB);
        alreadyCreatedGLBuffer=true;
        GLHelper::checkGlError(getTAG()+"createGL");
    }
    void checkSetAlreadyUploaded(){
        if(alreadyUploaded){
            LOG2(getTAG())<<+"uploadGL called twice";
        }
        alreadyUploaded=true;
    }
public:
    template<typename T, typename T2>
    void uploadGL(const std::vector<T> &vertices,const std::vector<T2> &indices,GLenum mode=GL_TRIANGLES){
        createGLBufferIfNeeded();
        checkSetAlreadyUploaded();
        GLBufferHelper::uploadGLBuffer(vertexB, vertices);
        nIndices = GLBufferHelper::uploadGLBuffer(indexB, indices);
        mMode=mode;
        GLHelper::checkGlError(getTAG()+"uploadGL");
    }

    /*template<typename T, typename T2>
    void initializeAndUploadGL(const std::vector<T> &vertices,const std::vector<T2> &indices,GLenum mode=GL_TRIANGLES){
        initializeGL();
        uploadGL(vertices,indices,mode);
    }*/

    void deleteGL() {
        glDeleteBuffers(1, &vertexB);
        glDeleteBuffers(1,&indexB);
    }
};

#endif //RENDERINGX_VERTEXINDEXBUFFER_H
