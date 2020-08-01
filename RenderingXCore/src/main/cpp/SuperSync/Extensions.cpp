//
// Created by geier on 31/07/2020.
//

#include "Extensions.hpp"
#include <string>


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

void Extensions::initialize(){
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
        Extensions::QCOM_tiled_rendering=true;
        Extensions::glStartTilingQCOM= reinterpret_cast<PFNGLSTARTTILINGQCOMPROC>(eglGetProcAddress("glStartTilingQCOM"));
        Extensions::glEndTilingQCOM = reinterpret_cast<PFNGLENDTILINGQCOMPROC>(eglGetProcAddress("glEndTilingQCOM"));
        assert(Extensions::glStartTilingQCOM!=nullptr && Extensions::glEndTilingQCOM!=nullptr);
    }
    if(ExtensionStringPresent("GL_OES_EGL_sync",glExtensions)){
        Extensions::GL_OES_EGL_sync=true;
        Extensions::eglCreateSyncKHR =  reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(eglGetProcAddress( "eglCreateSyncKHR"));
        Extensions::eglDestroySyncKHR =  reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(eglGetProcAddress( "eglDestroySyncKHR"));
        Extensions::eglClientWaitSyncKHR =  reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(eglGetProcAddress( "eglClientWaitSyncKHR"));
        Extensions::eglSignalSyncKHR =  reinterpret_cast<PFNEGLSIGNALSYNCKHRPROC>(eglGetProcAddress( "eglSignalSyncKHR"));
        Extensions::eglGetSyncAttribKHR =  reinterpret_cast<PFNEGLGETSYNCATTRIBKHRPROC>(eglGetProcAddress( "eglGetSyncAttribKHR"));
        assert(Extensions::eglCreateSyncKHR!=nullptr && Extensions::eglDestroySyncKHR!=nullptr && Extensions::eglClientWaitSyncKHR!=nullptr && Extensions::eglSignalSyncKHR!=nullptr && Extensions::eglGetSyncAttribKHR!=nullptr);
    }
    if(ExtensionStringPresent("GL_KHR_debug",glExtensions)){
        Extensions::KHR_debug=true;
        Extensions::glDebugMessageCallbackKHR =(PFNGLDEBUGMESSAGECALLBACKKHRPROC)eglGetProcAddress("glDebugMessageCallbackKHR");
        Extensions::glGetDebugMessageLogKHR =(PFNGLGETDEBUGMESSAGELOGKHRPROC)eglGetProcAddress("glGetDebugMessageLogKHR");
        assert(Extensions::glDebugMessageCallbackKHR!=nullptr && Extensions::glGetDebugMessageLogKHR!=nullptr);
    }
    if(ExtensionStringPresent("EGL_ANDROID_presentation_time",eglExtensions)){
        Extensions::EGL_ANDROID_presentation_time_available=true;
        Extensions::eglPresentationTimeANDROID=reinterpret_cast<PFNEGLPRESENTATIONTIMEANDROIDPROC>(eglGetProcAddress("eglPresentationTimeANDROID"));
        assert(Extensions::eglPresentationTimeANDROID!=nullptr);
    }
    if(ExtensionStringPresent("GL_EXT_disjoint_timer_query",glExtensions)){
        Extensions::GL_EXT_disjoint_timer_query_available=true;
        Extensions::glGenQueriesEXT_ = (PFNGLGENQUERIESEXTPROC)eglGetProcAddress("glGenQueriesEXT");
        Extensions::glDeleteQueriesEXT_ = (PFNGLDELETEQUERIESEXTPROC)eglGetProcAddress("glDeleteQueriesEXT");
        Extensions::glIsQueryEXT_ = (PFNGLISQUERYEXTPROC)eglGetProcAddress("glIsQueryEXT");
        Extensions::glBeginQueryEXT_ = (PFNGLBEGINQUERYEXTPROC)eglGetProcAddress("glBeginQueryEXT");
        Extensions::glEndQueryEXT_ = (PFNGLENDQUERYEXTPROC)eglGetProcAddress("glEndQueryEXT");
        Extensions::glQueryCounterEXT_ = (PFNGLQUERYCOUNTEREXTPROC)eglGetProcAddress("glQueryCounterEXT");
        Extensions::glGetQueryivEXT_ = (PFNGLGETQUERYIVEXTPROC)eglGetProcAddress("glGetQueryivEXT");
        Extensions::glGetQueryObjectivEXT_ = (PFNGLGETQUERYOBJECTIVEXTPROC)eglGetProcAddress("glGetQueryObjectivEXT");
        Extensions::glGetQueryObjectuivEXT_ = (PFNGLGETQUERYOBJECTUIVEXTPROC)eglGetProcAddress("glGetQueryObjectuivEXT");
        Extensions::glGetQueryObjecti64vEXT_ = (PFNGLGETQUERYOBJECTI64VEXTPROC)eglGetProcAddress("glGetQueryObjecti64vEXT");
        Extensions::glGetQueryObjectui64vEXT_  = (PFNGLGETQUERYOBJECTUI64VEXTPROC)eglGetProcAddress("glGetQueryObjectui64vEXT");
        Extensions::glGetInteger64v_  = (PFNGLGETINTEGER64VAPPLEPROC)eglGetProcAddress("glGetInteger64v");
        assert(Extensions::glGenQueriesEXT_!=nullptr && Extensions::glDeleteQueriesEXT_!=nullptr);
    }
    //other
    Extensions::glInvalidateFramebuffer_  = (Extensions::PFNGLINVALIDATEFRAMEBUFFER_)eglGetProcAddress("glInvalidateFramebuffer");
}

