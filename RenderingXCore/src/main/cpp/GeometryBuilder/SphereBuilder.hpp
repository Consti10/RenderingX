//
// Created by geier on 11/02/2020.
//

#ifndef RENDERINGX_SPHEREBUILDER_H
#define RENDERINGX_SPHEREBUILDER_H

#include <UvSphere.h>
#include "../GLPrograms/GLProgramTexture.h"

class SphereBuilder{
public:
    //
    //map equirect to insta360, same function as in fragment shader of GLProgramTexture with mapping enabled
    //
    static std::array<float,2> equirect_to_insta360(const float u_in, const float v_in){
        float pi = 3.14159265359f;
        float pi_2 = 1.57079632679f;
        float y2;
        if (v_in < 0.5){
            y2 = 2.0 * v_in;
        } else {
            y2 = 2.0 * (1.0 - v_in);
        }
        float sectorAngle = 2.0 * pi * u_in;
        float nx = y2 * cos(sectorAngle);
        float ny = y2 * sin(sectorAngle);
        float scale = 0.93;
        //LOGD("nx %f ny %f",nx,ny);

        float u = -nx * scale / 4.0 + 0.25;
        float v = -ny * scale / 2.0 + 0.5;
        if (v_in > 0.5) {
            u = 1.0 - u;
        }
        //u=std::clamp(u,0.0f,1.0f);
        //v=std::clamp(v,0.0f,1.0f);
        return {u,v};
    }

    //
    //And here is the binding for GLProgramTexture::Vertex
    //
    static std::vector<GLProgramTexture::Vertex>
    createSphereEquirectangularMonoscopic(float radius=1.0f, int latitudes=72, int longitudes=18) {
        const auto vertexDataAsInGvr=UvSphere::createUvSphere(radius,latitudes,longitudes,180,360,UvSphere::MEDIA_MONOSCOPIC);
        std::vector<GLProgramTexture::Vertex> ret;
        for(const auto& vertex:vertexDataAsInGvr){
            GLProgramTexture::Vertex v{
                    vertex.x,vertex.y,vertex.z,vertex.u_left,vertex.v_left
            };
            ret.push_back(v);
        }
        return ret;
    }

    //Use the map function to convert from equirect to dual fisheye insta360 - TODO fix 'black line'
    static std::vector<GLProgramTexture::Vertex>
    createSphereDualFisheyeInsta360() {
        float radius=1.0f;
        float latitudes=128;
        float longitudes=36;
        const auto vertexDataAsInGvr=UvSphere::createUvSphere(radius,latitudes,longitudes,180,360,UvSphere::MEDIA_MONOSCOPIC);
        std::vector<GLProgramTexture::Vertex> ret;
        for(const auto& vertex:vertexDataAsInGvr){
            const auto d=equirect_to_insta360(vertex.u_left,vertex.v_left);
            GLProgramTexture::Vertex v{
                    vertex.x,vertex.y,vertex.z,d[0],d[1]
            };
            ret.push_back(v);
        }
        return ret;
    }
};

#endif //RENDERINGX_SPHEREBUILDER_H
