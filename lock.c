//
// Created by chengwenjie on 2019/8/29.
//
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include "jam.h"
#include "thread.h"
#include "hash.h"

/* Trace lock operations and inflation/deflation */
#ifdef TRACELOCK
#define TRACE(x) printf x
#else
#define TRACE(x)
#endif

#define HASHTABSZE 1<<5
#define HASH(obj) ((int) obj >> LOG_OBJECT_GRAIN)
#define COMPARE(obj, mon, hash1, hash2) hash1 == hash2
#define PREPARE(obj) allocMonitor(obj)
#define FOUND(ptr) ptr->in_use = TRUE

static HashTable mon_cache;

void monitorInit(Monitor *mon) {
    pthread_mutex_init(&mon->lock, NULL);
    pthread_cond_init(&mon->cv, NULL);
    mon->owner = 0;
    mon->count = 0;
    mon->waiting = 0;
    mon->notifying = 0;
    mon->interrupting = 0;
    mon->entering = 0;
}

void objectNotifyAll(Object *obj) {}

void initialiseMonitor() {
    initHashTable(mon_cache, HASHTABSZE);
}

void objectLock(Object *obj) {}

void objectUnlock(Object *obj) {}

void objectWait(Object *obj, long long ms, int ns) {}