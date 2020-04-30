//
// Created by geier on 30/04/2020.
//

#ifndef RENDERINGX_NDKARRAYHELPER_H
#define RENDERINGX_NDKARRAYHELPER_H

#include <jni.h>
#include <vector>
#include <type_traits>

//
// The purpose of this namespace is to make it easier to transfer arrays of generic data types
// (Data types that are both used by cpp and java, like int, float ) via the NDK from java to cpp
// Only dependencies are standard libraries and the android java NDK
//
namespace NDKArrayHelper{
    // workaround from https://en.cppreference.com/w/cpp/language/if#Constexpr_If
    // To have compile time type safety
    template <typename T,typename T2>
    struct always_false : std::false_type {};
    /**
    * Returns a std::vector whose size depends on the size of the java array
    * and which owns the underlying memory. Most generic and memory safe.
    * Function has compile time type safety, see example() below
    */
    template <class T,class T2>
    static std::vector<T> DynamicSizeArray(JNIEnv *env,T2 array){
        const size_t size=(size_t)env->GetArrayLength(array);
        std::vector<T> ret(size);
        if constexpr (std::is_same_v<T,float> && std::is_same_v<T2,jfloatArray>){
            auto arrayP=env->GetFloatArrayElements(array, nullptr);
            std::memcpy(ret.data(),arrayP,size*sizeof(T));
            env->ReleaseFloatArrayElements(array,arrayP,0);
        }else if constexpr (std::is_same_v<T,int> && std::is_same_v<T2,jintArray>){
            auto arrayP=env->GetIntArrayElements(array, nullptr);
            std::memcpy(ret.data(),arrayP,size*sizeof(T));
            env->ReleaseIntArrayElements(array,arrayP,0);
        }else if constexpr (std::is_same_v<T,bool>  && std::is_same_v<T2,jbooleanArray>){
            auto arrayP=env->GetBooleanArrayElements(array, nullptr);
            std::memcpy(ret.data(),arrayP,size*sizeof(T));
            env->ReleaseBooleanArrayElements(array,arrayP,0);
        }else if constexpr (std::is_same_v<T,double> && std::is_same_v<T2,jdoubleArray >){
            auto arrayP=env->GetDoubleArrayElements(array, nullptr);
            std::memcpy(ret.data(),arrayP,size*sizeof(T));
            env->ReleaseDoubleArrayElements(array,arrayP,0);
        }else{
            // a) Make sure you use the right combination.
            // For example, if you want a std::vector<float> pass a jfloatArray as second parameter
            // b) Make sure you use a supported type. (e.g. one that appears in the above if - else)
            static_assert(always_false<T,T2>::value, "Unsupported Combination / Type");
        }
        return ret;
    }
    /**
     *  Whenever size is already known at compile time u can use this one
     *  but note that it is impossible to check at compile time if the java array has the same size
     *  Assert at run time if size!=array size
     */
    template<class T,std::size_t S,class T2>
    static std::array<T,S> FixedSizeArray(JNIEnv* env,T2 array){
        const auto data= DynamicSizeArray<T>(env, array);
        std::array<T,S> ret;
        assert(data.size()==S);
        std::memcpy(ret.data(),data.data(),data.size()*sizeof(T));
        return ret;
    }
    // Demonstrate the type safety of DynamicSizeArray:
    // X Compiles, but Y does not (which is exactly what we want)
    static void example(){
        JNIEnv* env= nullptr;
        jfloatArray array= nullptr;
        std::vector<float> X=DynamicSizeArray<float>(env,array);
        // This one does not compile - we cannot get a int array from a java float array
        //std::vector<int> Y=DynamicSizeArray<int>(env,array);
    }
}

#endif //RENDERINGX_NDKARRAYHELPER_H
