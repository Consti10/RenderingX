#This file is included in RenderingXExampe//CMakeLists.txt and
#includes & builds the cpp RenderingXCore lib

#It needs the path RX_CORE_DIR to be set to the root of the RenderingXCore module

##########################################################################################################
#Time & SuperSync
##########################################################################################################
include_directories(${RX_CORE_DIR}/src/main/cpp/Time)
add_library(Time SHARED
        ${RX_CORE_DIR}/src/main/cpp/Time/Chronometer.cpp
        ${RX_CORE_DIR}/src/main/cpp/Time/VRFrameCPUChronometer.cpp
        ${RX_CORE_DIR}/src/main/cpp/Time/FPSCalculator.cpp)
target_link_libraries( Time ${log-lib} android)
include_directories(${RX_CORE_DIR}/src/main/cpp/SuperSync)
add_library( SuperSync SHARED
        ${RX_CORE_DIR}/src/main/cpp/SuperSync/FBRManager.cpp)
target_link_libraries( SuperSync ${log-lib} android Time log EGL GLESv2)

##########################################################################################################
#gvr is a shared,imported library
##########################################################################################################
set(GVR_DIR ${RX_CORE_DIR}/libs/google/gvr)
include_directories(${GVR_DIR}/headers)
add_library( gvr-lib SHARED IMPORTED )
set_target_properties(gvr-lib PROPERTIES IMPORTED_LOCATION
        ${GVR_DIR}/jni/${ANDROID_ABI}/libgvr.so )

##########################################################################################################
#GLPrograms
##########################################################################################################
include_directories(${RX_CORE_DIR}/libs/glm)
include_directories(${RX_CORE_DIR}/src/main/cpp/Color)
include_directories(${RX_CORE_DIR}/src/main/cpp/DistortionCorrection)
include_directories(${RX_CORE_DIR}/src/main/cpp/GeometryBuilder)
include_directories(${RX_CORE_DIR}/src/main/cpp/GLHelper)
include_directories(${RX_CORE_DIR}/src/main/cpp/GLPrograms)
include_directories(${RX_CORE_DIR}/src/main/cpp/Helper)
include_directories(${RX_CORE_DIR}/src/main/cpp/Other)
set(RX_CORE_CPP_SPURCE_ROOT ${RX_CORE_DIR}/src/main/cpp)
add_library( GLPrograms SHARED
        ${RX_CORE_CPP_SPURCE_ROOT}/DistortionCorrection/DistortionManager.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/DistortionCorrection/PolynomialRadialDistortion.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/DistortionCorrection/VRHeadsetParams.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/DistortionCorrection/MLensDistortion.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/GLPrograms/GLProgramVC.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/GLPrograms/GLProgramText.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/GLPrograms/GLProgramTexture.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/GLPrograms/GLProgramLine.cpp
        ${RX_CORE_CPP_SPURCE_ROOT}/GeometryBuilder/UvSphere.cpp)
target_link_libraries( GLPrograms ${log-lib} android GLESv2 gvr-lib)