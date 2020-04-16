//
// Created by Consti10 on 27/11/2019.
//

#ifndef CARDBOARD_COORDINATESYSTEMLINES_H
#define CARDBOARD_COORDINATESYSTEMLINES_H

#include <GLES2/gl2.h>
#include "../GLPrograms/GLProgramVC.h"
#include "../GeometryBuilder/ColoredGeometry.hpp"
#include "GLBufferHelper.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Helper to draw lines trough the viewport coordinates (e.g. from -1 to 1)
// For debugging

class CoordinateSystemLines{
public:
    CoordinateSystemLines(){}
    //call with gl context bound
    void initializeGL(){
        const auto coordinateSystemLines=ColoredGeometry::makeDebugCoordinateSystemLines(100);
        glGenBuffers(1,&glBufferCoordinateSystemLines);
        nCoordinateSystemLinesVertices=GLBufferHelper::uploadGLBuffer(glBufferCoordinateSystemLines,coordinateSystemLines);
    }
    void drawGL(GLProgramVC* glProgramVC,const int viewportX,const int viewportY,const int viewportW,const int viewportH){
        glProgramVC->beforeDraw(glBufferCoordinateSystemLines);
        glProgramVC->draw(glm::value_ptr(glm::mat4(1.0)),glm::value_ptr(glm::mat4(1.0)),0,nCoordinateSystemLinesVertices,GL_LINES);
        glProgramVC->draw(glm::value_ptr(glm::mat4(1.0)),glm::value_ptr(glm::mat4(1.0)),0,nCoordinateSystemLinesVertices,GL_POINTS);
        glProgramVC->afterDraw();
    }
private:
    GLuint glBufferCoordinateSystemLines;
    int nCoordinateSystemLinesVertices;
};

#endif //CARDBOARD_COORDINATESYSTEMLINES_H
