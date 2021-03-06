
#include <NDKHelper.hpp>
#include "GLProgramTexture.h"

constexpr auto TAG="GLProgramTexture(Ext)";
constexpr auto GL_TEXTURE_EXTERNAL_OES=0x00008d65;

TexturedStereoMeshData TexturedStereoVertexHelper::convert(const TexturedMeshData &input) {
    std::vector<TexturedStereoVertex> vertices;
    for(const auto& vertex:input.vertices){
        vertices.push_back({vertex.x,vertex.y,vertex.z,vertex.u,vertex.v,vertex.u,vertex.v});
    }
    if(input.hasIndices()){
        return TexturedStereoMeshData(vertices,*input.indices,input.mode);
    }
    return TexturedStereoMeshData(vertices,input.mode);
}

TexturedMeshData
TexturedStereoVertexHelper::convert(const TexturedStereoMeshData &input, const bool left) {
    std::vector<TexturedVertex> vertices;
    for(const auto& vertex:input.vertices){
        if(left){
            vertices.push_back({vertex.x,vertex.y,vertex.z,vertex.u_left,vertex.v_left});
        }else{
            vertices.push_back({vertex.x,vertex.y,vertex.z,vertex.u_right,vertex.v_right});
        }
    }
    if(input.hasIndices()){
        return TexturedMeshData (vertices,*input.indices,input.mode);
    }
    return TexturedMeshData(vertices,input.mode);
}


AGLProgramTexture::AGLProgramTexture(const bool USE_EXTERNAL_TEXTURE,const bool ENABLE_VDDC, const bool USE_2D_COORDINATES, const bool mapEquirectangularToInsta360)
        : USE_EXTERNAL_TEXTURE(USE_EXTERNAL_TEXTURE), ENABLE_VDDC(ENABLE_VDDC),USE_2D_COORDINATES(USE_2D_COORDINATES), MAP_EQUIRECTANGULAR_TO_INSTA360(mapEquirectangularToInsta360) {
    // cannot enable VDDC and 2D coordinates at the same time
    assert(!((ENABLE_VDDC == true) && (USE_2D_COORDINATES == true)));
    std::string flags;
    if(ENABLE_VDDC){
        flags+="#define ENABLE_VDDC\n";
    }else if(USE_2D_COORDINATES){
        flags+="#define USE_2D_COORDINATES\n";
    }
    if(USE_EXTERNAL_TEXTURE)flags+="#define USE_EXTERNAL_TEXTURE\n";
    mProgram = GLHelper::createProgram(VS(), FS(mapEquirectangularToInsta360), flags);
    mMVMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uMVMatrix");
    mPMatrixHandle=GLHelper::GlGetUniformLocation(mProgram,"uPMatrix");
    mPositionHandle = GLHelper::GlGetAttribLocation(mProgram, "aPosition");
    mTextureHandle = GLHelper::GlGetAttribLocation(mProgram, "aTexCoord");
    mSamplerHandle = GLHelper::GlGetUniformLocation (mProgram, "sTexture" );
    if(ENABLE_VDDC){
        mUndistortionHandles=VDDC::getUndistortionUniformHandles(mProgram);
    }
    GLHelper::checkGlError(TAG);
}

void AGLProgramTexture::beforeDraw(const GLuint buffer, GLuint texture) const{
    glUseProgram((GLuint)mProgram);
    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,texture);
    glUniform1i(mSamplerHandle,MY_SAMPLER_UNIT);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), nullptr);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,sizeof(TexturedVertex),(GLvoid*)offsetof(TexturedVertex,u));
    //MLOGD<<"LOL USE_EXTERNAL_TEXTURE "<<USE_EXTERNAL_TEXTURE<<"ENABLE_VDDC "<<ENABLE_VDDC<<" USE_2D_COORDINATES "<<USE_2D_COORDINATES;
}

