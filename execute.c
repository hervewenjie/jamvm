//
// Created by chengwenjie on 2019/8/29.
//
#include "jam.h"
#include "sig.h"
#include "frame.h"
#include "lock.h"

#define VA_DOUBLE(args, sp)  *(u8*)sp = va_arg(args, u8); sp+=2
#define VA_SINGLE(args, sp)  *sp++ = va_arg(args, u4)

#define JA_DOUBLE(args, sp)  (u8*)sp++; *((u8*)sp) = *args++
#define JA_SINGLE(args, sp)  *sp++ = *(u4*)args; args++

void *executeMethodArgs(Object *ob, Class *class, MethodBlock *mb, ...) {
    // variable params
    va_list jargs;
    void *ret;

    va_start(jargs, mb);
    ret = executeMethodVaList(ob, class, mb, jargs);
    va_end(jargs);

    return ret;
}

void *executeMethodVaList(Object *ob, Class *class, MethodBlock *mb, va_list jargs) {
    ClassBlock *cb = CLASS_CB(class);
    char *sig = mb->type;

    ExecEnv *ee = getExecEnv();
    void *ret;
    u4 *sp;

    // create frame
    CREATE_TOP_FRAME(ee, class, mb, sp, ret);

    /* copy args onto stack */

    // index 0 of local variable table is always "this"
    if(ob)
        *sp++ = (u4) ob; /* push receiver first */

    SCAN_SIG(sig, VA_DOUBLE(jargs, sp), VA_SINGLE(jargs, sp))

    // synchronized lock
    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : (Object*)mb->class);

    // native
    if(mb->access_flags & ACC_NATIVE)
        (*(u4 *(*)(Class*, MethodBlock*, u4*))mb->native_invoker)(class, mb, ret);
        // java
    else
        executeJava();

    // synchronized unlock
    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : (Object*)mb->class);

    // pop frame
    POP_TOP_FRAME(ee);
    return ret;
}

void *executeMethodList(Object *ob, Class *class, MethodBlock *mb, u8 *jargs) {
    ClassBlock *cb = CLASS_CB(class);
    char *sig = mb->type;

    ExecEnv *ee = getExecEnv();
    void *ret;
    u4 *sp;

    CREATE_TOP_FRAME(ee, class, mb, sp, ret);

    /* copy args onto stack */

    if(ob)
        *sp++ = (u4) ob; /* push receiver first */

    SCAN_SIG(sig, JA_DOUBLE(jargs, sp), JA_SINGLE(jargs, sp))

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : (Object*)mb->class);

    if(mb->access_flags & ACC_NATIVE)
        (*(u4 *(*)(Class*, MethodBlock*, u4*))mb->native_invoker)(class, mb, ret);
    else
        executeJava();

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : (Object*)mb->class);

    POP_TOP_FRAME(ee);
    return ret;
}

