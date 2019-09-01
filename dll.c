//
// Created by chengwenjie on 2019/8/30.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jam.h"

#ifndef NO_JNI
#include <dlfcn.h>
#include "hash.h"
#include "jni.h"

/* Trace library loading and method lookup */
#ifdef TRACEDLL
#define TRACE(x) printf x
#else
#define TRACE(x)
#endif

#define HASHTABSZE 1<<4
static HashTable hash_table;
void *lookupLoadedDlls(MethodBlock *mb);
#endif

void *lookupLoadedDlls(MethodBlock *mb) {}

void *lookupInternal(MethodBlock *mb) {
    int i;

    TRACE(("<Dll: Looking up %s internally>\n", mb->name));

    for(i = 0; native_methods[i][0] &&
               (strcmp(mb->name, native_methods[i][0]) != 0) ; i++);

    if(native_methods[i][0])
        return mb->native_invoker = (void*)native_methods[i][1];
    else
        return NULL;
}

void *resolveNativeMethod(MethodBlock *mb) {
    void *func = lookupInternal(mb);

#ifndef NO_JNI
    if(func == NULL)
        func = lookupLoadedDlls(mb);
#endif

    return func;
}

u4 *resolveNativeWrapper(Class *class, MethodBlock *mb, u4 *ostack) {
    void *func = resolveNativeMethod(mb);

    if(func == NULL) {
        signalException("java/lang/UnsatisfiedLinkError", mb->name);
        return ostack;
    }
    return (*(u4 *(*)(Class*, MethodBlock*, u4*))func)(class, mb, ostack);
}