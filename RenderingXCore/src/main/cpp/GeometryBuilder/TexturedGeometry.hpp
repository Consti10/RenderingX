//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
#define FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H


#include "GLProgramTexture.h"

#include <utility>

class TexturedGeometry {
public:
    static const std::pair<std::vector<GLProgramTexture::Vertex>,std::vector<GLProgramTexture::INDEX_DATA>>  makeTesselatedVideoCanvas(const  glm::vec3& point,const float width,
                                                const float height,const int tessellation, const float uOffset,
                                                const float uRange){
        std::vector<GLProgramTexture::Vertex> vertices((tessellation+1)*(tessellation+1));
        std::vector<GLProgramTexture::INDEX_DATA> indices(6*tessellation*tessellation);
        const float vRange=1.0f;
        const int tessellationX=tessellation;
        const int tessellationY=tessellation;
        const float subW=width/(float)tessellationX;
        const float subH=height/(float)tessellationY;
        float subURange=uRange/(float)tessellationX;
        float subVRange=vRange/(float)tessellationY;
        int count=0;
        for(int i=0;i<tessellationX+1;i++){
            for(int j=0;j<tessellationY+1;j++){
                GLProgramTexture::Vertex& v=vertices.at(count);
                v.x=point.x+subW*j;
                v.y=point.y+subH*i;
                v.z=point.z;
                v.u=uOffset+subURange*j;//uRange-
                v.v=vRange-subVRange*i;
                count++;
            }
        }
         //__android_log_print(ANDROID_LOG_DEBUG, "count V:","%d",count);
        const int indicesX=tessellation;
        const int indicesY=tessellation;
        const int rowSize=tessellationX+1;
        count=0;
        for(int i=0;i<indicesX;i++){
            for(int j=0;j<indicesY;j++){
                indices.at(count++)=(GLushort)(i*rowSize+j);
                indices.at(count++)=(GLushort)((i+1)*rowSize+j);
                indices.at(count++)=(GLushort)(i*rowSize+j+1);
                //
                indices.at(count++)=(GLushort)(i*rowSize+j+1);
                indices.at(count++)=(GLushort)((i+1)*rowSize+j);
                indices.at(count++)=(GLushort)((i+1)*rowSize+j+1);
            }
        }
        return std::pair<std::vector<GLProgramTexture::Vertex>,std::vector<GLProgramTexture::INDEX_DATA>>{
            vertices,indices};
    }

    //Sometimes we want no indices for simplicity over performance
    static const std::vector<GLProgramTexture::Vertex> makeTesselatedVideoCanvas2(const  glm::vec3& point,const float width,
                                                                                  const float height,const int tessellation, const float uOffset,const float uRange){
        const auto tmp=makeTesselatedVideoCanvas(point,width,height,tessellation,uOffset,uRange);
        //we do not want indices
        std::vector<GLProgramTexture::Vertex> ret;
        ret.reserve(tmp.second.size());
        for(int i=0;i<tmp.second.size();i++){
            const auto index=tmp.second.at(i);
            ret.push_back(tmp.first.at(index));
        }
        return ret;
    }

};


#endif //FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