void AGLProgramTexture::draw(const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const int verticesOffset, const int numberVertices, GLenum mode) const{
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    glDrawArrays(mode, verticesOffset, numberVertices);

}

void AGLProgramTexture::drawIndexed(GLuint indexBuffer, const glm::mat4x4 &ViewM,
                                    const glm::mat4x4 &ProjM, int indicesOffset, int numberIndices,
                                    GLenum mode) const {
    glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
    glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(mode,numberIndices,GL_UNSIGNED_INT, (void*)(indicesOffset*sizeof(INDEX_DATA)));
}

void AGLProgramTexture::afterDraw() const{
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,0);
}

void AGLProgramTexture::loadTexture(GLuint texture, JNIEnv *env, jobject androidContext, const char *name) {
    //Load texture, generate mipmaps, set sampling parameters
    glBindTexture(GL_TEXTURE_2D,texture);
    NDKHelper::LoadPngFromAssetManager2(env,androidContext,GL_TEXTURE_2D,name);
    glHint(GL_GENERATE_MIPMAP_HINT,GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AGLProgramTexture::drawX(GLuint texture, const glm::mat4x4 &ViewM, const glm::mat4x4 &ProjM,
                              const TexturedGLMeshBuffer &mesh)const {
    mesh.logWarningWhenDrawingMeshWithoutData();
    beforeDraw(mesh.getVertexBufferId(), texture);
    if(mesh.hasIndices()){
        drawIndexed(mesh.getIndexBufferId(), ViewM, ProjM, 0, mesh.getCount(), mesh.getMode());
    }else{
        draw(ViewM,ProjM,0,mesh.getCount(),mesh.getMode());
    }
    afterDraw();
}

void AGLProgramTexture::updateUnDistortionUniforms(bool leftEye, const VDDC::DataUnDistortion &dataUnDistortion) const {
    if(!ENABLE_VDDC){
        MLOGE<<"called GLProgramTexture::updateUnDistortion with VDDC disabled";
        return;
    }
    glUseProgram(mProgram);
    VDDC::updateUnDistortionUniforms(leftEye, *mUndistortionHandles, dataUnDistortion);
    //MLOGD<<"GLPT"<<MLensDistortion::ViewportParamsNDCAsString(dataUnDistortion.screen_params[0],dataUnDistortion.texture_params[0]);
}

void AGLProgramTexture::beforeDrawStereoVertex(GLuint buffer, GLuint texture, bool useLeftTextureCoords) const {
    glUseProgram((GLuint)mProgram);
    glActiveTexture(MY_TEXTURE_UNIT);
    glBindTexture(USE_EXTERNAL_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,texture);
    glUniform1i(mSamplerHandle,MY_SAMPLER_UNIT);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*xyz*/, GL_FLOAT, GL_FALSE, sizeof(TexturedStereoVertex), nullptr);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    if(useLeftTextureCoords){
        glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/, GL_FLOAT, GL_FALSE, sizeof(TexturedStereoVertex), (GLvoid*)offsetof(TexturedStereoVertex, u_left));
    }else{
        glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/, GL_FLOAT, GL_FALSE, sizeof(TexturedStereoVertex), (GLvoid*)offsetof(TexturedStereoVertex, u_right));
    }
}

void AGLProgramTexture::drawXStereoVertex(GLuint texture, const glm::mat4x4& ViewM, const glm::mat4x4& ProjM, const TexturedStereoGLMeshBuffer& mesh, bool useLeftTextureCoords)const {
    mesh.logWarningWhenDrawingMeshWithoutData();
    beforeDrawStereoVertex(mesh.getVertexBufferId(),texture,useLeftTextureCoords);
    if(mesh.hasIndices()){
        drawIndexed(mesh.getIndexBufferId(), ViewM, ProjM, 0, mesh.getCount(), mesh.getMode());
    }else{
        draw(ViewM,ProjM,0,mesh.getCount(),mesh.getMode());
    }
    afterDraw();
}

