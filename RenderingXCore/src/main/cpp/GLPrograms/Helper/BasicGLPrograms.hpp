//
// Created by Constantin on 6/15/2018.
//

#ifndef FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H
#define FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H

#include "GLProgramText.h"
#include "GLProgramTextureExt.h"
#include "GLProgramVC.h"
#include "GLProgramLine.h"

//Every renderer uses these 3
//makes it easier to create all 3 of them at once
//and pass all 3 GLPrograms to functions together

class BasicGLPrograms{
public:
    explicit BasicGLPrograms(const bool enableDistortionCorrection=false,const std::array<float,7>* undistortionCoeficients= nullptr):
        vc(enableDistortionCorrection,undistortionCoeficients),
        text(enableDistortionCorrection,undistortionCoeficients),
        line(enableDistortionCorrection,undistortionCoeficients)
    {}
public:
    //Once these GLPrograms are created, they don't change - only the content of the buffers they draw changes
    const GLProgramVC vc;
    const GLProgramText text;
    const GLProgramLine line;
};


#endif //FPV_VR_GLPROGRAMS_HELPER_BASICGLPROGRAMS_H
