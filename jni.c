//
// Created by chengwenjie on 2019/8/30.
//

#ifndef NO_JNI
#include "jni.h"
#include "jam.h"
#include "thread.h"
#include "lock.h"

static VMLock global_ref_lock;

static void initJNIGrefs() {
    initVMLock(global_ref_lock);
}

void initialiseJNI() {
    initJNIGrefs();
}
#endif
