//
// Created by Consti10 on 14/05/2019.
//

#ifndef FPV_VR_PRIVATE_MDEBUG_H
#define FPV_VR_PRIVATE_MDEBUG_H

#include "android/log.h"
#include <string.h>
#include <sstream>

//#define TAG_MDEBUG "MDebug"
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG_MDEBUG, __VA_ARGS__)

/*namespace LOG{
    // taken from https://android.googlesource.com/platform/system/core/+/android-2.1_r1/liblog/logd_write.c
    static constexpr const auto ANDROID_LOG_BUFF_SIZE=1024;
    static constexpr auto DEFAULT_TAG="NoTag";
    static void D(const char* fmt,...)__attribute__((__format__(printf, 1, 2))) {
        va_list argptr;
        va_start(argptr, fmt);
        __android_log_vprint(ANDROID_LOG_DEBUG,DEFAULT_TAG,fmt,argptr);
        va_end(argptr);
    }
    static void E(const char* fmt,...)__attribute__((__format__(printf, 1, 2))) {
        va_list argptr;
        va_start(argptr, fmt);
        __android_log_vprint(ANDROID_LOG_ERROR,DEFAULT_TAG,fmt,argptr);
        va_end(argptr);
    }
    static void LOL(const char* fmt,...) {
        const auto ANDROID_LOG_BUFF_SIZE=1024;
        char buffer[ANDROID_LOG_BUFF_SIZE];
        va_list argptr;
        va_start(argptr, fmt);
        vsprintf (buffer,fmt, argptr);
        vsnprintf(buffer,ANDROID_LOG_BUFF_SIZE, fmt, argptr);
        va_end(argptr);
        __android_log_print(ANDROID_LOG_DEBUG,"HA","%s",buffer);
    }
}*/

class LOG2{
public:
    LOG2(const std::string& TAG="NoTag"):TAG(TAG) {
        uncaught = std::uncaught_exceptions();
    }
    ~LOG2() {
        if (uncaught >= std::uncaught_exceptions()) {
            logBigMessage(stream.str());
        }
    }
private:
    std::stringstream stream;
    int uncaught;
    const std::string TAG;
    // taken from https://android.googlesource.com/platform/system/core/+/android-2.1_r1/liblog/logd_write.c
    static constexpr const auto ANDROID_LOG_BUFF_SIZE=1024;
    //Splits debug messages that exceed the android log maximum length into smaller log(s)
    //Recursive declaration
    void logBigMessage(const std::string& message){
        if(message.length()>ANDROID_LOG_BUFF_SIZE){
            __android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"%s",message.substr(0,ANDROID_LOG_BUFF_SIZE).c_str());
            logBigMessage(message.substr(ANDROID_LOG_BUFF_SIZE));
        }else{
            __android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"%s",message.c_str());
        }
    }
    // the non-member function operator<< will now have access to private members
    template <typename T>
    friend LOG2& operator<<(LOG2& record, T&& t);
};
template <typename T>
LOG2& operator<<(LOG2& record, T&& t) {
    record.stream << std::forward<T>(t);
    return record;
}
template <typename T>
LOG2& operator<<(LOG2&& record, T&& t) {
    return record << std::forward<T>(t);
}

// print some example LOGs
namespace TEST_LOGGING_ON_ANDROID{
    /*static void test(){
        LOG::D("TestText %d",1);
        LOG::D("TestText %d",1);
        const char*LOL="LOL";
        LOG::D("TestText %s",LOL);
        LOG::LOL("HALLO %s %d",LOL,2);
    }*/
    static void test2(){
        __android_log_print(ANDROID_LOG_DEBUG,"TAG","Before");
        LOG2("MyTAG")<<"Hello World I "<<1<<" F "<<0.0f<<" X";
        __android_log_print(ANDROID_LOG_DEBUG,"TAG","After");
    }
}

#endif //FPV_VR_PRIVATE_MDEBUG_H
