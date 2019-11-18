//
// Created by Consti10 on 07/11/2019.
//

#ifndef RENDERINGX_HELPERX_H
#define RENDERINGX_HELPERX_H


#include <GLProgramTexture.h>

class HelperX{
public:
    //Input: mesh with 0,0 in lower left viewport and 1,1 in upper right viewport
    static std::vector<GLProgramTexture::Vertex> distortUVCoordinates(
            const std::vector<GLProgramTexture::Vertex> &data, gvr_context *context, gvr_eye eye,const int channel = -1){
        std::vector<GLProgramTexture::Vertex> ret(data.size());
        for(int i=0;i<data.size();i++){
            const GLProgramTexture::Vertex& v=data.at(i);
            gvr_vec2f out[3];
            gvr_compute_distorted_point(context,eye,{v.u,v.v},out);
            if(channel==-1){
                float avgX=(out[0].x+out[1].x+out[2].x)/3.0f;
                float avgY=(out[0].y+out[1].y+out[2].y)/3.0f;
                ret.at(i)={v.x,v.y,v.z,avgX,avgY};
            }else{
                ret.at(i)={v.x,v.y,v.z,out[channel].x,out[channel].y};
            }
        }
        return ret;
    }
    static void generateDistortionMeshBuffersTEST(const std::vector<GLProgramTexture::Vertex> &data,gvr_context *context,
            GLuint& glBufferLeftEye,GLuint& glBufferRightEye,GLuint glBuffersLeftEye_rgb[3],GLuint glBuffersRighEye_rgb[3]){
        glGenBuffers(1,&glBufferLeftEye);
        glGenBuffers(1,&glBufferRightEye);
        const auto tmp1=distortUVCoordinates(data,context,GVR_LEFT_EYE,-1);
        const auto tmp2=distortUVCoordinates(data,context,GVR_RIGHT_EYE,-1);
        GLBufferHelper::allocateGLBufferStatic(glBufferLeftEye,tmp1);
        GLBufferHelper::allocateGLBufferStatic(glBufferRightEye,tmp2);

        glGenBuffers(3,glBuffersLeftEye_rgb);
        for(int rgb=0;rgb<3;rgb++){
            const auto textV=HelperX::distortUVCoordinates(data,context, GVR_LEFT_EYE, rgb);
            GLBufferHelper::allocateGLBufferStatic(glBuffersLeftEye_rgb[rgb],textV);
        }
        glGenBuffers(3,glBuffersRighEye_rgb);
        for(int rgb=0;rgb<3;rgb++){
            const auto textV=HelperX::distortUVCoordinates(data,context, GVR_RIGHT_EYE, rgb);
            GLBufferHelper::allocateGLBufferStatic(glBuffersRighEye_rgb[rgb],textV);
        }
    }
    static void debugColorChannelDifferences(gvr_context* gvrContext){
        for(float x=0;x<1.0f;x+=0.01f){
            for(float y=0;y<1.0f;y+=0.01f){
                gvr_vec2f out[3];
                gvr_compute_distorted_point(gvrContext,GVR_LEFT_EYE,{x,y},out);
                float diffX=out[1].x-out[0].x;
                float diffY=out[1].y-out[0].y;
                bool exactSame=diffX==0.0f && diffY==0.0f;
                LOGD("For value (%f,%f) the diff between r and a is: (%f,%f) (=%s)",x,y,diffX,diffY,exactSame ? "exactSame" : "notExactSame");
            }
        }
    }

};

#endif //RENDERINGX_HELPERX_H
