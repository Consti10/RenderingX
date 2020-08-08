//
// Created by geier on 31/07/2020.
//

#include "Extensions.h"
#include <string>
#include <jni.h>


bool ExtensionStringPresent(const std::string& extension,const std::string& allExtensions){
    if (allExtensions.find(extension) != std::string::npos) {
        MLOGD<<"Found extension: "<<extension;
        return true;
    }
    MLOGD<<"Cannot find extension: "<<extension;
    return false;
}

//
bool Extensions::QCOM_tiled_rendering=false;
PFNGLSTARTTILINGQCOMPROC Extensions::glStartTilingQCOM=nullptr;
PFNGLENDTILINGQCOMPROC Extensions::glEndTilingQCOM=nullptr;
//
bool Extensions::GL_OES_EGL_sync=false;
PFNEGLCREATESYNCKHRPROC Extensions::eglCreateSyncKHR;
PFNEGLDESTROYSYNCKHRPROC Extensions::eglDestroySyncKHR;
PFNEGLCLIENTWAITSYNCKHRPROC Extensions::eglClientWaitSyncKHR;
PFNEGLSIGNALSYNCKHRPROC Extensions::eglSignalSyncKHR;
PFNEGLGETSYNCATTRIBKHRPROC Extensions::eglGetSyncAttribKHR;
//
bool Extensions::KHR_debug=false;
PFNGLDEBUGMESSAGECALLBACKKHRPROC Extensions::glDebugMessageCallbackKHR=nullptr;
PFNGLGETDEBUGMESSAGELOGKHRPROC Extensions::glGetDebugMessageLogKHR= nullptr;
//other
Extensions::PFNGLINVALIDATEFRAMEBUFFER_	Extensions::glInvalidateFramebuffer_;
//
bool Extensions::EGL_ANDROID_presentation_time_available;
PFNEGLPRESENTATIONTIMEANDROIDPROC Extensions::eglPresentationTimeANDROID;
//
bool Extensions::GL_EXT_disjoint_timer_query_available;
PFNGLGENQUERIESEXTPROC Extensions::glGenQueriesEXT_;
PFNGLDELETEQUERIESEXTPROC Extensions::glDeleteQueriesEXT_;
PFNGLISQUERYEXTPROC Extensions::glIsQueryEXT_;
PFNGLBEGINQUERYEXTPROC Extensions::glBeginQueryEXT_;
PFNGLENDQUERYEXTPROC Extensions::glEndQueryEXT_;
PFNGLQUERYCOUNTEREXTPROC Extensions::glQueryCounterEXT_;
PFNGLGETQUERYIVEXTPROC Extensions::glGetQueryivEXT_;
PFNGLGETQUERYOBJECTIVEXTPROC Extensions::glGetQueryObjectivEXT_;
PFNGLGETQUERYOBJECTUIVEXTPROC Extensions::glGetQueryObjectuivEXT_;
PFNGLGETQUERYOBJECTI64VEXTPROC Extensions::glGetQueryObjecti64vEXT_;
PFNGLGETQUERYOBJECTUI64VEXTPROC Extensions::glGetQueryObjectui64vEXT_;
PFNGLGETINTEGER64VAPPLEPROC Extensions::glGetInteger64v_;
//
bool Extensions::EGL_ANDROID_get_frame_timestamps_available;
PFNEGLGETNEXTFRAMEIDANDROIDPROC Extensions::eglGetNextFrameIdANDROID ;
PFNEGLGETFRAMETIMESTAMPSANDROIDPROC Extensions::eglGetFrameTimestampsANDROID;
PFNEGLGETCOMPOSITORTIMINGANDROIDPROC Extensions::eglGetCompositorTimingANDROID;
PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC Extensions::eglGetFrameTimestampSupportedANDROID;

