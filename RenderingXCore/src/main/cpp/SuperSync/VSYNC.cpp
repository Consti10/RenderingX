#include "VSYNC.h"
#include <jni.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_renderingx_core_VSYNC_##method_name

inline jlong jptr(VSYNC *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline VSYNC *native(jlong ptr) {
    return reinterpret_cast<VSYNC*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj) {
    return jptr(new VSYNC());
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong p) {
delete native(p);
}

JNI_METHOD(void, nativeSetVSYNCSentByChoreographer)
(JNIEnv *env, jobject obj, jlong p,jlong value) {
    native(p)->setVSYNCSentByChoreographer((int64_t)value);
}

}