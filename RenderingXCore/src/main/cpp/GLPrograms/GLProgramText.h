/***********************************************************
 * Abstraction for drawing Text.
 * The Application has to
 * 1)load the Texture font atlas into texture memory via "loadTextureImage",optionally with all mipmap levels
 * 2) create and fill a GLBuffer with vertex/uv/color data. This can be done using convertStringToVECs_UVs
 * 3) draw the Text by calling beforeDraw(),draw() and afterDraw()
 * an example for drawing first a red, then a green string:
 *  a)beforeDraw(mGLBuffer);
 *  b)draw(...,r=1.0,g=0.0,b=0.0)
 *  b)draw(...,r=0.0,g=1.0,b=0.0)
 *  c)afterDraw();
 *
 *  Note: When drawing dynamically changing text you should consider multithreading and asynchronous uploading of vecs_uvs data, like
 *  I did in OSD/TextElements.cpp
 * */


#ifndef GL_PROGRAM_RENDER_TEXT_H
#define GL_PROGRAM_RENDER_TEXT_H

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <jni.h>
#include <string>
#include "TextAssetsHelper.hpp"
#include <TrueColor.hpp>
#include "../GLHelper/GLBuffer.hpp"

class GLProgramText {
private:
    GLuint mProgram;
    GLuint mPositionHandle,mTextureHandle,mSamplerHandle,mColorHandle;
    GLuint uProjectionMatrix;
    GLuint mOutlineColorHandle,mOutlineStrengthHandle;
    GLuint mOverrideColorHandle;
    GLuint uEdge,uBorderEdge;
    using INDEX_DATA=GLushort;
    //We only need 1 text texture
    GLuint mTexture;
    static constexpr auto MY_TEXTURE_UNIT=GL_TEXTURE0;
    static constexpr auto MY_SAMPLER_UNIT=0;
    static constexpr int INDEX_BUFFER_SIZE=65535; //max size of GL unsigned short
    // Create one big index buffer that is used to draw all the text vertices
    GLuint mGLIndicesB;
    struct Vertex{
        float x,y;//z is always 0
        float u,v;//texture coordinates
        TrueColor color;// text base color
    };
public:
    //Characters are indexed squares
    struct Character{
        Vertex upperLeft;
        Vertex lowerLeft;
        Vertex lowerRight;
        Vertex upperRight;
    };
    static constexpr const int VERTICES_PER_CHARACTER=4; //2 quads
    static constexpr const int INDICES_PER_CHARACTER=6;
public:
    explicit GLProgramText();
    void loadTextRenderingData(JNIEnv *env, jobject androidContext,const TextAssetsHelper::TEXT_STYLE& textStyle)const;
    void beforeDraw(GLuint buffer) const;
    void beforeDraw(GLBuffer<Character>& buffer)const{
        beforeDraw(buffer.getGLBufferId());
    }
    //Outline with: 0==no outline, 0.2==default outline size
    void updateOutline(const glm::vec3 &outlineColor=glm::vec3(1,1,1), float outlineStrength=0.2f)const;
    void setOtherUniforms(float edge=0.1f,float borderEdge=0.1f)const;
    void draw(const glm::mat4x4& MVPMatrix, int verticesOffset, int numberIndices) const;
    void afterDraw() const;
public:
    // converts the string into a data format that can be read by OpenGL
    // and writes this content into @param array at position @param arrayOffset
    // less safe (array has to be pre-allocated elsewhere) but sometimes needed for performance
    static int convertStringToRenderingData(float X, float Y,float charHeight,
                                            const std::wstring &text, TrueColor color, Character array[], int arrayOffset);

