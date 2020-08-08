#include "VSYNC.h"
#include <android/choreographer.h>

static void refreshRateCallback(int64_t vsyncPeriodNanos, void *data) {
    MLOGD<<"Refresh rate is"<<MyTimeHelper::ReadableNS(vsyncPeriodNanos);
}
static void frameCallback(int64_t frameTimeNanos, void* data) {
    MLOGD<<"Frame callback"<<MyTimeHelper::ReadableNS(frameTimeNanos);
}

static void test(){
    //auto choreographer=AChoreographer_getInstance();
    //AChoreographer_registerRefreshRateCallback(choreographer,refreshRateCallback,nullptr);
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

JNI_METHOD(void, nativeSetVSYNCSentByChoreographer)
(JNIEnv *env, jobject obj, jlong p,jlong value) {
    VSYNC::native(p)->setVSYNCSentByChoreographer((int64_t)value);
}

}