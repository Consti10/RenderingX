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
#include <jni.h>
#include <sstream>
#include <android/log.h>

// constants //////////////////////////////////////////////////////////////////
const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT  = 2;

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
Sphere::Sphere(float radius, int sectors, int stacks):
radius(radius),sectorCount(sectors),stackCount(stacks)
{
    if(sectors<MIN_SECTOR_COUNT || stacks<MIN_STACK_COUNT){
        __android_log_print(ANDROID_LOG_DEBUG,"Sphere","Error! %d %d",sectors,stacks);
    }
    buildVerticesSmooth();
}

///////////////////////////////////////////////////////////////////////////////
// print itself
///////////////////////////////////////////////////////////////////////////////
void Sphere::printSelf() const
{
    std::stringstream ss;
    ss<< "===== Sphere =====\n"
              << "        Radius: " << radius << "\n"
              << "  Sector Count: " << sectorCount << "\n"
              << "   Stack Count: " << stackCount << "\n"
              << "Smooth Shading: " << (smooth ? "true" : "false") << "\n"
              << "   Index Count: " << getIndexCount() << "\n"
              << "   Index Size: " << getIndexSize() << "\n"
              <<std::endl;
     __android_log_print(ANDROID_LOG_DEBUG,"Sphere","%s",ss.str().c_str());
}

static glm::vec2 map_equirectangular(float u,float v){
    //if(u>0.5f){
        //u=1.0-0.5f;
    //    u=0.5f;
    //}
    float pi = 3.14159265359;
    float pi_2 = 1.57079632679;
    float xy;
    if (v < 0.5){
        xy = 2.0 * v;
    } else {
        xy = 2.0 * (1.0 - v);
    }
    float sectorAngle = 2.0 * pi * u;
    float nx = xy * cos(sectorAngle);
    float ny = xy * sin(sectorAngle);
    //float scale = 0.93;
    float scale = 1.0f;
    float t = -ny * scale / 2.0 + 0.5;
    float s = -nx * scale / 4.0 + 0.25;
    if (v > 0.5) {
        s = 1.0 - s;
    }
    __android_log_print(ANDROID_LOG_DEBUG,"u,v","%f %f",s,t);

    return glm::vec2(s,t);
}


///////////////////////////////////////////////////////////////////////////////
// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void Sphere::buildVerticesSmooth()
{
    const float PI = 3.1415926f;
    std::cerr << "aaa\n";

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float s, t;                                     // texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            //The new vertex to add
            Vertex vertex;

            // vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertex.x=x;
            vertex.y=y;
            vertex.z=z;

            // normalized vertex normal !we don't need normals, but I'm leaving it here!
            //nx = x * lengthInv;
            //ny = y * lengthInv;
            //nz = z * lengthInv;
            //addNormal(nx, ny, nz);

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            glm::vec2 st=map_equirectangular(s,t);
            s=st.x;
            t=st.y;
            vertex.u=s;
            vertex.v=t;

            interleavedVertices.push_back(vertex);
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if(i != 0)
            {
                addIndices(k1, k2, k1+1);   // k1---k2---k1+1
            }

            if(i != (stackCount-1))
            {
                addIndices(k1+1, k2, k2+1); // k1+1---k2---k2+1
            }

            // vertical lines for all stacks
            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if(i != 0)  // horizontal lines except 1st stack
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }
}



///////////////////////////////////////////////////////////////////////////////
// add 3 indices to array
///////////////////////////////////////////////////////////////////////////////
void Sphere::addIndices(unsigned int i1, unsigned int i2, unsigned int i3)
{
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}

