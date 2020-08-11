#include "VSYNC.h"
#include <android/choreographer.h>

static bool resumed=false;

static void refreshRateCallback(int64_t vsyncPeriodNanos, void *data) {
    MLOGD<<"Refresh rate is"<<MyTimeHelper::ReadableNS(vsyncPeriodNanos);
}
static void frameCallback(long frameTimeNanos, void* data) {
    MLOGD<<"Frame callback A:"<<frameTimeNanos;
    reinterpret_cast<VSYNC*>(data)->setVSYNCSentByChoreographer(frameTimeNanos);
    if(resumed){
        //AChoreographer_postFrameCallback(AChoreographer_getInstance(),frameCallback,data);
    }
}

static void test(){
    //auto choreographer=AChoreographer_getInstance();
    //AChoreographer_registerRefreshRateCallback(choreographer,refreshRateCallback,nullptr);
    //AChoreographer_refreshRateCallback* p;
    //AChoreographer_postFrameCallback64(choreographer,frameCallback,nullptr);
}

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_core_VSYNC_##method_name


extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj) {
    return VSYNC::jptr(new VSYNC());
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
delete VSYNC::native(p);
}

/*JNI_METHOD(void, nativeResume)
(JNIEnv *env, jobject obj, jlong p) {
    resumed=true;
    auto choreographer=AChoreographer_getInstance();
    AChoreographer_postFrameCallback(choreographer,frameCallback,(void*)p);
}
JNI_METHOD(void, nativePause)
(JNIEnv *env, jobject obj, jlong p) {
    resumed=false;
}*/


JNI_METHOD(void, nativeSetVSYNCSentByChoreographer)
(JNIEnv *env, jobject obj, jlong p,jlong value) {
    //MLOGD<<"Frame callback B:"<<value;
    VSYNC::native(p)->setVSYNCSentByChoreographer((int64_t)value);
}

}