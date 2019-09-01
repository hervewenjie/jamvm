//
// Created by chengwenjie on 2019/8/29.
//

//#ifndef UNTITLED2_THREAD_H
//#define UNTITLED2_THREAD_H
//
//#endif //UNTITLED2_THREAD_H
#ifndef CREATING
#include <pthread.h>
#include <setjmp.h>

/* Thread states */

#define CREATING     0
#define STARTED      1
#define RUNNING      2
#define WAITING      3
#define SUSPENDED    5

typedef struct thread Thread;

typedef struct monitor {
    pthread_mutex_t lock;
    pthread_cond_t cv;
    Thread *owner;
    int count;
    int waiting;
    int notifying;
    int interrupting;
    int entering;
    struct monitor *next;
    char in_use;
} Monitor;

struct thread {
    char state;
    char interrupted;
    char interrupting;
    char suspend;
    char blocking;
    pthread_t tid;         // tid
    int id;                // id
    ExecEnv *ee;           // env
    void *stack_top;       // stack top
    void *stack_base;      // stack base
    Monitor *wait_mon;
    Thread *prev, *next;   // thread list
};

extern Thread *threadSelf();


extern void disableSuspend0(Thread *thread, void *stack_top);
extern void enableSuspend(Thread *thread);

#define disableSuspend(thread)          \
{                                       \
    sigjmp_buf *env;                    \
    env = alloca(sizeof(sigjmp_buf));   \
    sigsetjmp(*env, FALSE);             \
    disableSuspend0(thread, (void*)env);\
}

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cv;
} VMWaitLock;

typedef pthread_mutex_t VMLock;

#define initVMLock(lock) pthread_mutex_init(&lock, NULL)
#define initVMWaitLock(wait_lock) {            \
    pthread_mutex_init(&wait_lock.lock, NULL); \
    pthread_cond_init(&wait_lock.cv, NULL);    \
}

#define lockVMLock(lock, self) { \
    self->state = WAITING;       \
    pthread_mutex_lock(&lock);   \
    self->state = RUNNING;       \
}
#define unlockVMLock(lock, self) pthread_mutex_unlock(&lock)

#endif