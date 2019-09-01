//
// Created by chengwenjie on 2019/8/30.
//

#ifdef NO_JNI
#error to use classpath, Jam must be compiled with JNI!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "jam.h"
#include "thread.h"
#include "lock.h"

u4 *arraycopy(Class *class, MethodBlock *mb, u4 *ostack) {}

u4 *getPrimitiveClass(Class *class, MethodBlock *mb, u4 *ostack) {}

u4 *insertSystemProperties(Class *class, MethodBlock *mb, u4 *ostack) {}

u4 *resolveClass0(Class *class, MethodBlock *mb, u4 *ostack) {}

u4 *defineClass0(Class *clazz, MethodBlock *mb, u4 *ostack) {}


char *native_methods[][2] = {
        "arraycopy",		        (char*)arraycopy,
        "insertSystemProperties",	(char*)insertSystemProperties,
        "getPrimitiveClass",	    (char*)getPrimitiveClass,
        "defineClass",              (char*)defineClass0,
        "resolveClass",             (char*)resolveClass0,
//        "intern",			        (char*)intern,
//        "forName0",		            (char*)forName0,
//        "newInstance",		        (char*)newInstance,
//        "isInstance",		        (char*)isInstance,
//        "isAssignableFrom",	        (char*)isAssignableFrom,
//        "isInterface",		        (char*)isInterface,
//        "isPrimitive",		        (char*)isPrimitive,
//        "getSuperclass",		    (char*)getSuperclass,
//        "getName",			        (char*)getName,
//        "getClass",		            (char*)getClass,
//        "identityHashCode",	        (char*)identityHashCode,
//        "clone",			        (char*)jamClone,
//        "wait",			            (char*)wait,
//        "notify",			        (char*)notify,
//        "notifyAll",		        (char*)notifyAll,
//        "gc",			            (char*)gc,
//        "runFinalization",		    (char*)runFinalization,
//        "exitInternal",		        (char*)exitInternal,
//        "fillInStackTrace",	        (char*)fillInStackTrace,
//        "printStackTrace0",	        (char*)printStackTrace0,
//        "currentClassLoader",	    (char*)currentClassLoader,
//        "getClassContext",		    (char*)getClassContext,
//        "getConstructor",		    (char*)getConstructor,
//        "getClassLoader0",		    (char*)getClassLoader0,
//        "constructNative",		    (char*)constructNative,
//        "nativeLoad",		        (char*)nativeLoad,
//        "nativeGetLibname",	        (char*)nativeGetLibname,
//        "freeMemory",		        (char*)freeMemory,
//        "totalMemory",		        (char*)totalMemory,
//        "maxMemory",		        (char*)maxMemory,
//        "currentThread",		    (char*)currentThread,
//        "nativeInit",		        (char*)nativeInit,
//        "start",			        (char*)start,
//        "sleep",			        (char*)jamSleep,
//        "nativeInterrupt",		    (char*)interrupt,
//        "isAlive",			        (char*)isAlive,
//        "yield",			        (char*)yield,
//        "isInterrupted",		    (char*)isInterrupted,
//        "interrupted",		        (char*)interrupted,
//        "nativeSetPriority",	    (char*)nativeSetPriority,
        NULL,			NULL};