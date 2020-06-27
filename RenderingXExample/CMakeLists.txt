cmake_minimum_required(VERSION 3.6)

##########################################################################################################
# include the cmake fle that builds the lib
##########################################################################################################
include(${CMAKE_SOURCE_DIR}/../RenderingXCore/RenderingXCore.cmake)

##########################################################################################################
# build the example renderer
##########################################################################################################
set(BASIC_DEPENDENCIES GLPrograms Time ${log-lib} android log EGL GLESv2)

add_library( example-mono
        SHARED
        src/main/cpp/mono/example_renderer1.cpp
        )
target_link_libraries( example-mono ${BASIC_DEPENDENCIES})

add_library( example-1
        SHARED
        src/main/cpp/stereo/distortion/RendererDistortion.cpp
        ${CMAKE_SOURCE_DIR}/../RenderingXCore/src/main/cpp/Other/gvr_util/util.cc
       )
target_link_libraries( example-1 ${BASIC_DEPENDENCIES} gvr-lib)

add_library( example-2
        SHARED
        src/main/cpp/stereo/video360Degree/Renderer360Video.cpp
        ${CMAKE_SOURCE_DIR}/../RenderingXCore/src/main/cpp/Other/gvr_util/util.cc
        )
target_link_libraries( example-2 ${BASIC_DEPENDENCIES} gvr-lib)


add_library( example-supersync
        SHARED
        src/main/cpp/supersync/example_supersync.cpp
        )
target_link_libraries( example-supersync ${BASIC_DEPENDENCIES} SuperSync)

