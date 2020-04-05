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
add_library( gvr-lib SHARED IMPORTED )
set_target_properties(gvr-lib PROPERTIES IMPORTED_LOCATION
        ${GVR_DIR}/jni/${ANDROID_ABI}/libgvr.so )
# glm is header-only
include_directories(${RX_CORE_EXTERNAL_LIBS}/glm)

##########################################################################################################
#Time & SuperSync
##########################################################################################################
include_directories(${RX_CORE_CPP}/Time)
add_library(Time SHARED
        ${RX_CORE_CPP}/Time/Chronometer.cpp
        ${RX_CORE_CPP}/Time/VRFrameCPUChronometer.cpp
        ${RX_CORE_CPP}/Time/FPSCalculator.cpp)
target_link_libraries( Time ${log-lib} android)
include_directories(${RX_CORE_CPP}/SuperSync)
add_library( SuperSync SHARED
        ${RX_CORE_CPP}/SuperSync/FBRManager.cpp)
target_link_libraries( SuperSync ${log-lib} android Time log EGL GLESv2)

##########################################################################################################
#GLPrograms
##########################################################################################################
include_directories(${RX_CORE_CPP}/Color)
include_directories(${RX_CORE_CPP}/DistortionCorrection)
include_directories(${RX_CORE_CPP}/GeometryBuilder)
include_directories(${RX_CORE_CPP}/GeometryBuilder/Sphere)
include_directories(${RX_CORE_CPP}/GLHelper)
include_directories(${RX_CORE_CPP}/GLPrograms)
include_directories(${RX_CORE_CPP}/Helper)
include_directories(${RX_CORE_CPP}/Other)
add_library( GLPrograms SHARED
        ${RX_CORE_CPP}/DistortionCorrection/DistortionManager.cpp
        ${RX_CORE_CPP}/DistortionCorrection/PolynomialRadialDistortion.cpp
        ${RX_CORE_CPP}/DistortionCorrection/VRHeadsetParams.cpp
        ${RX_CORE_CPP}/DistortionCorrection/MLensDistortion.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramVC.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramText.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramTexture.cpp
        ${RX_CORE_CPP}/GLPrograms/GLProgramLine.cpp)
target_link_libraries( GLPrograms ${log-lib} android GLESv2 gvr-lib)