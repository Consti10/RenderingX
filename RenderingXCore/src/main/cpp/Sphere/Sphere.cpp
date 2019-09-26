///////////////////////////////////////////////////////////////////////////////
// Sphere.cpp
// ==========
// Sphere for OpenGL with (radius, sectors, stacks)
// The min number of sectors is 3 and the min number of stacks are 2.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2017-11-01
// UPDATED: 2018-12-13
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <cmath>
#include "Sphere.h"
#include "GvrSphere.h"
#include <jni.h>
#include <sstream>
#include <android/log.h>

constexpr auto TAG="Sphere";
#define LOGDX(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// constants //////////////////////////////////////////////////////////////////
const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT  = 2;
//constexpr float PI = 3.1415926F;
constexpr double PI = 3.141592653589793;

static glm::vec2 map_equirectangular(float u,float v,bool secondHalf);


static std::vector<Sphere::Vertex> createUvSphereInsta360(float radius,int latitudes,int longitudes){
    const std::vector<float> vertexDataAsInGvr=GvrSphere::createUvSphere(radius,latitudes,longitudes,180,360,GvrSphere::MEDIA_MONOSCOPIC);
    std::vector<Sphere::Vertex> vertexDataInsta360={};
    //LOGDX("vertexDataAsInGvr.size()%d",vertexDataAsInGvr.size());
    const float scale=0.9f;
    for(int i=0;i<vertexDataAsInGvr.size()-GvrSphere::CPV;i+=GvrSphere::CPV){
        Sphere::Vertex vertex;
        //Vertex Data-Sphere
        vertex.x=vertexDataAsInGvr.at(i);
        vertex.y=vertexDataAsInGvr.at(i+1);
        vertex.z=vertexDataAsInGvr.at(i+2);
        //Texture Data - Insta360, it only has data for one eye
        //glm::vec2 tmp=map_equirectangular(vertexDataAsInGvr.at(i+3),vertexDataAsInGvr.at(i+4),false);
        //vertex.u=tmp.x;
        //vertex.v=tmp.y;
        vertex.u=(vertexDataAsInGvr.at(i+3)-0.5f)*scale+0.5f;
        vertex.v=(vertexDataAsInGvr.at(i+4)-0.25f)*scale+0.25f;
        //vertex.u=vertexDataAsInGvr.at(i+3)*scale;
        //vertex.v=vertexDataAsInGvr.at(i+4)*scale;
        //vertex.u2=vertexDataAsInGvr.at(i+5);
        //vertex.v2=vertexDataAsInGvr.at(i+6);
        vertexDataInsta360.push_back(vertex);
    }
    //LOGDX("vertexDataInsta360.size()%d",vertexDataInsta360.size());
    return vertexDataInsta360;
}



Sphere::Sphere(float radius, int sectors, int stacks):
radius(radius),
sectorCount(sectors),
stackCount(stacks)
{
    if(sectors<MIN_SECTOR_COUNT || stacks<MIN_STACK_COUNT){
        __android_log_print(ANDROID_LOG_DEBUG,"Sphere","Error! %d %d",sectors,stacks);
    }
    if(sectors % 2 !=0 || stacks % 2 !=0){
        __android_log_print(ANDROID_LOG_DEBUG,"Sphere","Error! %d %d",sectors,stacks);
    }
    //buildVertices();
    //buildVertices2HalfSpheres();

    //vertexData=createUvSphere(1,12,24,180,360,MEDIA_MONOSCOPIC);
    vertexData=createUvSphereInsta360(radius,12,24);
}


static glm::vec2 map_equirectangular(float u,float v,bool secondHalf){
    //if(u>0.5f){
        //u=1.0-0.5f;
    //    u=0.5f;
    //}
    /*if(secondHalf){
        if(u==0.5f){
            u=0.50001f;
        }
    }*/
    float pi = 3.14159265359;
    float pi_2 = 1.57079632679;
    float xy;
    if (v < 0.5){
        xy = 2.0 * v;
    } else {
        xy = 2.0 * (1.0 - v);
    }
    float sectorAngle = 2.0 * pi * u;
    float nx = xy * sin(sectorAngle);
    float ny = xy * cos(sectorAngle);
    //float scale = 0.93;
    float scale = 0.93f;
    float t = -ny * scale / 2.0 + 0.5;
    float s = -nx * scale / 4.0 + 0.25;
    if (v > 0.5) {
        s = 1.0 - s;
    }
    __android_log_print(ANDROID_LOG_DEBUG,"u,v","%f %f",s,t);

    return glm::vec2(s,t);
}


void Sphere::uploadToGPU(GLuint glBuffVertices) const{
/*glBindBuffer(GL_ARRAY_BUFFER, glBuffVertices);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sphere.getInterleavedVertexSize(), sphere.getInterleavedVertices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)sphere.getIndexSize(), sphere.getIndices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/

    glBindBuffer(GL_ARRAY_BUFFER,glBuffVertices);
    glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)getVertexDataSizeBytes(),getVertexData(),GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    //LOGD("mSphere.getIndexSize():%d",mSphere.getIndexSize());
    //LOGD("mSphere.getIndexCount():%d",mSphere.getIndexCount());
    //LOGD("mSphere.getInterleavedVertexSize():%d",mSphere.getInterleavedVertexSize());
    //LOGD("mSphere.getInterleavedVertexCount():%d",mSphere.getInterleavedVertexCount());
}

