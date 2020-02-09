//
// Created by Constantin on 1/10/2020.
// Copied from Brian Webb's original source code in ogl.cc ( OpenHD )
//

#ifndef RENDERINGX_EQUIRECTANGULARSPHERE_H
#define RENDERINGX_EQUIRECTANGULARSPHERE_H

#include <array>
#include <vector>
#include <GLES2/gl2.h>
#include "cmath"
#include <GLBufferHelper.hpp>
#include <GLProgramTexture.h>

class EquirectangularSphere {
public:
    // Adjust this to fit the FOV of the lens on the 360 camera.
    // Smaller values indicate a larger FOV and more overlap between the two sides.
    static constexpr const float radius_scale = 0.44f;
    // Larger number means more triangles
    static constexpr const float triangle_density = 1000;

    static void create_sphere(std::vector<GLfloat>& vertexData,std::vector<GLuint>& indexData,int surf_w,int surf_h){
        uint32_t cam1_cx = 1.0 * surf_w / 4;
        uint32_t cam1_cy = surf_h / 2;
        uint32_t cam2_cx = 1.0 * 3 * surf_w / 4;
        uint32_t cam2_cy = surf_h / 2;
        uint32_t radius = static_cast<uint32_t>(rint(radius_scale * surf_h));
        uint32_t triangle_size = static_cast<uint32_t>(rint(surf_h / triangle_density));
        create_rect(vertexData, indexData, surf_w, surf_h, radius, cam1_cx, cam1_cy, cam2_cx, cam2_cy,
                    triangle_size);
    }

    static std::array<float,2> map_equirectangular(const float x,const float y){
        float pi = 3.14159265359;
        float pi_2 = 1.57079632679;
        float xy;
        if (y < 0.5){
            xy = 2.0 * y;
        } else {
            xy = 2.0 * (1.0 - y);
        }
        float sectorAngle = 2.0 * pi * x;
        float nx = xy * cos(sectorAngle);
        float ny = xy * sin(sectorAngle);
        float scale = 0.93;

        float t = -ny * scale / 2.0 + 0.5;
        float s = -nx * scale / 4.0 + 0.25;
        if (y > 0.5) {
            s = 1.0 - s;
        }
        return {s,t};
    }

    static void create_rect(std::vector<GLfloat> &verts, std::vector<GLuint> &indexes,
                     uint32_t width, uint32_t height, uint32_t radius, // In pixels
                     uint32_t c1x, uint32_t c1y, uint32_t c2x, uint32_t c2y, // In pixels
                     uint32_t trisize) { // In pixels
        uint32_t nx = static_cast<uint32_t>(std::ceil(static_cast<float>(width) / trisize));
        uint32_t ny = static_cast<uint32_t>(std::ceil(static_cast<float>(height) / trisize));
        uint32_t dx = trisize;
        uint32_t dy = trisize;

        // Create the vertex array
        for (uint32_t y = 0; y <= ny; ++y) {
            uint32_t cy = y * dy;
            for (uint32_t x = 0; x <= nx; ++x) {
                uint32_t cx = x * dx;
                add_vert(verts, width, height, radius, c1x, c1y, c2x, c2y, cx, cy);
            }
        }

        // Create the triangles
        uint32_t xinc = nx + 1;
        uint32_t yinc = ny + 1;
        for (uint32_t y = 0; y < ny; ++y) {
            // Place the starting point of the row
            indexes.push_back(y * xinc);
            for (uint32_t x = 0; x < nx; ++x) {
                // Place the next triangle diagonal
                indexes.push_back((y + 1) * xinc + x);
                indexes.push_back(y * xinc + (x + 1));
            }
            // Finish off this row with a pair of degenerate triangles.
            indexes.push_back((y + 1) * xinc + nx);
            indexes.push_back((y + 1) * xinc + nx);
            indexes.push_back((y + 1) * xinc);
        }
    }
    static void add_vert(std::vector<GLfloat> &verts,
                         uint32_t width, uint32_t height, uint32_t radius,
                         uint32_t c1x, uint32_t c1y, uint32_t c2x, uint32_t c2y,
                         uint32_t x, uint32_t y) {
        float r2 = static_cast<float>(radius) * static_cast<float>(radius);
        float fx = static_cast<float>(std::min(x, width - 1)) / static_cast<float>(width);
        float fy = static_cast<float>(std::min(y, height - 1)) / static_cast<float>(height);
        float fz = 0;
        float u = fx;
        float v = 1.0 - fy;

        // Calculate the Z value from the X,Y values to make a sphere
        // The right side of the frame should also be folded over to make
        // the back side of the sphere.
        if (x < width / 2) {
            float dx = static_cast<float>(x) - static_cast<float>(c1x);
            float dy = static_cast<float>(y) - static_cast<float>(c1y);
            float d2 = dx * dx + dy * dy;
            fz = (d2 > r2) ? 0.0001 : 0.5 * sqrt(r2 - d2) / static_cast<float>(height / 2);
            fx = 2.0 * fx;
        } else {
            float dx = static_cast<float>(x) - static_cast<float>(c2x);
            float dy = static_cast<float>(y) - static_cast<float>(c2y);
            float d2 = dx * dx + dy * dy;
            fz = (d2 > r2) ? -0.0001 : 0.5 * -sqrt(r2 - d2) / static_cast<float>(height / 2);
            fx = 2.0 * (1.0 - fx);
        }

        // The coordinates should range from -0.5 to 0.5.
        verts.push_back(fx - 0.5);
        verts.push_back(fy - 0.5);
        verts.push_back(fz);
        verts.push_back(1.0f-u);
        verts.push_back(v);
    }
public:
    //
    //Convenient conversions from original data layout to RenderingXCore Vertex data
    //
    //GLushort instead of GLuint should be sufficient for index data and is used by GLProgramTexture
    //Also, convert to GLProgramTexture::Vertex
    static void create_sphere(std::vector<GLProgramTexture::Vertex>& vertexData,std::vector<GLProgramTexture::INDEX_DATA >& indexData,int surf_w,int surf_h){
        std::vector<GLfloat> tmpVertices;
        std::vector<GLuint> tmpIndices;
        create_sphere(tmpVertices,tmpIndices,surf_w,surf_h);
        for(int i=0;i<=tmpVertices.size()-5;i+=5){
            GLProgramTexture::Vertex v;
            v.x=tmpVertices[i+0];
            v.y=tmpVertices[i+1];
            v.z=tmpVertices[i+2];
            v.u=tmpVertices[i+3];
            v.v=tmpVertices[i+4];
            vertexData.push_back(v);
        }
        for(auto i=0;i<tmpIndices.size();i++){
            indexData.push_back((GLProgramTexture::INDEX_DATA) tmpIndices.at(i));
        }
    }

    //Takes a !already initialized! VertexIndex Buffer
    static void uploadSphereGL(VertexIndexBuffer& data,int surf_w,int surf_h){
        std::vector<GLProgramTexture::Vertex> vertices;
        std::vector<GLProgramTexture::INDEX_DATA > indices;
        EquirectangularSphere::create_sphere(vertices,indices,surf_w,surf_h);
        /*LOGD("N vertices %d N indices %d",vertices.size(),indices.size());
        for(unsigned int i=0;i<indices.size();i++){
            const int value=indices.at(i);
            //LOGD("Index %d has value %d ",i,value);
            if(value>vertices.size()){
                LOGD("Error value too high");
            }
        }*/
        data.uploadGL(vertices,indices,GL_TRIANGLE_STRIP);
    }

};


#endif //RENDERINGX_EQUIRECTANGULARSPHERE_H
