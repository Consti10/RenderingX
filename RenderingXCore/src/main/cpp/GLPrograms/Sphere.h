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

class Sphere
{
public:
    struct Vertex{
        float x,y,z;
        float u,v;
    };
public:
    // ctor/dtor
    Sphere(float radius=1.0f, int sectorCount=36, int stackCount=18);
    ~Sphere() {}
    //indices
    unsigned int getIndexCount() const      { return (unsigned int)indices.size(); }
    unsigned int getIndexSize() const       { return (unsigned int)indices.size() * sizeof(unsigned int); }
    const unsigned int* getIndices() const  { return indices.data(); }
    //vertices
    unsigned int getInterleavedVertexSize() const   { return interleavedVertices.size() * sizeof(Vertex); }    // # of bytes
    const Vertex* getInterleavedVertices() const     { return interleavedVertices.data(); }
    // debug
    void printSelf() const;
public:
    const float radius;
    const int sectorCount;                        // longitude, # of slices
    const int stackCount;                         // latitude, # of stacks
    const bool smooth=true;
private:
    void buildVerticesSmooth();
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);

    std::vector<unsigned int> indices={};
    std::vector<unsigned int> lineIndices={};
    std::vector<Vertex> interleavedVertices={};

};

#endif
