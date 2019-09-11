cmake_minimum_required(VERSION 3.6)

find_library( log-lib
              log )
include_directories(libs/glm)


##########################################################################################################
#Time & SuperSync
##########################################################################################################
include_directories(src/main/cpp/Time)
add_library( Time SHARED
        src/main/cpp/Time/Chronometer.cpp
        src/main/cpp/Time/VRFrameCPUChronometer.cpp
        src/main/cpp/Time/FPSCalculator.cpp)
target_link_libraries( Time ${log-lib} android)
include_directories(src/main/cpp/SuperSync)
add_library( SuperSync SHARED
        src/main/cpp/SuperSync/FBRManager.cpp)
target_link_libraries( SuperSync ${log-lib} android Time log EGL GLESv2)

#gvr is a shared,imported library
#set(GVR_DIR libs/google/gvr)
#include_directories(${GVR_DIR}/headers)
#add_library( gvr-lib SHARED IMPORTED )
#set_target_properties(gvr-lib PROPERTIES IMPORTED_LOCATION
#        ${GVR_DIR}/jni/${ANDROID_ABI}/libgvr.so )

##########################################################################################################
#GLPrograms
##########################################################################################################
include_directories(src/main/cpp/GLPrograms)
add_library( GLPrograms SHARED
             src/main/cpp/GLPrograms/GLProgramVC.cpp
             src/main/cpp/GLPrograms/GLProgramText.cpp
             src/main/cpp/GLPrograms/GLProgramTextureExt.cpp
             src/main/cpp/GLPrograms/GLProgramLine.cpp
             src/main/cpp/GLPrograms/Sphere.cpp
             src/main/cpp/GLPrograms/GLProgramSpherical.cpp
             )
target_link_libraries( GLPrograms ${log-lib} android GLESv2)

##########################################################################################################
#Example Renderer
##########################################################################################################
add_library( example-renderer
             SHARED
        src/main/cpp/example_renderer.cpp
        src/main/cpp/example_supersync.cpp
        src/main/cpp/dummy.cpp
        #src/main/cpp/Other/MatricesManager.cpp
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