//
// Created by Constantin on 2/16/2019.
//

#ifndef FPV_VR_GLPROGRAMLINE_H
#define FPV_VR_GLPROGRAMLINE_H

#include <GLES2/gl2.h>
#include <jni.h>
#include <string>
#include <vector>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Types/Color.hpp>
#include <HelperShader/VDDC.hpp>

class GLProgramLine {
private:
    GLuint mProgram;
    GLuint mPositionHandle,mNormalHandle,mLineWidthHandle,mBaseColorHandle,mOutlineColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    const bool distortionCorrection;
public:
    struct Vertex{
        float x,y,z;
        float normalX,normalY,normalZ;
        float lineW;
        TrueColor baseColor;
        TrueColor outlineColor;
    };
    static constexpr int VERTICES_PER_LINE=6;
    explicit GLProgramLine(bool enableDist=false,const std::array<float,7> *optionalCoeficients= nullptr);
    void beforeDraw(GLuint buffer) const;
    void draw(const glm::mat4x4& ViewM, const  glm::mat4x4& ProjM, int verticesOffset, int numberVertices) const;
    void afterDraw() const;
public:
    static void convertLineToRenderingData(const glm::vec3& start,const glm::vec3& end,float lineWidth,
            Vertex array[],int arrayOffset,TrueColor baseColor=Color::BLACK,TrueColor  outlineColor=Color::WHITE);
private:
    static const std::string VS(const bool eVDDC,
                                const std::array<float, VDDC::N_UNDISTORTION_COEFICIENTS> *optionalCoeficients){
        std::stringstream s;
        s<<"uniform mat4 uMVMatrix;\n";
        s<<"uniform mat4 uPMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec3 aNormal;\n";
        s<<"attribute float aLineWidth;";
        s<<"attribute vec4 aBaseColor;";
        s<<"attribute vec4 aOutlineColor;";
        s<<"varying vec3 vNormal;";
        s<<"varying vec4 vBaseColor;";
        s<<"varying vec4 vOutlineColor;";
        if(eVDDC){
            s<<VDDC::undistortCoeficientsToString(*optionalCoeficients);
        }
        s<<"vec4 extruded_pos;";
        s<<"void main(){\n";
        s<<"extruded_pos=aPosition+vec4(aNormal*aLineWidth,0.0);\n";
        s<<VDDC::writeGLPosition(eVDDC,"extruded_pos");
        //s<<"gl_Position = (uPMatrix*uMVMatrix)* extruded_pos;\n";
        s<<"vNormal=aNormal;";
        s<<"vBaseColor=aBaseColor;";
        s<<"vOutlineColor=aOutlineColor;";
        s<<"}\n";
        return s.str();
    }

    static const std::string FS(){
        std::stringstream s;
        s<<"precision mediump float;\n";
        s<<"varying vec3 vNormal;";
        s<<"varying vec4 vBaseColor;";
        s<<"varying vec4 vOutlineColor;";

        s<<"const float width=0.5;";
        s<<"const float edge=0.01;";
        s<<"const float outlineStrength=0.4;";
        s<<"const float borderEdge=0.01;";

        s<<"void main(){\n";
        s<<"float distanceInv=abs(length(vNormal));";

        /*s<<"vec4 color;";
        s<<"if(distance<0.5){";
        s<<"color=vec4(1.0,1.0,1.0,1.0);";
        s<<"}else{";
        s<<"color=vec4(0.0,0.0,0.0,1.0);";
        s<<"}";
        s<<"gl_FragColor=color;";*/

        s<<"float alpha = 1.0- smoothstep(width,width+edge,distanceInv);";
        s<<"float outlineAlpha = 1.0 - smoothstep(width+outlineStrength,width+outlineStrength+borderEdge,distanceInv);";
        s<<"float overallAlpha= alpha + (1.0 - alpha) * outlineAlpha ;";
        s<<"vec3 overallColor= mix(vOutlineColor.rgb,vBaseColor.rgb, alpha / overallAlpha);\n";
        s<<"gl_FragColor=vec4(overallColor*overallAlpha,overallAlpha);";

        /*s<<"float alpha = smoothstep(width,width+edge,distance);";
        s<<"gl_FragColor = vec4(vec3(1.0,0.0,0.0)*alpha,alpha);";*/

        s<<"}\n";
        s<<"\n";
        return s.str();
    }
};


#endif //FPV_VR_GLPROGRAMLINE_H
