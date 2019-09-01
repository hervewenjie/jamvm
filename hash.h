//
// Created by chengwenjie on 2019/8/29.
//

//#ifndef UNTITLED2_HASH_H
//#define UNTITLED2_HASH_H
//
//#endif //UNTITLED2_HASH_H
#include <string.h>
#include "thread.h"

typedef struct hash_entry {
    int hash;
    void *data;
} HashEntry;

typedef struct hash_table {
    HashEntry *hash_table;
    int hash_size;
    int hash_count;
    VMLock lock;
} HashTable;

extern void resizeHash(HashTable *table, int new_size);

#define initHashTable(table, initial_size)                                         \
{                                                                                  \
    table.hash_table = (HashEntry*)malloc(sizeof(HashEntry)*initial_size);         \
    memset(table.hash_table, 0, sizeof(HashEntry)*initial_size);                   \
    table.hash_size = initial_size;                                                \
    table.hash_count = 0;                                                          \
    initVMLock(table.lock);                                                        \
}

// table, classname, return class,
#define findHashEntry(table, ptr, ptr2, add_if_absent, scavenge)                   \
{                                                                                  \
    int hash = HASH(ptr);                                                          \
    int i;                                                                         \
                                                                                   \
    Thread *self;                                                                  \
    disableSuspend(self = threadSelf());                                           \
    lockVMLock(table.lock, self);                                                  \
    i = hash & (table.hash_size - 1);                                              \
                                                                                   \
    for(;;) {                                                                      \
        ptr2 = table.hash_table[i].data;                                           \
        if((ptr2 == NULL) || COMPARE(ptr, ptr2, hash, table.hash_table[i].hash))   \
            break;                                                                 \
                                                                                   \
        i = (i+1) & (table.hash_size - 1);                                         \
    }                                                                              \
                                                                                   \
    if(ptr2) {                                                                     \
        FOUND(ptr2);                                                               \
    } else                                                                         \
        if(add_if_absent) {                                                        \
            table.hash_table[i].hash = hash;                                       \
            ptr2 = table.hash_table[i].data = PREPARE(ptr);                        \
                                                                                   \
            table.hash_count++;                                                    \
            if((table.hash_count * 4) > (table.hash_size * 3)) {                   \
                int new_size;                                                      \
                if(scavenge) {                                                     \
                    int n;                                                         \
                    for(i = 0, n = table.hash_count; n--; i++) {                   \
                        void *data = table.hash_table[i].data;                     \
                        if(data && SCAVENGE(data)) {                               \
                            table.hash_table[i].data = NULL;                       \
	                    table.hash_count--;                                        \
                        }                                                          \
                    }                                                              \
                    if((table.hash_count * 3) > (table.hash_size * 2))             \
                        new_size = table.hash_size*2;                              \
                    else                                                           \
                        new_size = table.hash_size;                                \
                } else                                                             \
                    new_size = table.hash_size*2;                                  \
                                                                                   \
                resizeHash(&table, new_size);                                      \
	    }                                                                      \
        }                                                                          \
    unlockVMLock(table.lock, self);                                                \
    enableSuspend(self);                                                           \
}