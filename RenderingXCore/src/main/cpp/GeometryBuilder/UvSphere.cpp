//
// Created by Consti10 on 24/09/2019.
//

#include "UvSphere.h"

#include <cmath>

constexpr auto TAG="GvrSphere";

constexpr double PI = 3.141592653589793;

//equivalent of (java) Math.toRadians()
static double Math_toRadians(double degrees){
    return (degrees * PI) / 180.0;
}

std::vector<UvSphere::Vertex> UvSphere::createUvSphere(
        float radius,
        int latitudes,
        int longitudes,
        float verticalFovDegrees,
        float horizontalFovDegrees,
        const MEDIA_FORMAT mediaFormat,
	const ROTATION rotation) {
    if (radius <= 0
        || latitudes < 1 || longitudes < 1
        || verticalFovDegrees <= 0 || verticalFovDegrees > 180
        || horizontalFovDegrees <= 0 || horizontalFovDegrees > 360) {
        //throw new IllegalArgumentException("Invalid parameters for sphere.");
    }

    // Compute angular size in radians of each UV quad.
    const float verticalFovRads = (float) Math_toRadians(verticalFovDegrees);
    const float horizontalFovRads = (float) Math_toRadians(horizontalFovDegrees);
    const float quadHeightRads = verticalFovRads / latitudes;
    const float quadWidthRads = horizontalFovRads / longitudes;

    // Each latitude strip has 2 * (longitudes quads + extra edge) vertices + 2 degenerate vertices.
    const unsigned int vertexCount = (unsigned)((2 * (longitudes + 1) + 2) * latitudes);
    // Buffer to return.
    std::vector<UvSphere::Vertex> vertexData(vertexCount);

    // Generate the data for the sphere which is a set of triangle strips representing each
    // latitude band.
    int v = 0; // Index into the vertex array.
    // (i, j) represents a quad in the equirectangular sphere.
    for (int j = 0; j < latitudes; ++j) { // For each horizontal triangle strip.
        // Each latitude band lies between the two phi values. Each vertical edge on a band lies on
        // a theta value.
        const float phiLow = (quadHeightRads * j - verticalFovRads / 2);
        const float phiHigh = (quadHeightRads * (j + 1) - verticalFovRads / 2);

        for (int i = 0; i < longitudes + 1; ++i) { // For each vertical edge in the band.
            for (int k = 0; k < 2; ++k) { // For low and high points on an edge.
                // For each point, determine it's position in polar coordinates.
                const float phi = (k == 0) ? phiLow : phiHigh;
                const float theta = quadWidthRads * i + (float) PI - horizontalFovRads / 2;

                // Set vertex position data as Cartesian coordinates.
                switch (rotation) {
                    case ROTATE_0:
                        vertexData[v].x = (float) (radius * std::cos(theta) * std::cos(phi));
                        vertexData[v].y = (float) (radius * std::sin(theta) * std::cos(phi));
                        vertexData[v].z = -(float) (radius * std::sin(phi));
                        break;
                    case ROTATE_90:
                        vertexData[v].x = -(float) (radius * std::sin(theta) * std::cos(phi));
                        vertexData[v].y = -(float) (radius * std::cos(theta) * std::cos(phi));
                        vertexData[v].z = -(float) (radius * std::sin(phi));
                        break;
                    case ROTATE_180:
                        vertexData[v].x = (float) (radius * std::cos(theta) * std::cos(phi));
                        vertexData[v].y = -(float) (radius * std::sin(theta) * std::cos(phi));
                        vertexData[v].z = -(float) (radius * std::sin(phi));
                        break;
                    case ROTATE_270:
                        vertexData[v].x = -(float) (radius * std::sin(theta) * std::cos(phi));
                        vertexData[v].y = (float) (radius * std::cos(theta) * std::cos(phi));
                        vertexData[v].z = -(float) (radius * std::sin(phi));
                        break;
                    default:
                        //This is what the original source code of GvrSphere uses.
                        //Not sure what rotation, but needed for the Gvr test video
                        vertexData[v].x = -(float) (radius * std::sin(theta) * std::cos(phi));
                        vertexData[v].y =  (float) (radius * std::sin(phi));
                        vertexData[v].z =  (float) (radius * std::cos(theta) * std::cos(phi));
                        break;
                }
                //vertexData[v].y =  (float) (radius * sin(theta) * sin(phi));
                //vertexData[v].z =  (float) (radius * cos(theta));

                // Set vertex texture.x data.
                if (mediaFormat == MEDIA_EQUIRECT_STEREO_LEFT_RIGHT) {
                    // For left-right media, each eye's x coordinate points to the left or right half of the
                    // texture.
                    vertexData[v].u_left = (i * quadWidthRads / horizontalFovRads) / 2;
                    vertexData[v].u_right = (i * quadWidthRads / horizontalFovRads) / 2 + .5f;
                } else {
                    // For top-bottom or monoscopic media, the eye's x spans the full width of the texture.
                    vertexData[v].u_left = i * quadWidthRads / horizontalFovRads;
                    vertexData[v].u_right = vertexData[v].u_left;
                }

                // Set vertex texture.y data. The "1 - ..." is due to Canvas vs GL coords.
                if (mediaFormat == MEDIA_EQUIRECT_STEREO_TOP_BOTTOM) {
                    // For top-bottom media, each eye's y coordinate points to the top or bottom half of the
                    // texture.
                    vertexData[v].v_left = 1 - (((j + k) * quadHeightRads / verticalFovRads) / 2 + .5f);
                    vertexData[v].v_right = 1 - ((j + k) * quadHeightRads / verticalFovRads) / 2;
                } else {
                    // For left-right or monoscopic media, the eye's y spans the full height of the texture.
                    vertexData[v].v_left = 1 - (j + k) * quadHeightRads / verticalFovRads;
                    vertexData[v].v_right = vertexData[v].v_left;
                }

                v++;

                // Break up the triangle strip with degenerate vertices by copying first and last points.
                if ((i == 0 && k == 0) || (i == longitudes && k == 1)) {
                    vertexData[v]=vertexData[v-1];
                    v++;
                }
            }
            // Move on to the next vertical edge in the triangle strip.
        }
        // Move on to the next triangle strip.
    }
    return vertexData;
}
