//
// Created by Consti10 on 24/09/2019.
//

#include "GvrSphere.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <jni.h>
#include <sstream>
#include <android/log.h>

constexpr auto TAG="GvrSphere";
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

constexpr double PI = 3.141592653589793;

//equivalent of (java) Math.toRadians()
static double Math_toRadians(double degrees){
    return (degrees * PI) / 180.0;
}

//equivalent of (java) System.arraycopy()
static void System_arraycopy(const float *source, int sourceOffset, float *dest, int destOffset, int size){
    for(int i=0;i<size;i++){
        dest[destOffset+i]=source[sourceOffset+i];
    }
}

std::vector<float> GvrSphere::createUvSphere(
        float radius,
        int latitudes,
        int longitudes,
        float verticalFovDegrees,
        float horizontalFovDegrees,
        int mediaFormat) {
    if (radius <= 0
        || latitudes < 1 || longitudes < 1
        || verticalFovDegrees <= 0 || verticalFovDegrees > 180
        || horizontalFovDegrees <= 0 || horizontalFovDegrees > 360) {
        //throw new IllegalArgumentException("Invalid parameters for sphere.");
    }
    //System.out.println("Params:"+radius+","+latitudes+","+longitudes+","+verticalFovDegrees+","+horizontalFovDegrees+","+mediaFormat);
    //LOGDX("Params:%f,%d,%d,%f,%f,%d",radius,latitudes,longitudes,verticalFovDegrees,horizontalFovDegrees,mediaFormat);

    // Compute angular size in radians of each UV quad.
    float verticalFovRads = (float) Math_toRadians(verticalFovDegrees);
    float horizontalFovRads = (float) Math_toRadians(horizontalFovDegrees);
    float quadHeightRads = verticalFovRads / latitudes;
    float quadWidthRads = horizontalFovRads / longitudes;

    // Each latitude strip has 2 * (longitudes quads + extra edge) vertices + 2 degenerate vertices.
    int vertexCount = (2 * (longitudes + 1) + 2) * latitudes;
    // Buffer to return.
    const int vertexDataSize=vertexCount * CPV;
    //float[] vertexData = new float[vertexCount * CPV];
    float vertexData[vertexDataSize];

    // Generate the data for the sphere which is a set of triangle strips representing each
    // latitude band.
    int v = 0; // Index into the vertex array.
    // (i, j) represents a quad in the equirectangular sphere.
    for (int j = 0; j < latitudes; ++j) { // For each horizontal triangle strip.
        // Each latitude band lies between the two phi values. Each vertical edge on a band lies on
        // a theta value.
        float phiLow = (quadHeightRads * j - verticalFovRads / 2);
        float phiHigh = (quadHeightRads * (j + 1) - verticalFovRads / 2);

        for (int i = 0; i < longitudes + 1; ++i) { // For each vertical edge in the band.
            for (int k = 0; k < 2; ++k) { // For low and high points on an edge.
                // For each point, determine it's position in polar coordinates.
                float phi = (k == 0) ? phiLow : phiHigh;
                float theta = quadWidthRads * i + (float) PI - horizontalFovRads / 2;

                // Set vertex position data as Cartesian coordinates.
                vertexData[CPV * v + 0] = -(float) (radius * sin(theta) * cos(phi));
                vertexData[CPV * v + 1] =  (float) (radius * sin(phi));
                vertexData[CPV * v + 2] =  (float) (radius * cos(theta) * cos(phi));

                // Set vertex texture.x data.
                if (mediaFormat == MEDIA_STEREO_LEFT_RIGHT) {
                    // For left-right media, each eye's x coordinate points to the left or right half of the
                    // texture.
                    vertexData[CPV * v + 3] = (i * quadWidthRads / horizontalFovRads) / 2;
                    vertexData[CPV * v + 5] = (i * quadWidthRads / horizontalFovRads) / 2 + .5f;
                } else {
                    // For top-bottom or monoscopic media, the eye's x spans the full width of the texture.
                    vertexData[CPV * v + 3] = i * quadWidthRads / horizontalFovRads;
                    vertexData[CPV * v + 5] = i * quadWidthRads / horizontalFovRads;
                }

                // Set vertex texture.y data. The "1 - ..." is due to Canvas vs GL coords.
                if (mediaFormat == MEDIA_STEREO_TOP_BOTTOM) {
                    // For top-bottom media, each eye's y coordinate points to the top or bottom half of the
                    // texture.
                    vertexData[CPV * v + 4] = 1 - (((j + k) * quadHeightRads / verticalFovRads) / 2 + .5f);
                    vertexData[CPV * v + 6] = 1 - ((j + k) * quadHeightRads / verticalFovRads) / 2;
                } else {
                    // For left-right or monoscopic media, the eye's y spans the full height of the texture.
                    vertexData[CPV * v + 4] = 1 - (j + k) * quadHeightRads / verticalFovRads;
                    vertexData[CPV * v + 6] = 1 - (j + k) * quadHeightRads / verticalFovRads;
                }
                v++;

                // Break up the triangle strip with degenerate vertices by copying first and last points.
                if ((i == 0 && k == 0) || (i == longitudes && k == 1)) {
                    System_arraycopy(vertexData, CPV * (v - 1), vertexData, CPV * v, CPV);
                    v++;
                }
            }
            // Move on to the next vertical edge in the triangle strip.
        }
        // Move on to the next triangle strip.
    }

    //return new Mesh(vertexData);
    std::vector<float> ret={};
    for(int i=0;i<vertexDataSize;i++){
        ret.push_back(vertexData[i]);
    }
    /*std::stringstream ss;
    ss<<std::fixed;
    int line=0;
    for(int i=0;i<ret.size();i++){
        ss<<ret.at(i)<<",";
        line++;
        if(line>10){
            ss<<"\n";
            line=0;
        }
    }
    LOGDX("Data:[%s]",ss.str().c_str());
    LOGDX("Vertex data size%d",vertexDataSize);*/
    return ret;
}

std::vector<GvrSphere::Vertex>
GvrSphere::createUvSphere2(float radius, int latitudes, int longitudes, float verticalFovDegrees,
                           float horizontalFovDegrees, int mediaFormat) {
    const std::vector<float> tmp=createUvSphere(radius,latitudes,longitudes,verticalFovDegrees,horizontalFovDegrees,mediaFormat);
    std::vector<GvrSphere::Vertex> ret;
    for(unsigned int i=0;i<=tmp.size()-GvrSphere::CPV;i+=GvrSphere::CPV){
        const GvrSphere::Vertex vertex{
                tmp.at(i),
                tmp.at(i+1),
                tmp.at(i+2),
                tmp.at(i+3),
                tmp.at(i+4),
                tmp.at(i+5),
                tmp.at(i+6)
        };
        ret.push_back(vertex);
    }
    return ret;
}

