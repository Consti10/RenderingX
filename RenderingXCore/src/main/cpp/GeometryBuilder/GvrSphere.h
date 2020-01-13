//
// Created by Consti10 on 24/09/2019.
//

#ifndef FPV_VR_2018_GVRSPHERE_H
#define FPV_VR_2018_GVRSPHERE_H

#include <vector>

//Inside th gvr 360° video example source code, there is a function to create
//The geometry with texture data for a 360° video sphere
//This class is the .cpp equivalent to the following class:
// https://github.com/googlevr/gvr-android-sdk/blob/master/samples/sdk-video360/src/main/java/com/google/vr/sdk/samples/video360/rendering/Mesh.java
//As less modifcations have been made as possible
//Only the functions Math.toRadians and System.arraycopy() did not exist in .cpp
//Also std::vector<float> instead of dynamic java float array


class GvrSphere{
public:
    /** Standard media where a single camera frame takes up the entire media frame. */
    static constexpr int MEDIA_MONOSCOPIC = 0;
/**
 * Stereo media where the left & right halves of the frame are rendered for the left & right eyes,
 * respectively. If the stereo media is rendered in a non-VR display, only the left half is used.
 */
    static constexpr int MEDIA_STEREO_LEFT_RIGHT = 1;
/**
 * Stereo media where the top & bottom halves of the frame are rendered for the left & right eyes,
 * respectively. If the stereo media is rendered in a non-VR display, only the top half is used.
 */
    static constexpr int MEDIA_STEREO_TOP_BOTTOM = 2;

// Constants related to vertex data.
    static constexpr int POSITION_COORDS_PER_VERTEX = 3; // X, Y, Z.
// The vertex contains texture coordinates for both the left & right eyes. If the scene is
// rendered in VR, the appropriate part of the vertex will be selected at runtime. For a mono
// scene, only the left eye's UV coordinates are used.
// For mono media, the UV coordinates are duplicated in each. For stereo media, the UV coords
// point to the appropriate part of the source media.
    static constexpr int TEXTURE_COORDS_PER_VERTEX = 2 * 2;
// COORDS_PER_VERTEX
    static constexpr int CPV = POSITION_COORDS_PER_VERTEX + TEXTURE_COORDS_PER_VERTEX;
// Data is tightly packed. Each vertex is [x, y, z, u_left, v_left, u_right, v_right].
    static constexpr int VERTEX_STRIDE_BYTES = CPV * sizeof(float);//CPV * Utils.BYTES_PER_FLOAT;
public:
    static std::vector<float> createUvSphere(
            float radius,
            int latitudes,
            int longitudes,
            float verticalFovDegrees,
            float horizontalFovDegrees,
            int mediaFormat);

public:
    //The gvr vertex is different than what I use in my GLProgram's.
    //A struct better represents the internal data layout than a float array
    struct Vertex{
        float x,y,z;
        float u_left,v_left;
        float u_right,v_right;
    };
    static std::vector<Vertex> createUvSphere2(
            float radius,
            int latitudes,
            int longitudes,
            float verticalFovDegrees,
            float horizontalFovDegrees,
            int mediaFormat);
};


#endif //FPV_VR_2018_GVRSPHERE_H
