///////////////////////////////////////////////////////////////////////////////
// Sphere.h
// ========
// Sphere for OpenGL with (radius, sectors, stacks)
// The min number of sectors is 3 and The min number of stacks are 2.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2017-11-01
// UPDATED: 2018-12-13
///////////////////////////////////////////////////////////////////////////////

#ifndef GEOMETRY_SPHERE_H
#define GEOMETRY_SPHERE_H
#include <vector>

#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <GLES2/gl2.h>
#include <GLPrograms/GLProgramTexture.h>


class Sphere
{
public:
    //We use a different layout than GvrSphere
    struct Vertex{
        float x,y,z;
        float u,v;
    };
private:
    std::vector<Vertex> vertexData;
public:
    Sphere(float radius=1.0f, int sectorCount=36, int stackCount=18);
    unsigned int getVertexCount() const      { return vertexData.size(); }
    unsigned int getVertexDataSizeBytes()const {return vertexData.size()*sizeof(Vertex);}
    const void* getVertexData()const { return vertexData.data(); }
public:
    const float radius;
    const int sectorCount;                        // longitude,
    const int stackCount;                         // latitude
public:
    void uploadToGPU(GLuint glBuffVertices)const;


    static std::vector<GLProgramTexture::Vertex> createGvrSphere(float radius,int latitudes,int longitudes);;
};

#endif
