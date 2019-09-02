//
// Created by chengwenjie on 2019/8/29.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "jam.h"
#include "thread.h"
#include "lock.h"

#ifdef TRACETHREAD
#define TRACE(x) printf x
#else
#define TRACE(x)
#endif

static int java_stack_size;

/* Thread create/destroy lock and condvar */
static pthread_mutex_t lock;
static pthread_cond_t cv;

/* lock and condvar used by main thread to wait for
 * all non-daemon threads to die */
static pthread_mutex_t exit_lock;
static pthread_cond_t exit_cv;

/* Monitor for sleeping threads to do a timed-wait against */
static Monitor sleep_mon;

/* Thread specific key holding thread's Thread pntr */
static pthread_key_t threadKey;

/* Attributes for spawned threads */
static pthread_attr_t attributes;

/* The main thread info - head of the thread list */
static Thread main, dead_thread;

/* Main thread ExecEnv */
static ExecEnv main_ee;

/* Various field offsets into java.lang.Thread -
 * cached at startup and used in thread creation */
static int vmData_offset;
static int daemon_offset;
static int group_offset;
static int priority_offset;
static int name_offset;

/* Method table indexes of Thread.run method and
 * ThreadGroup.removeThread - cached at startup */
static int run_mtbl_idx;
static int rmveThrd_mtbl_idx;

/* Cached java.lang.Thread class */
static Class *thread_class;

/* Count of non-daemon threads still running in VM */
static int non_daemon_thrds = 0;

/* Bitmap - used for generating unique thread ID's */
#define MAP_INC 32
static unsigned int *tidBitmap;
static int tidBitmapSize = 0;

/* Mark a threadID value as no longer used */
#define freeThreadID(n) tidBitmap[n>>5] &= ~(1<<(n&0x1f))

/* Generate a new thread ID - assumes the thread queue
 * lock is held */

static int genThreadID() {
    int i = 0;

    retry:
    for(; i < tidBitmapSize; i++) {
        if(tidBitmap[i] != 0xffffffff) {
            int n = ffs(~tidBitmap[i]);
            tidBitmap[i] |= 1 << (n-1);
            return (i<<5) + n;
        }
    }

    tidBitmap = (unsigned int *)realloc(tidBitmap, (tidBitmapSize + MAP_INC) * sizeof(unsigned int));
    memset(tidBitmap + tidBitmapSize, 0, MAP_INC * sizeof(unsigned int));
    tidBitmapSize += MAP_INC;
    goto retry;
}

void *getStackTop(Thread *thread) {
    return thread->stack_top;
}

void *getStackBase(Thread *thread) {
    return thread->stack_base;
}

Thread *threadSelf0(Object *jThread) {
    return (Thread*)(INST_DATA(jThread)[vmData_offset]);
}

Thread *threadSelf() {
    return (Thread*)pthread_getspecific(threadKey);
}

void setThreadSelf(Thread *thread) {
    pthread_setspecific(threadKey, thread);
}

ExecEnv *getExecEnv() {
    return threadSelf()->ee;
}

void initialiseJavaStack(ExecEnv *ee) {
    printf("%d\n", java_stack_size);
    char *stack = (char *)malloc(java_stack_size);
    MethodBlock *mb = (MethodBlock *) stack;
    Frame *top = (Frame *) (mb+1);

    mb->max_stack = 0;
    top->mb = mb;
    top->ostack = (u4*)(top+1);
    top->prev = 0;

    ee->stack = stack;
    ee->last_frame = top;
    ee->stack_end = stack + java_stack_size-1024;
}

void mainThreadWaitToExitVM() {}

static void suspendLoop(Thread *thread) {
    char old_state = thread->state;
    sigset_t mask;
    sigjmp_buf env;

    sigsetjmp(env, FALSE);

    thread->stack_top = &env;
    thread->state = SUSPENDED;

    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGTERM);

    while(thread->suspend)
        sigsuspend(&mask);

    thread->state = old_state;
}

void disableSuspend0(Thread *thread, void *stack_top) {
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    thread->stack_top = stack_top;
    thread->blocking = TRUE;
}

void enableSuspend(Thread *thread) {
    sigset_t mask;

    sigemptyset(&mask);

    thread->blocking = FALSE;

    if(thread->suspend)
        suspendLoop(thread);

    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}


void initialiseMainThread(int stack_size) {
    Class *thrdGrp_class;
    FieldBlock *vmData;
    MethodBlock *run, *remove_thread, uncaught_exp;
    FieldBlock *daemon, *name, *group, *priority, *root;

    java_stack_size = stack_size;

    pthread_key_create(&threadKey, NULL);   // thread local

    pthread_mutex_init(&lock, NULL);        // init lock
    pthread_cond_init(&cv, NULL);           // init condition

    pthread_mutex_init(&exit_lock, NULL);   // init exit lock
    pthread_cond_init(&exit_cv, NULL);      // init condition

    monitorInit(&sleep_mon);                // init monitor

    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    main.stack_base = &thrdGrp_class;

    main.tid = pthread_self();
    main.id = genThreadID();
    main.state = RUNNING;
    main.ee = &main_ee;

    initialiseJavaStack(&main_ee);
    setThreadSelf(&main);

    /* As we're initialising, VM will abort if Thread can't be found */
    thread_class = findSystemClass0("java/lang/Thread");
}