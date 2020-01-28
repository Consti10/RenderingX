//
// Created by Constantin on 6/15/2018.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H
#define FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H

#include "GLProgramText.h"
#include "GLProgramVC.h"
#include "GLProgramLine.h"

//Every renderer uses these 4
//makes it easier to create all of them at once
//and pass all GLPrograms to functions together

class BasicGLPrograms{
public:
    explicit BasicGLPrograms(const DistortionManager* distortionManager):
        vc(distortionManager),
        text(distortionManager),
        line(distortionManager),
        vc2D()
    {}
public:
    //Once these GLPrograms are created, they don't change - only the content of the buffers they draw changes
    const GLProgramVC vc;
    const GLProgramText text;
    const GLProgramLine line;
    //The 2D program never does V.D.D.C -it only works on ('raw') viewport coordinates
    const GLProgramVC2D vc2D;
};


#endif //FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H
