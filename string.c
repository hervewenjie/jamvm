//
// Created by chengwenjie on 2019/8/29.
//
#include <string.h>
#include <stdio.h>
#include "jam.h"
#include "hash.h"

#define HASHTABSZE 1<<10
#define HASH(ptr) stringHash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) (ptr1 == ptr2) || \
                  ((hash1 == hash2) && stringComp(ptr1, ptr2))
#define ITERATE(ptr)  markObject(ptr)
#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr)

extern void markObject(Object *ob);

static Class *string;
static int count_offset;
static int value_offset;
static int offset_offset;

static HashTable hash_table;

static int inited = FALSE;


Object *Cstr2String(char *cstr) {}

// init java.lang.String
void initialiseString() {
    if(!inited) {
        FieldBlock *count, *value, *offset;

        /* As we're initialising, VM will abort if String can't be found */
        string = findSystemClass0("java/lang/String");

        count = findField(string, "count", "I");
        value = findField(string, "value", "[C");
        offset = findField(string, "offset", "I");

        /* findField doesn't throw an exception... */
        if((count == NULL) || (value == NULL) || (offset == NULL)) {
            printf("Error initialising VM (initialiseString)\n");
            exit(0);
        }

        count_offset = count->offset;
        value_offset = value->offset;
        offset_offset = offset->offset;

        initHashTable(hash_table, HASHTABSZE);
        inited = TRUE;
    }
}

int stringHash(Object *ptr) {
    Object *array = (Object*)INST_DATA(ptr)[value_offset];
    int len = INST_DATA(ptr)[count_offset];
    int offset = INST_DATA(ptr)[offset_offset];
    short *dpntr = ((short*)INST_DATA(array))+offset+2;
    int hash = 0;

    for(; len > 0; len--)
        hash = hash * 37 + *dpntr++;

    return hash;
}

int stringComp(Object *ptr, Object *ptr2) {
    int len = INST_DATA(ptr)[count_offset];
    int len2 = INST_DATA(ptr2)[count_offset];

    if(len == len2) {
        Object *array = (Object*)INST_DATA(ptr)[value_offset];
        Object *array2 = (Object*)INST_DATA(ptr2)[value_offset];
        int offset = INST_DATA(ptr)[offset_offset];
        int offset2 = INST_DATA(ptr2)[offset_offset];
        short *src = ((short*)INST_DATA(array))+offset+2;
        short *dst = ((short*)INST_DATA(array2))+offset2+2;

        for(; (len > 0) && (*src++ == *dst++); len--);

        if(len == 0)
            return TRUE;
    }

    return FALSE;
}

Object *createString(unsigned char *utf8) {
    int len = utf8Len(utf8);
    Object *array;
    short *data;
    Object *ob;

    if(!inited)
        initialiseString();

    if((array = allocTypeArray(T_CHAR, len)) == NULL ||
       (ob = allocObject(string)) == NULL)
        return NULL;

    data = (short*)INST_DATA(array)+2;
    convertUtf8(utf8, data);

    INST_DATA(ob)[count_offset] = len;
    INST_DATA(ob)[value_offset] = (u4)array;

    return ob;
}

Object *findInternedString(Object *string) {
    Object *interned;

    findHashEntry(hash_table, string, interned, TRUE, FALSE);

    return interned;
}