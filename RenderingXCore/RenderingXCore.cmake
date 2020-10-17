##########################################################################################################
# This file is included in RenderingXExampe/CMakeLists.txt and
# includes & builds the cpp RenderingXCore lib
##########################################################################################################

find_library( log-lib
        log )

#Set the path where all the native (cpp) code is
set(RX_CORE_CPP ${CMAKE_CURRENT_LIST_DIR}/src/main/cpp)
#set the path where external libs are stored (google vr and glm)
set(RX_CORE_EXTERNAL_LIBS ${CMAKE_CURRENT_LIST_DIR}/libs)

##########################################################################################################
# First include all external libraries
##########################################################################################################
# gvr is a shared,imported library
set(GVR_DIR ${RX_CORE_EXTERNAL_LIBS}/google/gvr)
include_directories(${GVR_DIR}/headers)
add_library(gvr-lib SHARED IMPORTED )
set_target_properties(gvr-lib PROPERTIES IMPORTED_LOCATION
        ${GVR_DIR}/jni/${ANDROID_ABI}/libgvr.so )
# glm is header-only
include_directories(${RX_CORE_EXTERNAL_LIBS}/glm)

##########################################################################################################
#Time & SuperSync & VrSettings
##########################################################################################################
include_directories(${RX_CORE_CPP}/Time)

include_directories(${RX_CORE_CPP}/SuperSync)
add_library(Extensions SHARED
        ${RX_CORE_CPP}/SuperSync/Extensions.cpp
        )
target_link_libraries( Extensions ${log-lib} android EGL GLESv2)

include_directories(${RX_CORE_CPP}/VRSettings)
add_library(VRSettings SHARED
        ${RX_CORE_CPP}/VRSettings/VRSettings.cpp)
target_link_libraries(VRSettings ${log-lib} android)


##########################################################################################################
#GLPrograms
##########################################################################################################
include_directories(${RX_CORE_CPP}/Color)
include_directories(${RX_CORE_CPP}/DistortionCorrection)
include_directories(${RX_CORE_CPP}/DistortionCorrection/PolynomialRadialDistortion)
include_directories(${RX_CORE_CPP}/GeometryBuilder)
include_directories(${RX_CORE_CPP}/GeometryBuilder/Sphere)
include_directories(${RX_CORE_CPP}/GLHelper)
include_directories(${RX_CORE_CPP}/GLPrograms)

set(DIR_Shared ${CMAKE_CURRENT_LIST_DIR}/../../LiveVideo10ms/Shared/src/main/cpp)

include_directories(${DIR_Shared}/NDKHelper)
include_directories(${DIR_Shared}/Helper)

include_directories(${RX_CORE_CPP}/Other)
add_library( GLPrograms SHARED
        ${RX_CORE_CPP}/DistortionCorrection/PolynomialRadialDistortion/PolynomialRadialDistortion.cpp
        ${RX_CORE_CPP}/DistortionCorrection/PolynomialRadialDistortion/PolynomialRadialInverse.cpp
        ${RX_CORE_CPP}/DistortionCorrection/LensDistortion/MLensDistortion.cpp
        ${RX_CORE_CPP}/DistortionCorrection/VrCompositorRenderer.cpp
        #${RX_CORE_CPP}/DistortionCorrection/VDDC.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramVC.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramText.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramTexture.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramLine.cpp
        ${RX_CORE_CPP}/GLPrograms/ProjTex/GLPTextureProj.cpp
        ${RX_CORE_CPP}/GLPrograms/ProjTex/GLPTextureProj2.cpp
        )
target_link_libraries( GLPrograms ${log-lib} android GLESv2 gvr-lib Extensions VRSettings)

#
include_directories(${RX_CORE_CPP}/SuperSync)
add_library( SuperSync SHARED
        ${RX_CORE_CPP}/SuperSync/VSYNC.cpp
        ${RX_CORE_CPP}/SuperSync/FBRManager.cpp)
target_link_libraries( SuperSync ${log-lib} android log EGL GLESv2 Extensions GLPrograms)
#