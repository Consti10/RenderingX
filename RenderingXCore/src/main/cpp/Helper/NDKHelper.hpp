//
// Created by Consti10 on 12/10/2019.
//

#ifndef RENDERINGX_NDKHELPER_H
#define RENDERINGX_NDKHELPER_H

#include <jni.h>
#include <string>
#include <vector>
#include <array>
#include <MDebug.hpp>

#include <type_traits>

namespace NDKHelper {
    /**
     * Returns a std::vector whose size depends on the size of the java array
     * and who owns the underlying memory. Not most efficient, but most generic and memory safe
     */
    static std::vector<float> javaArrayToVector(JNIEnv *env, jfloatArray array){
        const size_t size=(size_t)env->GetArrayLength(array);
        std::vector<float> ret(size);
        const auto arrayP=env->GetFloatArrayElements(array, nullptr);
        std::memcpy(ret.data(),&arrayP[0],size*sizeof(float));
        env->ReleaseFloatArrayElements(array,arrayP,0);
        return ret;
    }
    /**
     * Same as above but for int
     */
    static std::vector<int> javaArrayToVector(JNIEnv *env, jintArray array){
        const size_t size=(size_t)env->GetArrayLength(array);
        std::vector<int> ret(size);
        const auto arrayP=env->GetIntArrayElements(array, nullptr);
        std::memcpy(ret.data(),&arrayP[0],size*sizeof(int));
        env->ReleaseIntArrayElements(array,arrayP,0);
        return ret;
    }
    /**
     *  Same as javaArrayToVector, but size is already known at compile time
     * Logs a warning if size!=array size
     */
    template<std::size_t S>
    static std::array<float,S> javaArrayToFixedArray(JNIEnv* env,jfloatArray array){
        const auto data=javaArrayToVector(env,array);
        std::array<float,S> ret;
        if(data.size()!=S){
            LOGD("Error javaArrayToCPP2 : size!=size");
        }else{
            std::memcpy(ret.data(),data.data(),data.size()*sizeof(float));
        }
        return ret;
    }
    //Obtain the asset manager instance from the provided android context object
    static jobject getAssetManagerFromContext(JNIEnv* env,jobject android_context){
        jclass context_class =
                env->FindClass("android/content/Context");
        jmethodID get_asset_manager_method = env->GetMethodID(
                context_class, "getAssets", "()Landroid/content/res/AssetManager;");
        jobject java_asset_mgr=env->CallObjectMethod(android_context,get_asset_manager_method);
        return java_asset_mgr;
    }

    // Returns a java 'InputStream' instance by opening the Asset specified at path
    // If the specified file does not exist, java throws an exception.
    // In this case,the exception is cleared and nullptr is returned
    // @param java_asset_mgr valid java asset manager
    // @param path path to the asset
    static jobject createInputStreamFromAsset(JNIEnv* env,jobject java_asset_mgr,const std::string& path){
        jclass asset_manager_class =
                env->FindClass("android/content/res/AssetManager");
        jmethodID open_method = env->GetMethodID(
                asset_manager_class, "open", "(Ljava/lang/String;)Ljava/io/InputStream;");
        jstring j_path = env->NewStringUTF(path.c_str());
        jobject input_stream =env->CallObjectMethod(java_asset_mgr, open_method, j_path);
        if (env->ExceptionOccurred() != nullptr) {
            MDebug::log("Java exception in createInputStreamFromAsset for file:"+path);
            env->ExceptionClear();
            env->DeleteLocalRef(j_path);
            return nullptr;
        }
        env->DeleteLocalRef(j_path);
        return input_stream;
    }


    // Inspired by gvr_util/util.cc
    // Loads a png file from assets folder and then assigns it to the OpenGL target.
    // This method must be called from the renderer thread since it will result in
    // OpenGL calls to assign the image to the texture target.
    //
    // @param env The JNIEnv to use.
    // @param target, OpenGL texture target to load the image into.
    // @param path, path to the file, relative to the assets folder.
    // @param extractAlpha, upload texture as alpha only (needed for text)
    // @return true if png is loaded correctly, otherwise false.
    static bool LoadPngFromAssetManager(JNIEnv* env, jobject java_asset_mgr, int target,
                                 const std::string& path,const bool extractAlpha=false) {
        jobject image_stream = createInputStreamFromAsset(env,java_asset_mgr,path);
        if(image_stream==nullptr){
            return false;
        }
        jclass bitmap_class =
                env->FindClass("android/graphics/Bitmap");
        jclass bitmap_factory_class =
                env->FindClass("android/graphics/BitmapFactory");
        jclass gl_utils_class = env->FindClass("android/opengl/GLUtils");
        jmethodID extract_alpha_method = env->GetMethodID(
                bitmap_class, "extractAlpha","()Landroid/graphics/Bitmap;");
        jmethodID decode_stream_method = env->GetStaticMethodID(
                bitmap_factory_class, "decodeStream",
                "(Ljava/io/InputStream;)Landroid/graphics/Bitmap;");
        jmethodID tex_image_2d_method = env->GetStaticMethodID(
                gl_utils_class, "texImage2D", "(IILandroid/graphics/Bitmap;I)V");
        jobject image_obj = env->CallStaticObjectMethod(
                bitmap_factory_class, decode_stream_method, image_stream);
        if(extractAlpha){
            image_obj=env->CallObjectMethod(image_obj,extract_alpha_method);
        }
        if (env->ExceptionOccurred() != nullptr) {
            MDebug::log("Java exception while loading image");
            env->ExceptionClear();
            image_obj = nullptr;
            return false;
        }
        env->CallStaticVoidMethod(gl_utils_class, tex_image_2d_method, target, 0,
                                  image_obj, 0);
        return true;
    }
    // Only pass java context instead of asset manager
    static bool LoadPngFromAssetManager2(JNIEnv* env, jobject android_context, int target,
                                 const std::string& path,const bool extractAlpha=false) {
        jobject asset_mgr=getAssetManagerFromContext(env,android_context);
        return LoadPngFromAssetManager(env,asset_mgr,target,path,extractAlpha);
    }

