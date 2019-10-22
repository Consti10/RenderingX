//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
#define FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H


#include "GLProgramTextureExt.h"

class TexturedGeometry {
public:
    static const void makeTexturedTriangle(GLProgramTextureExt::Vertex *array,const int arrayOffset,
                                           const glm::vec3& point1,const glm::vec3& point2,const glm::vec3& point3,
                                           const glm::vec2& uv1,const glm::vec2& uv2,const glm::vec2& uv3) {
        auto* p=&array[arrayOffset];
        p[ 0].x = point1[0];
        p[ 0].y = point1[1];
        p[ 0].z = point1[2];
        p[ 0].u = uv1[0];
        p[ 0].v = uv1[1];

        p[ 1].x = point2[0];
        p[ 1].y = point2[1];
        p[ 1].z = point2[2];
        p[ 1].u = uv2[0];
        p[ 1].v = uv2[1];

        p[ 2].x = point3[0];
        p[ 2].y = point3[1];
        p[ 2].z = point3[2];
        p[ 2].u = uv3[0];
        p[ 2].v = uv3[1];
    }


    static const void makeTexturedRect(GLProgramTextureExt::Vertex array[],const int arrayOffset,const glm::vec3& point,const glm::vec3& width,const glm::vec3& height,
                                       const float u,const float v,const float uRange,const float vRange){
        //|--------------------------------|h
        //|                                |e
        //|                                |i (V)
        //|                                |g
        //|                                |h
        //|--------------------------------|t
        // point           width (U)
        //u is normal, but v is flipped
        glm::vec3 p1=glm::vec3(point[0],point[1],point[2]);
        glm::vec3 p2=glm::vec3(point[0]+height[0],point[1]+height[1],point[2]+height[2]);
        glm::vec3 p3=glm::vec3(point[0]+height[0]+width[0],point[1]+height[1]+width[1],point[2]+height[2]+width[2]);
        glm::vec3 p4=glm::vec3(point[0],point[1],point[2]);
        glm::vec3 p5=glm::vec3(point[0]+height[0]+width[0],point[1]+height[1]+width[1],point[2]+height[2]+width[2]);
        glm::vec3 p6=glm::vec3(point[0]+width[0],point[1]+width[1],point[2]+width[2]);

        glm::vec2 uv1=glm::vec2(u,v+vRange);
        glm::vec2 uv2=glm::vec2(u,v);
        glm::vec2 uv3=glm::vec2(u+uRange,v);
        glm::vec2 uv4=glm::vec2(u,v+vRange);
        glm::vec2 uv5=glm::vec2(u+uRange,v);
        glm::vec2 uv6=glm::vec2(u+uRange,v+vRange);

        makeTexturedTriangle(array, arrayOffset + 0, p1, p2, p3, uv1, uv2, uv3);
        makeTexturedTriangle(array, arrayOffset + 4, p4, p5, p6, uv4, uv5, uv6);
    }

    static const void makeTesselatedVideoCanvas(GLProgramTextureExt::Vertex *vertices,
                                                GLushort *indices,const  glm::vec3& point,const float width,
                                                const float height,const int tesselation, const float uOffset,
                                                const float uRange){
        const float vRange=1.0f;
        const int tesselationX=tesselation;
        const int tesselationY=tesselation;
        const float subW=width/(float)tesselationX;
        const float subH=height/(float)tesselationY;
        float subURange=uRange/(float)tesselationX;
        float subVRange=vRange/(float)tesselationY;

        int count=0;
        for(int i=0;i<tesselationX+1;i++){
            for(int j=0;j<tesselationY+1;j++){
                vertices[count].x=point.x+subW*j;
                vertices[count].y=point.y+subH*i;
                vertices[count].z=point.z;
                vertices[count].u=uOffset+subURange*j;//uRange-
                vertices[count].v=vRange-subVRange*i;
                count++;
            }
        }
         //__android_log_print(ANDROID_LOG_DEBUG, "count V:","%d",count);
        const int indicesX=tesselation;
        const int indicesY=tesselation;
        const int rowSize=tesselationX+1;
        count=0;
        for(int i=0;i<indicesX;i++){
            for(int j=0;j<indicesY;j++){
                indices[count++]=(GLushort)(i*rowSize+j);
                indices[count++]=(GLushort)((i+1)*rowSize+j);
                indices[count++]=(GLushort)(i*rowSize+j+1);
                //
                indices[count++]=(GLushort)(i*rowSize+j+1);
                indices[count++]=(GLushort)((i+1)*rowSize+j);
                indices[count++]=(GLushort)((i+1)*rowSize+j+1);
            }
        }
        //__android_log_print(ANDROID_LOG_DEBUG, "count I:","%d",count);
        /*std::stringstream ss;
        for(int i=0;i<58;i++){
            ss<<indices[i]<<" ";
        }
        __android_log_print(ANDROID_LOG_DEBUG, "X","%s",ss.str().c_str());*/
    }


};


#endif //FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
