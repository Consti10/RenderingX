//
// Created by Constantin on 2/17/2019.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER2_H
#define FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER2_H


#include "GLProgramVC.h"
#include "Color/Color.hpp"
#include <array>

class ColoredGeometry {
public:
    static const void makeColoredLine(GLProgramVC::Vertex array[],const int arrayOffset,const glm::vec3& point1,const glm::vec3& point2,
                                      const TrueColor color1,const TrueColor color2){
        auto* p=&array[arrayOffset];
        p[0].x=point1[0];
        p[0].y=point1[1];
        p[0].z=point1[2];
        p[0].colorRGBA=color1;
        p[1].x=point2[0];
        p[1].y=point2[1];
        p[1].z=point2[2];
        p[1].colorRGBA=color2;
    };
    static const void makeColoredTriangle(GLProgramVC::Vertex array[],const int arrayOffset,
                                              const glm::vec3& point1,const glm::vec3& point2,const glm::vec3& point3,
                                          const TrueColor color1,const TrueColor color2,const TrueColor color3) {
        GLProgramVC::Vertex* p=&array[arrayOffset];
        p[0].x = point1[0];
        p[0].y = point1[1];
        p[0].z = point1[2];
        p[0].colorRGBA = color1;
        p[1].x = point2[0];
        p[1].y = point2[1];
        p[1].z = point2[2];
        p[1].colorRGBA = color2;
        p[2].x = point3[0];
        p[2].y = point3[1];
        p[2].z = point3[2];
        p[2].colorRGBA = color3;
    };
    static const void makeColoredRect(GLProgramVC::Vertex array[],const glm::vec3& point,const glm::vec3& width,const glm::vec3& height,const TrueColor color){
        glm::vec3 p1=glm::vec3(point[0],point[1],point[2]);
        glm::vec3 p2=glm::vec3(point[0]+height[0],point[1]+height[1],point[2]+height[2]);
        glm::vec3 p3=glm::vec3(point[0]+height[0]+width[0],point[1]+height[1]+width[1],point[2]+height[2]+width[2]);
        glm::vec3 p4=glm::vec3(point[0],point[1],point[2]);
        glm::vec3 p5=glm::vec3(point[0]+height[0]+width[0],point[1]+height[1]+width[1],point[2]+height[2]+width[2]);
        glm::vec3 p6=glm::vec3(point[0]+width[0],point[1]+width[1],point[2]+width[2]);
        makeColoredTriangle(array,0,p1,p2,p3,color,color,color);
        makeColoredTriangle(array,3,p4,p5,p6,color,color,color);
    };

    static const void makeColoredTriangle1(GLProgramVC::Vertex array[],const glm::vec3& point,const float width,const float height,const TrueColor color) {
        glm::vec3 p1=glm::vec3(point[0],point[1],point[2]);
        glm::vec3 p2=glm::vec3(point[0]+width,point[1],point[2]);
        glm::vec3 p3=glm::vec3(point[0]+width/2.0f,point[1]+height,point[2]);
        makeColoredTriangle(array,0,p1,p2,p3,color,color,color);
    }

    static const void makeTesselatedColoredRect(GLProgramVC::Vertex array[],const int tesselation,const glm::vec3& point,const glm::vec3& width,const glm::vec3& height,const TrueColor color){
        int tesselationX=tesselation;
        int tesselationY=tesselation;
        float subW=width.x/(float)tesselationX;
        float subH=height.y/(float)tesselationY;
        int arrayOffset=0;
        for(int i=0;i<tesselationX;i++){
            float xPos=point[0]+(i*subW);
            for(int i2=0;i2<tesselationY;i2++){
                float yPos=point[1]+(i2*subH);
                //TODO
                makeColoredRect(&array[arrayOffset],glm::vec3(xPos,yPos,point[2]),glm::vec3(subW,0.0f,0.0f),glm::vec3(0.0f,subH,0.0f),color);
                arrayOffset+=6;
            }
        }
    };

