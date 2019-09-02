//
// Created by chengwenjie on 2019/8/29.
//
#include <stdio.h>
#include "jam.h"
#include "lock.h"

Object *exceptionOccured() { return NULL; }

void signalException(char *excep_name, char *message) {}

unsigned char *findCatchBlockInMethod(MethodBlock *mb, Class *exception, unsigned char *pc_pntr) {
    ExceptionTableEntry *table = mb->exception_table;
    int size = mb->exception_table_size;
    int pc = pc_pntr - mb->code;
    int i;

    for(i = 0; i < size; i++)
        if((pc >= table[i].start_pc) && (pc < table[i].end_pc)) {

            /* If the catch_type is 0 it's a finally block, which matches
               any exception.  Otherwise, the thrown exception class must
               be an instance of the caught exception class to catch it */

            if(table[i].catch_type != 0) {
                Class *caught_class = resolveClass(mb->class, table[i].catch_type, FALSE);
                if(caught_class == NULL) {
                    clearException();
                    continue;
                }
                if(!isInstanceOf(caught_class, exception))
                    continue;
            }
            return mb->code + table[i].handler_pc;
        }

    return NULL;
}

unsigned char *findCatchBlock(Class *exception) {
    Frame *frame = getExecEnv()->last_frame;
    unsigned char *handler_pc = NULL;

    while(((handler_pc = findCatchBlockInMethod(frame->mb, exception, frame->last_pc)) == NULL)
          && (frame->prev->mb != NULL)) {

        if(frame->mb->access_flags & ACC_SYNCHRONIZED) {
            Object *sync_ob = frame->mb->access_flags & ACC_STATIC ?
                              (Object*)frame->mb->class : (Object*)frame->lvars[0];
            objectUnlock(sync_ob);
        }
        frame = frame->prev;
    }

    getExecEnv()->last_frame = frame;

    return handler_pc;
}