cmake_minimum_required(VERSION 3.6)

find_library( log-lib
              log )

set(RX_CORE_DIR ${CMAKE_SOURCE_DIR}/../RenderingXCore)

#include the cmake fle that builds the lib
include(${RX_CORE_DIR}/RenderingXCore.cmake)

##########################################################################################################
# build the example renderer
##########################################################################################################
add_library( example-renderer
        SHARED
        src/main/cpp/Mono/example_renderer1.cpp
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
        src/main/cpp/StereoVR/ExampleRendererVR.cpp
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