    // same as above, but return a vector with the OpenGL data (more safe regarding pointers)
    static std::vector<Character> convertStringToRenderingData(float X, float Y, float charHeight,
                                            const std::wstring &text, TrueColor color);
    // increases the size of buff accordingly and
    // appends the rendering data at the current position
    static void appendString(std::vector<Character>& buff,float X, float Y,float charHeight, const std::wstring &text, TrueColor color);
    // returns the length in OpenGL coordinates of @param s for @param scale
    static float getStringLength(std::wstring s,float scale);
    static float getFontWidthSafe(int idx);
    static constexpr int ICONS_OFFSET=192;
    static constexpr const wchar_t ICON_BATTERY=(wchar_t)ICONS_OFFSET;
    static constexpr const wchar_t ICON_CHIP=(wchar_t)ICONS_OFFSET+1;
    static constexpr const wchar_t ICON_HOME=(wchar_t)ICONS_OFFSET+2;
    static constexpr const wchar_t ICON_LATITUDE=(wchar_t)ICONS_OFFSET+3;
    static constexpr const wchar_t ICON_LONGITUDE=(wchar_t)ICONS_OFFSET+4;
    static constexpr const wchar_t ICON_SATELITE=(wchar_t)ICONS_OFFSET+5;
    static constexpr const wchar_t ICON_SPACING=(wchar_t)ICONS_OFFSET+6;
    static constexpr const wchar_t ICON_ARTIFICIAL_HORIZON=(wchar_t)ICONS_OFFSET+7;
    // We need specific blend mode(s) when rendering text
    static void setupDepthTestAndBlending(){
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
private:
    static const std::string VS(){
        std::stringstream s;
        s<<"uniform mat4 uProjectionMatrix;\n";
        s<<"attribute vec4 aPosition;\n";
        s<<"attribute vec2 aTexCoord;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"attribute vec4 aVertexColor;\n";
        s<<"varying vec4 vVertexColor;\n";
        s<<"void main() {\n";
        s<<"gl_Position = vec4((uProjectionMatrix * aPosition).xy,0.0,1.0);\n";
        s<<"  vTexCoord = aTexCoord;\n";
        //s<<" gl_Position.zw=vec2(0.0,1.0);";
        s<<"  vVertexColor = aVertexColor;\n";
#ifdef WIREFRAME
        s<<"gl_PointSize=4.0;";
#endif
        s<<"}";
        return s.str();
    }
    static const std::string FS(){
        std::stringstream s;
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"varying vec4 vVertexColor;\n";
        s<<"uniform sampler2D sTexture;\n";
        s<<"const float width = 0.5;";
        s<<"const float smoothing = 1.0/16.0;";
        s<<"const float outlineDistance=0.45;";
        s<<"void main() {\n";
        s<<"float distance = texture2D( sTexture, vTexCoord ).a;\n";
        s<<"float alpha = smoothstep(width-smoothing,width+smoothing,distance);";
        s<<"gl_FragColor = vec4(vVertexColor.rgb,alpha);";
#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        return s.str();
    }
    static const std::string FS2(){
        std::stringstream s;
        s<<"precision mediump float;\n";
        s<<"varying vec2 vTexCoord;\n";
        s<<"varying vec4 vVertexColor;\n";
        s<<"uniform sampler2D sTexture;\n";
        s<<"uniform vec3 uOutlineColor;";
        s<<"const float width=0.5;";
        s<<"uniform float uEdge;";
        s<<"uniform float uBorderEdge;";
        s<<"uniform float uOutlineStrength;";
        //s<<"uniform float uOverrideColor;";

        s<<"void main() {\n";

        s<<"float distance = 1.0 - texture2D( sTexture, vTexCoord ).a;\n";
        s<<"float alpha = 1.0 - smoothstep(width,width+uEdge,distance);";
        s<<"float outlineAlpha = 1.0 - smoothstep(width+uOutlineStrength,width+uOutlineStrength+uBorderEdge,distance);";
        s<<"float overallAlpha= alpha + (1.0 - alpha) * outlineAlpha ;";
        s<<"vec3 overallColor= mix(uOutlineColor,vVertexColor.rgb, alpha / overallAlpha);\n";
        s<<"gl_FragColor=vec4(overallColor*overallAlpha,overallAlpha);";

        //s<<"overallColor+=vec3(uOverrideColor,uOverrideColor,uOverrideColor);";
        //s<<"overallAlpha+=uOverrideColor;";
        // s<<"vec4 fragmentColor;";
        //s<<"fragmentColor=vec4(vVertexColor.rgb*alpha,alpha);";
        //s<<"fragmentColor+=vec4(outlineColor.rgb*outlineAlpha,outlineAlpha);";
        //s<<"gl_FragColor=fragmentColor;";

#ifdef WIREFRAME
        s<<"gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n";
#endif
        s<<"}\n";
        return s.str();
    }
};


#endif //GL_PROGRAM_RENDER_TEXT_H
