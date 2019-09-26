//
// Created by Consti10 on 10/09/2019.
//
//This one is to trick gradle into checking all source files for compilation issues,
//Not only hose used by the example
#include <GLProgramTextureExt.h>
#include <GLProgramSpherical.h>
#include "Sphere/Sphere.h"


static void dummy(){
    GLProgramSpherical* p1=new GLProgramSpherical(0);
    GLProgramTextureExt* p2=new GLProgramTextureExt(0);
}