    static const std::vector<GLProgramVC::Vertex> makeTesselatedColoredRectLines(const int tesselation,const glm::vec3& point,const float width,const float height,const TrueColor color){
        const int tesselationX=tesselation;
        const int tesselationY=tesselation;
        const float subW=width/(float)tesselationX;
        const float subH=height/(float)tesselationY;

       std::vector<GLProgramVC::Vertex> allVertexPoints((tesselationX+1)*(tesselationY+1));
       int idx=0;
        for(int x=0;x<=tesselationX;x++){
            float xPos=point.x+(x*subW);
            for(int y=0;y<=tesselationY;y++){
                float yPos=point.y+(y*subH);
                allVertexPoints[idx].x=xPos;
                allVertexPoints[idx].y=yPos;
                allVertexPoints[idx].z=point.z;
                allVertexPoints[idx].colorRGBA=color;
                idx++;
            }
        }
        //vertical lines
        std::vector<GLProgramVC::Vertex> horizontalLines(2*(tesselation+1)*(tesselation));
        int arrayOffset=0;
        for(int x=0;x<=tesselationX;x++){
            for(int y=0;y<tesselationY;y++){
                horizontalLines.at(arrayOffset++)=allVertexPoints[x*(tesselationX+1)+y];
                horizontalLines.at(arrayOffset++)=allVertexPoints[x*(tesselationX+1)+y+1];
            }
        }
        //horizontal lines
        std::vector<GLProgramVC::Vertex> verticalLines(2*(tesselation+1)*(tesselation));
        arrayOffset=0;
        for(int x=0;x<tesselationX;x++){
            for(int y=0;y<=tesselationY;y++){
                verticalLines.at(arrayOffset++)=allVertexPoints[x*(tesselationX+1)+y];
                verticalLines.at(arrayOffset++)=allVertexPoints[(x+1)*(tesselationX+1)+y];
            }
        }
        std::vector<GLProgramVC::Vertex> ret;
        ret.reserve(horizontalLines.size()+verticalLines.size());
        ret.insert(ret.end(),horizontalLines.begin(),horizontalLines.end());
        ret.insert(ret.end(),verticalLines.begin(),verticalLines.end());
        return ret;
    }

    //Create a tesselated wireframe mesh that spawns exactly the viewport
    static const std::vector<GLProgramVC::Vertex> makeTesselatedColoredRectLines0_1(const int tesselation,const TrueColor color){
        return makeTesselatedColoredRectLines(tesselation,{-1.0f,-1.0f,0},2.0f,2.0f,color);
    }


    static const void makeOutlineQuadWithLines(GLProgramVC::Vertex array[],const float mX,const float mY,const float mZ,const float quadWith,const float quadHeight,
                                               const TrueColor color){
        //left and right
        makeColoredLine(array, 0 * 2, glm::vec3(mX, mY, mZ), glm::vec3(mX, mY + quadHeight, mZ),color, color);
        makeColoredLine(array, 1 * 2, glm::vec3(mX + quadWith, mY, mZ),glm::vec3(mX + quadWith, mY + quadHeight, mZ), color, color);
        //top and bottom
        makeColoredLine(array, 2 * 2, glm::vec3(mX, mY + quadHeight, mZ),glm::vec3(mX + quadWith, mY + quadHeight, mZ), color, color);
        makeColoredLine(array, 3 * 2, glm::vec3(mX, mY, mZ), glm::vec3(mX + quadWith, mY, mZ),color, color);
    }
    static const void makeBackgroundRect(GLProgramVC::Vertex array[],const glm::vec3& point,const float width,const float height,const TrueColor color1,const TrueColor color2){
        //p4    p1
        //   p5
        //p3    p2
        glm::vec3 p1=glm::vec3(point.x+width,point.y+height,point.z);
        glm::vec3 p2=glm::vec3(point.x+width,point.y,point.z);
        glm::vec3 p3=glm::vec3(point.x,point.y,point.z);
        glm::vec3 p4=glm::vec3(point.x,point.y+height,point.z);
        glm::vec3 p5=glm::vec3(point.x+width/2.0f,point.y+height/2.0f,point.z);
        makeColoredTriangle(array,0*3,p5,p1,p4,color1,color2,color2);
        makeColoredTriangle(array,1*3,p5,p1,p2,color1,color2,color2);
        makeColoredTriangle(array,2*3,p5,p2,p3,color1,color2,color2);
        makeColoredTriangle(array,3*3,p5,p4,p3,color1,color2,color2);
    };
    static std::vector<GLProgramVC::Vertex> makeDebugCoordinateSystemLines(const int tesselation=1){
        std::vector<GLProgramVC::Vertex> ret;
        ret.reserve(((tesselation+1)*(tesselation+1)));
        const float size=10.0f;
        const float stepSize=size/tesselation;
        const auto color=Color::BLUE;
        for(int i=0;i<tesselation;i++){
            const float x=(-size/2.f)+i*stepSize;
            const float x2=(-size/2.f)+(i+1)*stepSize;
            GLProgramVC::Vertex line[2];
            ColoredGeometry::makeColoredLine(line,0,glm::vec3(x,0,0),glm::vec3(x2,0,0),color,color);
            ret.push_back(line[0]);
            ret.push_back(line[1]);
            ColoredGeometry::makeColoredLine(line,0,glm::vec3(0,x,0),glm::vec3(0,x2,0),color,color);
            ret.push_back(line[0]);
            ret.push_back(line[1]);
        }
        return ret;
    }
};


#endif //FPV_VR_GLPROGRAMS_HELPER_GEOMETRYHELPER2_H
