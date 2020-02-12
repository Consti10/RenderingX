//
// Created by Consti10 on 24/09/2019.
//

#ifndef FPV_VR_2018_GVRSPHERE_H
#define FPV_VR_2018_GVRSPHERE_H

#include <vector>

//Only depends on standard libraries
//Inside th gvr 360° video example source code, there is a function to create
//The geometry with texture data for a 360° video sphere
//This .cpp class is inspired by the following .java class:
// https://github.com/googlevr/gvr-android-sdk/blob/master/samples/sdk-video360/src/main/java/com/google/vr/sdk/samples/video360/rendering/Mesh.java
//U,Vs map to an Equirectangular image
//https://en.wikipedia.org/wiki/Equirectangular_projection

class UvSphere{
public:
    enum MEDIA_FORMAT{
/** Standard media where a single camera frame takes up the entire media frame. */
   MEDIA_MONOSCOPIC = 0,
/**
 * Stereo media where the left & right halves of the frame are rendered for the left & right eyes,
 * respectively. If the stereo media is rendered in a non-VR display, only the left half is used.
 */
    MEDIA_STEREO_LEFT_RIGHT = 1,
/**
 * Stereo media where the top & bottom halves of the frame are rendered for the left & right eyes,
 * respectively. If the stereo media is rendered in a non-VR display, only the top half is used.
 */
    MEDIA_STEREO_TOP_BOTTOM = 2,
    };
// The vertex contains texture coordinates for both the left & right eyes. If the scene is
// rendered in VR, the appropriate part of the vertex will be selected at runtime. For a mono
// scene, only the left eye's UV coordinates are used.
// For mono media, the UV coordinates are duplicated in each. For stereo media, the UV coords
// point to the appropriate part of the source media.
    struct Vertex{
        float x,y,z;
        float u_left,v_left;
        float u_right,v_right;
    };
    /**
  * Generates a 3D UV sphere for rendering monoscopic or stereoscopic video.
  *
  * @param radius Size of the sphere. Must be > 0.
  * @param latitudes Number of rows that make up the sphere. Must be >= 1.
  * @param longitudes Number of columns that make up the sphere. Must be >= 1.
  * @param verticalFovDegrees Total latitudinal degrees that are covered by the sphere. Must be in
  *    (0, 180].
  * @param horizontalFovDegrees Total longitudinal degrees that are covered by the sphere.Must be
  *    in (0, 360].
  * @param mediaFormat A MEDIA_* value.
  * @return  std::vector of type GvrSphere::Vertex
  */
    static std::vector<UvSphere::Vertex> createUvSphere(
            float radius,
            int latitudes,
            int longitudes,
            float verticalFovDegrees,
            float horizontalFovDegrees,
            MEDIA_FORMAT mediaFormat);
};


#endif //FPV_VR_2018_GVRSPHERE_H
