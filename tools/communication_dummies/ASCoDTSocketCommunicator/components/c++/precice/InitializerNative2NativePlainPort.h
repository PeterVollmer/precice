#ifndef PRECICE_INITIALIZERNATIVE2NATIVEPLAINPORT_H_
#define PRECICE_INITIALIZERNATIVE2NATIVEPLAINPORT_H_ 

#include "precice/Initializer.h"
#include <jni.h> 
#include <iostream>

#ifdef __cplusplus
  extern "C" {
#endif


          
JNIEXPORT void JNICALL Java_precice_InitializerNative2NativePlainPort_createInstance(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL Java_precice_InitializerNative2NativePlainPort_destroyInstance(JNIEnv *env, jobject obj,jlong ref);
JNIEXPORT void JNICALL Java_precice_InitializerNative2NativePlainPort_connect(JNIEnv *env, jobject obj,jlong ref,jlong destination);


#ifdef __cplusplus
  }
#endif

namespace precice { 

     class InitializerNative2NativePlainPort;
}

class precice::InitializerNative2NativePlainPort: public precice::Initializer{
  private:
    precice::Initializer* _destination;
  public:
    InitializerNative2NativePlainPort();
    ~InitializerNative2NativePlainPort();
    
    void connect(precice::Initializer*);
    void acknowledge(const int identifier,int& tag);  
    void acknowledgeParallel(const int identifier,int& tag);
   
    void initialize(const std::string* addresses, const int addresses_len,const int* vertexes, const int vertexes_len);  
    void initializeParallel(const std::string* addresses, const int addresses_len,const int* vertexes, const int vertexes_len);
   
};

#endif
