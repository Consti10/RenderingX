//
// Created by geier on 11/02/2020.
//

#ifndef RENDERINGX_SPHEREBUILDER_H
#define RENDERINGX_SPHEREBUILDER_H

#include <Sphere/UvSphere.hpp>
#include <Sphere/DualFisheyeSphere.hpp>
#include "GLProgramTexture.h"
#include "../TexturedGeometry.hpp"

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
        return {1.0f -u,v};
    }

    //
    //map equirect to a single fisheye image.
    //
    static std::array<float,2> equirect_to_fisheye(const float u_in, const float v_in, // output coordinates
						   const float radiusx, const float radiusy, const float fov,
						   const float x_shift, const float y_shift){
        float pi = 3.14159265359f;
        float pi_2 = 1.57079632679f;

	// Convert outcoords to radians on the sphere.
	// Center the frame on 180 degrees and adjust the angles to account for the FOV of the lens
	float theta = (2.0 * u_in * pi - pi_2);
	float phi = (v_in * pi - pi_2);
  
	// Convert outcoords to coordinates on a unit sphere
	float x = sin(theta) * cos(phi);
	float y = cos(theta) * cos(phi);
	float z = -sin(phi);

	// Convert to spherical with respect to the lens
	float theta2 = atan2(y, x);
	float adj = sqrt(x * x + y * y);
	float phi2 = atan2(adj, z);
	float radius = 180.0 * phi2 / (M_PI * fov);
	float u = 0;
	float v = 0;
	if (radius <= 0.5) {
	  u = (radius * sin(theta2) * radiusx + 0.5);
	  v = (radius * cos(theta2) * radiusy + 0.5);
	}
	return {u,v};
   }

    //
    //And here is the binding for GLProgramTexture
    //
    static TexturedMeshData
    createSphereEquirectangularMonoscopic(float radius=1.0f, int latitudes=64, int longitudes=32) {
        const auto vertexDataAsInGvr=UvSphere::createUvSphere(radius,latitudes,longitudes,180,360,UvSphere::MEDIA_EQUIRECT_MONOSCOPIC,UvSphere::ROTATE_UNKNOWN);
        std::vector<GLProgramTexture::Vertex> ret;
        for(const auto& vertex:vertexDataAsInGvr){
            GLProgramTexture::Vertex v{
                    vertex.x,vertex.y,vertex.z,vertex.u_left,vertex.v_left
            };
            ret.push_back(v);
        }
        return TexturedMeshData(ret, GL_TRIANGLE_STRIP);
    }

    //Use the map function to convert from equirect to dual fisheye insta360 - TODO fix 'black line'
    static TexturedMeshData
    createSphereDualFisheyeInsta360(UvSphere::ROTATION rot=UvSphere::ROTATION::ROTATE_UNKNOWN) {
        float radius=1.0f;
        float latitudes=128;
        float longitudes=36;
        const auto vertexDataAsInGvr=UvSphere::createUvSphere(radius,latitudes,longitudes,180,360,UvSphere::MEDIA_EQUIRECT_MONOSCOPIC,rot);
        std::vector<GLProgramTexture::Vertex> ret;
        for(const auto& vertex:vertexDataAsInGvr){
            const auto d=equirect_to_insta360(vertex.u_left,vertex.v_left);
            GLProgramTexture::Vertex v{
                    vertex.x,vertex.y,vertex.z,d[0],d[1]
            };
            ret.push_back(v);
        }
        return TexturedMeshData(ret, GL_TRIANGLE_STRIP);
    }

    //Use the map function to convert from equirect to fisheye
    static TexturedMeshData
    createSphereFisheye(UvSphere::ROTATION rot,
			const float radiusx, const float radiusy, const float fov,
			const float x_shift, const float y_shift){
        float radius=1.0f;
        float latitudes=128;
        float longitudes=36;
        const auto vertexDataAsInGvr=UvSphere::createUvSphere(radius,latitudes,longitudes,180,360,UvSphere::MEDIA_EQUIRECT_MONOSCOPIC,rot);
        std::vector<GLProgramTexture::Vertex> ret;
        for(const auto& vertex:vertexDataAsInGvr){
            const auto d=equirect_to_fisheye(vertex.u_left,vertex.v_left,radiusx,radiusy,fov,x_shift,y_shift);
            GLProgramTexture::Vertex v{
                    vertex.x,vertex.y,vertex.z,d[0],d[1]
            };
            ret.push_back(v);
        }
        return TexturedMeshData(ret, GL_TRIANGLE_STRIP);
    }
};

#endif //RENDERINGX_SPHEREBUILDER_H
