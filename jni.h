//
// Created by chengwenjie on 2019/8/30.
//

//#ifndef UNTITLED2_JNI_H
//#define UNTITLED2_JNI_H
//
//#endif //UNTITLED2_JNI_H
#include <stdio.h>
#include <stdarg.h>

#define JNI_FALSE 0
#define JNI_TRUE 1

#define JNI_OK 0
#define JNI_ERR (-1)

#define JNI_COMMIT 1
#define JNI_ABORT 2

#define JNIEXPORT
#define JNICALL

typedef int 		jint;
typedef long long 	jlong;
typedef signed char 	jbyte;
typedef unsigned char	jboolean;
typedef unsigned short	jchar;
typedef short		jshort;
typedef float		jfloat;
typedef double		jdouble;
typedef jint            jsize;

typedef void *jobject;
typedef jobject jclass;
typedef jobject jthrowable;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jbooleanArray;
typedef jarray jbyteArray;
typedef jarray jcharArray;
typedef jarray jshortArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jarray jfloatArray;
typedef jarray jdoubleArray;
typedef jarray jobjectArray;

typedef jobject jref;

typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;

typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} JNINativeMethod;

typedef void *jfieldID;
typedef void *jmethodID;
typedef const void *JavaVM;