void Extensions::initializeGL(){
    const char* glExtensionsC=(const char*)glGetString(GL_EXTENSIONS);
    if (glExtensionsC==nullptr){
        MLOGE<<"glGetString(GL_EXTENSIONS) returned NULL";
        return;
    }else{
        MLOGD<<"glGetString(GL_EXTENSIONS) not NULL";
    }
    const char* eglExtensionsC=(const char*) eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
    if(eglExtensionsC!=nullptr){
        MLOGD<<"All EGL extensions:"<<std::string(eglExtensionsC);
    }
    const std::string glExtensions=std::string(glExtensionsC);
    const std::string eglExtensions=std::string(eglExtensionsC);
    MLOGD<<"All extensions "<<glExtensions;
    if(ExtensionStringPresent("GL_QCOM_tiled_rendering",glExtensions)){
        QCOM_tiled_rendering=true;
        glStartTilingQCOM= reinterpret_cast<PFNGLSTARTTILINGQCOMPROC>(eglGetProcAddress("glStartTilingQCOM"));
        glEndTilingQCOM = reinterpret_cast<PFNGLENDTILINGQCOMPROC>(eglGetProcAddress("glEndTilingQCOM"));
        assert(glStartTilingQCOM!=nullptr && glEndTilingQCOM!=nullptr);
    }
    if(ExtensionStringPresent("GL_OES_EGL_sync",glExtensions)){
        GL_OES_EGL_sync=true;
        eglCreateSyncKHR =  reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(eglGetProcAddress( "eglCreateSyncKHR"));
        eglDestroySyncKHR =  reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(eglGetProcAddress( "eglDestroySyncKHR"));
        eglClientWaitSyncKHR =  reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(eglGetProcAddress( "eglClientWaitSyncKHR"));
        eglSignalSyncKHR =  reinterpret_cast<PFNEGLSIGNALSYNCKHRPROC>(eglGetProcAddress( "eglSignalSyncKHR"));
        eglGetSyncAttribKHR =  reinterpret_cast<PFNEGLGETSYNCATTRIBKHRPROC>(eglGetProcAddress( "eglGetSyncAttribKHR"));
        assert(eglCreateSyncKHR!=nullptr && eglDestroySyncKHR!=nullptr && eglClientWaitSyncKHR!=nullptr && eglSignalSyncKHR!=nullptr && eglGetSyncAttribKHR!=nullptr);
    }
    if(ExtensionStringPresent("GL_KHR_debug",glExtensions)){
        KHR_debug=true;
        glDebugMessageCallbackKHR =(PFNGLDEBUGMESSAGECALLBACKKHRPROC)eglGetProcAddress("glDebugMessageCallbackKHR");
        glGetDebugMessageLogKHR =(PFNGLGETDEBUGMESSAGELOGKHRPROC)eglGetProcAddress("glGetDebugMessageLogKHR");
        assert(glDebugMessageCallbackKHR!=nullptr && glGetDebugMessageLogKHR!=nullptr);
    }
    if(ExtensionStringPresent("EGL_ANDROID_presentation_time",eglExtensions)){
        EGL_ANDROID_presentation_time_available=true;
        eglPresentationTimeANDROID=reinterpret_cast<PFNEGLPRESENTATIONTIMEANDROIDPROC>(eglGetProcAddress("eglPresentationTimeANDROID"));
        assert(eglPresentationTimeANDROID!=nullptr);
    }
    if(ExtensionStringPresent("GL_EXT_disjoint_timer_query",glExtensions)){
        GL_EXT_disjoint_timer_query_available=true;
        glGenQueriesEXT_ = (PFNGLGENQUERIESEXTPROC)eglGetProcAddress("glGenQueriesEXT");
        glDeleteQueriesEXT_ = (PFNGLDELETEQUERIESEXTPROC)eglGetProcAddress("glDeleteQueriesEXT");
        glIsQueryEXT_ = (PFNGLISQUERYEXTPROC)eglGetProcAddress("glIsQueryEXT");
        glBeginQueryEXT_ = (PFNGLBEGINQUERYEXTPROC)eglGetProcAddress("glBeginQueryEXT");
        glEndQueryEXT_ = (PFNGLENDQUERYEXTPROC)eglGetProcAddress("glEndQueryEXT");
        glQueryCounterEXT_ = (PFNGLQUERYCOUNTEREXTPROC)eglGetProcAddress("glQueryCounterEXT");
        glGetQueryivEXT_ = (PFNGLGETQUERYIVEXTPROC)eglGetProcAddress("glGetQueryivEXT");
        glGetQueryObjectivEXT_ = (PFNGLGETQUERYOBJECTIVEXTPROC)eglGetProcAddress("glGetQueryObjectivEXT");
        glGetQueryObjectuivEXT_ = (PFNGLGETQUERYOBJECTUIVEXTPROC)eglGetProcAddress("glGetQueryObjectuivEXT");
        glGetQueryObjecti64vEXT_ = (PFNGLGETQUERYOBJECTI64VEXTPROC)eglGetProcAddress("glGetQueryObjecti64vEXT");
        glGetQueryObjectui64vEXT_  = (PFNGLGETQUERYOBJECTUI64VEXTPROC)eglGetProcAddress("glGetQueryObjectui64vEXT");
        glGetInteger64v_  = (PFNGLGETINTEGER64VAPPLEPROC)eglGetProcAddress("glGetInteger64v");
        assert(glGenQueriesEXT_!=nullptr && glDeleteQueriesEXT_!=nullptr);
    }
    if(ExtensionStringPresent("EGL_ANDROID_get_frame_timestamps",eglExtensions)){
        EGL_ANDROID_get_frame_timestamps_available=true;
        eglGetNextFrameIdANDROID = reinterpret_cast<PFNEGLGETNEXTFRAMEIDANDROIDPROC>(eglGetProcAddress("eglGetNextFrameIdANDROID"));
        eglGetFrameTimestampsANDROID = reinterpret_cast<PFNEGLGETFRAMETIMESTAMPSANDROIDPROC>(eglGetProcAddress("eglGetFrameTimestampsANDROID"));
        eglGetCompositorTimingANDROID= reinterpret_cast<PFNEGLGETCOMPOSITORTIMINGANDROIDPROC >(eglGetProcAddress("eglGetCompositorTimingANDROID"));
        eglGetFrameTimestampSupportedANDROID=reinterpret_cast<PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC>(eglGetProcAddress("eglGetFrameTimestampSupportedANDROID"));
        assert(eglGetNextFrameIdANDROID!=nullptr && eglGetFrameTimestampsANDROID!=nullptr);
    }
    //other
    glInvalidateFramebuffer_  = (PFNGLINVALIDATEFRAMEBUFFER_)eglGetProcAddress("glInvalidateFramebuffer");
}

extern "C" {

void Java_constantin_renderingx_core_Extensions_nativeSetThreadAffinity(JNIEnv *env, jclass jclass1,jint core) {
CPUAffinityHelper::setAffinity(core);
}

}