    //Load a java float array that has been serialized from the specified asset location
    //and return its data in cpp-style std::array
    template<std::size_t S>
    static std::array<float,S> getFloatArrayFromAssets2(JNIEnv *env, jobject android_context,const std::string& path){
        jobject asset_mgr=getAssetManagerFromContext(env,android_context);
        jobject input_stream=createInputStreamFromAsset(env,asset_mgr,path);
        if(input_stream==nullptr){
            return std::array<float,S>();
        }
        jclass object_input_stream_class=
                env->FindClass("java/io/ObjectInputStream");
        jmethodID object_input_stream_constructor =env->GetMethodID(object_input_stream_class, "<init>", "(Ljava/io/InputStream;)V");
        jmethodID read_object_method =env->GetMethodID(object_input_stream_class, "readObject", "()Ljava/lang/Object;");
        //Create the Object input stream
        jobject object_input_stream=env->NewObject(object_input_stream_class,object_input_stream_constructor,input_stream);
        //read its underlying instance (float array in our case) then cast
        jobject object_float_array=env->CallObjectMethod(object_input_stream,read_object_method);
        jfloatArray array=(jfloatArray)object_float_array;
        return javaArrayToFixedArray<S>(env,array);
    }

    // return the java name for a simple cpp primitive
    // e.g. float -> F, int -> I
    template <class T> struct cppTypeToNdkName { static const char * const value; };
    template <> constexpr const char *cppTypeToNdkName<int>::value = "I";
    template <> constexpr const char *cppTypeToNdkName<float>::value = "F";
    template <> constexpr const char *cppTypeToNdkName<std::vector<int>>::value = "[I";
    template <> constexpr const char *cppTypeToNdkName<std::vector<float>>::value = "[F";

     // Helper to obtain member values of a java class instance
    class ClassMemberFromJava {
    private:
        JNIEnv* env;
        jclass jclass1;
        jobject jobject1;
        template <class T> jfieldID getFieldId(const char* name){
            const char* ndkTypeName=cppTypeToNdkName<T>::value;
            jfieldID field=env->GetFieldID(jclass1,name,ndkTypeName);
            if(field==nullptr){
                LOGD("cannot find member %s of type %s",name,ndkTypeName);
                env->ExceptionClear();
                return nullptr;
            }
            return field;
        }
    public:
        /**
         * We can obtain the java class (name) from the instance
         * When @param env becomes invalid this instance mustn't be used anymore
         * @param jobject1 instance of the jobject you want to obtain member values from
         */
        ClassMemberFromJava(JNIEnv* env,jobject jobject1){
            this->env=env;
            this->jclass1=env->GetObjectClass(jobject1);
            this->jobject1=jobject1;
        }
        template <typename T>
        struct always_false : std::false_type {};
        /**
         * If a class member with the name @param name is found AND
         * Its type can be translated into a generic cpp type
         * (e.g. java int == cpp int but a special java 'Car' class cannot be translated into cpp)
         * @return the value of the java class member as generic cpp type or 0 when not found
         */
        template <typename T>
        T get(const char *name) {
            jfieldID field=ClassMemberFromJava::getFieldId<T>(name);
            if(field== nullptr)return T();
            //switch case for all supported cpp types
            if constexpr (std::is_same_v<T, float>){
                return env->GetFloatField(jobject1,field);
            }else if constexpr (std::is_same_v<T, int>){
                return env->GetIntField(jobject1,field);
            }else if constexpr (std::is_same_v<T, std::vector<float>>){
                jfloatArray array=(jfloatArray)env->GetObjectField(jobject1,field);
                return NDKHelper::javaArrayToVector(env,array);
            }else{
                static_assert(always_false<T>::value, "Type wrong.");
            }
            return T();
        }
        template<std::size_t S> std::array<float,S> getFloatArrayFixed(const char *name) {
            jfieldID field=ClassMemberFromJava::getFieldId<std::vector<float>>(name);
            if(field== nullptr)return std::array<float,S>();
            jfloatArray array=(jfloatArray)env->GetObjectField(jobject1,field);
            return NDKHelper::javaArrayToFixedArray<S>(env,array);
        }
    };
};


#endif //RENDERINGX_NDKHELPER_H
