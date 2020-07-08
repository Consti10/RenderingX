//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER2_H
#define FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER2_H

#include <IndicesHelper.hpp>
#include "GLProgramVC.h"
#include <TrueColor.hpp>
#include <array>
#include "VerticalPlane.hpp"

//Create vertex/index buffers for drawing colored geometry
namespace ColoredGeometry {
    //Needs at least space for 2 more vertices at array*
    static const void makeColoredLine(GLProgramVC::Vertex* array, const glm::vec3& point1, const glm::vec3& point2,
                                      const TrueColor color1, const TrueColor color2){
        auto& p0=array[0];
        auto& p1=array[1];
        p0.x=point1[0];
        p0.y=point1[1];
        p0.z=point1[2];
        p0.colorRGBA=color1;
        p1.x=point2[0];
        p1.y=point2[1];
        p1.z=point2[2];
        p1.colorRGBA=color2;
    };
    //Needs at least space for 3 more vertices at array*
    static const void makeColoredTriangle(GLProgramVC::Vertex array[], const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3,
                                          const TrueColor color1, const TrueColor color2, const TrueColor color3) {
        auto& p0=array[0];
        auto& p1=array[1];
        auto& p2=array[2];
        p0.x = point1[0];
        p0.y = point1[1];
        p0.z = point1[2];
        p0.colorRGBA = color1;
        p1.x = point2[0];
        p1.y = point2[1];
        p1.z = point2[2];
        p1.colorRGBA = color2;
        p2.x = point3[0];
        p2.y = point3[1];
        p2.z = point3[2];
        p2.colorRGBA = color3;
    };
    //Needs at least space for 3 more vertices at array*
    static void makeColoredTriangle1(GLProgramVC::Vertex array[],const glm::vec3& point,const float width,const float height,const TrueColor color) {
        glm::vec3 p1=glm::vec3(point[0],point[1],point[2]);
        glm::vec3 p2=glm::vec3(point[0]+width,point[1],point[2]);
        glm::vec3 p3=glm::vec3(point[0]+width/2.0f,point[1]+height,point[2]);
        makeColoredTriangle(array,p1,p2,p3,color,color,color);
    }
    static ColoredMeshData makeTessellatedColoredRect(const unsigned int tessellation, const glm::vec3 &translation,const glm::vec2 scale,const TrueColor color){
        const auto vertices= VerticalPlane::createVerticesPlaneColored(tessellation,translation,scale,color);
        const auto indices=VerticalPlane::createIndicesPlane(tessellation);
        auto ret=ColoredMeshData(vertices,indices,GL_TRIANGLES);
        //we do not want indices, remove them
        ret.mergeIndicesIntoVertexBuffer();
        return ret;
    }
    static ColoredMeshData makeTessellatedColoredRectWireframe(const unsigned int tessellation, const glm::vec3 &translation,const glm::vec2 scale,const TrueColor color){
        const auto vertices=  VerticalPlane::createVerticesPlaneColored(tessellation,translation,scale,color);
        const auto indices=VerticalPlane::createIndicesWireframe(tessellation);
        auto ret=ColoredMeshData(vertices,indices,GL_LINES);
        //we do not want indices, remove them
        ret.mergeIndicesIntoVertexBuffer();
        return ret;
    }
    //Create a tesselated wireframe mesh that spawns exactly the viewport
    //e.q [-1,1] in x and y direction
    static const ColoredMeshData makeTessellatedColoredRectLinesNDC(
            const unsigned int tessellation, const TrueColor color){
        return makeTessellatedColoredRectWireframe(tessellation,{-1.0f,-1.0f,0},{2.0f,2.0f},color);
    }
    static const void makeOutlineQuadWithLines(GLProgramVC::Vertex array[],const float mX,const float mY,const float mZ,const float quadWith,const float quadHeight,
                                               const TrueColor color){
        //left and right
        makeColoredLine(&array[0 * 2], glm::vec3(mX, mY, mZ), glm::vec3(mX, mY + quadHeight, mZ),color, color);
        makeColoredLine(&array[1 * 2], glm::vec3(mX + quadWith, mY, mZ),glm::vec3(mX + quadWith, mY + quadHeight, mZ), color, color);
        //top and bottom
        makeColoredLine(&array[2 * 2], glm::vec3(mX, mY + quadHeight, mZ),glm::vec3(mX + quadWith, mY + quadHeight, mZ), color, color);
        makeColoredLine(&array[3 * 2], glm::vec3(mX, mY, mZ), glm::vec3(mX + quadWith, mY, mZ),color, color);
    }
    static const void makeBackgroundRect(GLProgramVC::Vertex array[], const glm::vec3& point, const float width, const float height, const TrueColor color1, const TrueColor color2){
        //p4    p1
        //   p5
        //p3    p2
        glm::vec3 p1=glm::vec3(point.x+width,point.y+height,point.z);
        glm::vec3 p2=glm::vec3(point.x+width,point.y,point.z);
        glm::vec3 p3=glm::vec3(point.x,point.y,point.z);
        glm::vec3 p4=glm::vec3(point.x,point.y+height,point.z);
        glm::vec3 p5=glm::vec3(point.x+width/2.0f,point.y+height/2.0f,point.z);
        makeColoredTriangle(&array[0*3],p5,p1,p4,color1,color2,color2);
        makeColoredTriangle(&array[1*3],p5,p1,p2,color1,color2,color2);
        makeColoredTriangle(&array[2*3],p5,p2,p3,color1,color2,color2);
        makeColoredTriangle(&array[3*3],p5,p4,p3,color1,color2,color2);
    };
    static std::vector<GLProgramVC::Vertex> makeDebugCoordinateSystemLines(const unsigned int tessellation=1){
        std::vector<GLProgramVC::Vertex> ret;
        ret.reserve(((tessellation+1)*(tessellation+1)));
        const float size=10.0f;
        const float stepSize=size/tessellation;
        const auto color=TrueColor2::BLUE;
        for(int i=0;i<tessellation;i++){
            const float x=(-size/2.f)+i*stepSize;
            const float x2=(-size/2.f)+(i+1)*stepSize;
            GLProgramVC::Vertex line[2];
            ColoredGeometry::makeColoredLine(line,glm::vec3(x,0,0),glm::vec3(x2,0,0),color,color);
            ret.push_back(line[0]);
            ret.push_back(line[1]);
            ColoredGeometry::makeColoredLine(line,glm::vec3(0,x,0),glm::vec3(0,x2,0),color,color);
            ret.push_back(line[0]);
            ret.push_back(line[1]);
        }
        return ret;
    }
};


#endif //FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER2_H
