//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
#define FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H


#include "GLProgramTexture.h"

#include <utility>

//Create vertex/index buffers for drawing textured geometry
class TexturedGeometry {
private:
    static std::vector<GLProgramTexture::Vertex> createGridVertices(const unsigned int tessellation,const glm::vec3& point,float width,float height,const float uOffset,
                                                                    const float uRange){
        std::vector<GLProgramTexture::Vertex> vertices((tessellation+1)*(tessellation+1));
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
        return vertices;
    }
    static std::vector<GLProgramTexture::INDEX_DATA> createIndicesPlane(const unsigned int tessellation){
        const int indicesX=tessellation;
        const int indicesY=tessellation;
        const int rowSize=tessellation+1;
        unsigned int count=0;
        std::vector<GLProgramTexture::INDEX_DATA> indices(6*tessellation*tessellation);
        for(int i=0;i<indicesX;i++){
            for(int j=0;j<indicesY;j++){
                indices.at(count++)=(GLProgramTexture::INDEX_DATA)(i*rowSize+j);
                indices.at(count++)=(GLProgramTexture::INDEX_DATA)((i+1)*rowSize+j);
                indices.at(count++)=(GLProgramTexture::INDEX_DATA)(i*rowSize+j+1);
                indices.at(count++)=(GLProgramTexture::INDEX_DATA)(i*rowSize+j+1);
                indices.at(count++)=(GLProgramTexture::INDEX_DATA)((i+1)*rowSize+j);
                indices.at(count++)=(GLProgramTexture::INDEX_DATA)((i+1)*rowSize+j+1);
            }
        }
        return indices;
    }
    using VerticesIndices=std::pair<std::vector<GLProgramTexture::Vertex>,std::vector<GLProgramTexture::INDEX_DATA>>;
    static std::vector<GLProgramTexture::Vertex> mergeIndicesIntoVertices(const std::vector<GLProgramTexture::Vertex>& vertices,const std::vector<GLProgramTexture::INDEX_DATA>& indices){
        std::vector<GLProgramTexture::Vertex> ret={};
        ret.reserve(indices.size());
        for(unsigned int index:indices){
            if(index>=vertices.size()){
                LOGD("Error wanted %d",index);
            }
            ret.push_back(vertices.at(index));
        }
        return ret;
    }
public:
    static const VerticesIndices makeTesselatedVideoCanvas(const  glm::vec3& point,const float width,
                                                const float height,const int tessellation, const float uOffset,
                                                const float uRange){
        const auto vertices=createGridVertices(tessellation,point,width,height,uOffset,uRange);
        const auto indices=createIndicesPlane(tessellation);
        return {vertices,indices};
    }
    //Sometimes we want no indices for simplicity over performance
    static const std::vector<GLProgramTexture::Vertex> makeTesselatedVideoCanvas2(const  glm::vec3& point,const float width,
                                                                                  const float height,const int tessellation, const float uOffset,const float uRange){
        const auto tmp=makeTesselatedVideoCanvas(point,width,height,tessellation,uOffset,uRange);
        return mergeIndicesIntoVertices(tmp.first,tmp.second);
    }

    static const std::vector<GLProgramTexture::Vertex> makePyramid(){
        return {
                { 1.0f, -1.0f, 1.0f, 0.0319, 0.4192 }, // face 1
                { -1.0f, -1.0f, -1.0f, 0.3546, 0.0966 },
                { 1.0f, -1.0f, -1.0f, 0.3546, 0.4192 },
                { 1.0f, -1.0f, -1.0f, 0.4223, 0.5177 }, // face 2
                { 0.0f, 1.0f, 0.0f, 0.2541, 0.8753 },
                { 1.0f, -1.0f, 1.0f, 0.0996, 0.5116 },
                { 1.0f, -1.0f, 1.0f, 0.8047, 0.5250 }, // face 3
                { 0.0f, 1.0f, 0.0f, 0.6434, 0.8857 },
                { -1.0f, -1.0f, 1.0f, 0.4820, 0.5250 },
                { -1.0f, -1.0f, 1.0f, 0.6637, 0.0981 }, // face 4
                { 0.0f, 1.0f, 0.0f, 0.5130, 0.4184 },
                { -1.0f, -1.0f, -1.0f,0.3748, 0.0926 },
                { 0.0f, 1.0f, 0.0f, 0.8416, 0.4227  }, // face 5
                { 1.0f, -1.0f, -1.0f, 0.6922, 0.0988 },
                { -1.0f, -1.0f, -1.0f,0.9834, 0.0954 },
                { 1.0f, -1.0f, 1.0f,  0.0319, 0.4192 }, // face 6
                { -1.0f, -1.0f, 1.0f, 0.0319, 0.0966 },
                { -1.0f, -1.0f, -1.0f, 0.3546, 0.0966 }
        };
    }
};


#endif //FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER1_H
