//
// Created by Consti10 on 14/05/2019.
//

#ifndef FPV_VR_PRIVATE_MDEBUG_H
#define FPV_VR_PRIVATE_MDEBUG_H

#include "android/log.h"
#include <string.h>

#define TAGMDEBUG "GLHelper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAGMDEBUG, __VA_ARGS__)

class MDebug{
public:
    static void log(const std::string& data,const std::string& tag="NoTag"){
        __android_log_print(ANDROID_LOG_DEBUG,tag.c_str(),"%s",data.c_str());
    }
};

#endif //FPV_VR_PRIVATE_MDEBUG_H
