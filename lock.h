//
// Created by chengwenjie on 2019/8/29.
//

//#ifndef UNTITLED2_LOCK_H
//#define UNTITLED2_LOCK_H
//
//#endif //UNTITLED2_LOCK_H
#include "thread.h"

extern void monitorInit(Monitor *mon);

extern void objectLock(Object *ob);
extern void objectUnlock(Object *ob);
extern void objectNotifyAll(Object *ob);
extern void objectWait(Object *ob, long long ms, int ns);
