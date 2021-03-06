//
// Created by Constantin on 6/15/2018.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H
#define FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H

#include "GLProgramText.h"
#include "GLProgramVC.h"
#include "GLProgramLine.h"

class BasicGLPrograms{
public:
    explicit BasicGLPrograms():
        vc(),
        text(),
        line()
    {}
public:
    //Once these GLPrograms are created, they don't change - only the content of the buffers they draw changes
    const GLProgramVC vc;
    const GLProgramText text;
    const GLProgramLine line;
};


#endif //FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H
