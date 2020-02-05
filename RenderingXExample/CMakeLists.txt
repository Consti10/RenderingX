cmake_minimum_required(VERSION 3.6)

find_library( log-lib
              log )

set(RX_CORE_DIR ${CMAKE_SOURCE_DIR}/../RenderingXCore)


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

#gvr is a shared,imported library
set(GVR_DIR ${RX_CORE_DIR}/libs/google/gvr)
include_directories(${GVR_DIR}/headers)
add_library( gvr-lib SHARED IMPORTED )
set_target_properties(gvr-lib PROPERTIES IMPORTED_LOCATION
        ${GVR_DIR}/jni/${ANDROID_ABI}/libgvr.so )

##########################################################################################################
#GLPrograms
##########################################################################################################
include_directories(${RX_CORE_DIR}/libs/glm)
include_directories(${RX_CORE_DIR}/src/main/cpp)
include_directories(${RX_CORE_DIR}/src/main/cpp/Color)
include_directories(${RX_CORE_DIR}/src/main/cpp/GLPrograms)
include_directories(${RX_CORE_DIR}/src/main/cpp/GeometryBuilder)
include_directories(${RX_CORE_DIR}/src/main/cpp/Helper)
include_directories(${RX_CORE_DIR}/src/main/cpp/Other)
add_library( GLPrograms SHARED
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/DistortionManager.cpp
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/PolynomialRadialDistortion.cpp
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/VRHeadsetParams.cpp
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/MLensDistortion.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramVC.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramText.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramTexture.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramLine.cpp
        ${RX_CORE_DIR}/src/main/cpp/GeometryBuilder/GvrSphere.cpp)
target_link_libraries( GLPrograms ${log-lib} android GLESv2 gvr-lib)

##########################################################################################################
#Example Renderer
##########################################################################################################
add_library( example-renderer
        SHARED
        src/main/cpp/example1/example_renderer1.cpp
        src/main/cpp/example_supersync.cpp
        )
target_link_libraries( example-renderer
        GLPrograms
        Time
        ${log-lib}
        android
        log
        EGL
        GLESv2
        SuperSync
        #gvr-lib
        )

add_library( example-renderer2
        SHARED
        src/main/cpp/example2/example_distortion.cpp
        ${RX_CORE_DIR}/src/main/cpp/Other/gvr_util/util.cc
        )
target_link_libraries( example-renderer2
        GLPrograms
        Time
        ${log-lib}
        android
        log
        EGL
        GLESv2
        SuperSync
        gvr-lib
        )

