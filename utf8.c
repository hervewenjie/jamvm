//
// Created by chengwenjie on 2019/8/29.
//
#include <string.h>
#include "jam.h"
#include "hash.h"

#define HASHTABSZE 1<<10
#define HASH(ptr) utf8Hash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) (ptr1 == ptr2) || \
                  ((hash1 == hash2) && utf8Comp(ptr1, ptr2))
#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr)

static HashTable hash_table;

#define GET_UTF8_CHAR(ptr, c)                         \
{                                                     \
    int x = *ptr++;                                   \
    if(x & 0x80) {                                    \
        int y = *ptr++;                               \
        if(x & 0x20) {                                \
            int z = *ptr++;                           \
            c = ((x&0xf)<<12)+((y&0x3f)<<6)+(z&0x3f); \
        } else                                        \
            c = ((x&0x1f)<<6)+(y&0x3f);               \
    } else                                            \
        c = x;                                        \
}

void convertUtf8(unsigned char *utf8, short *buff) {
    while(*utf8)
    GET_UTF8_CHAR(utf8, *buff++);
}

int utf8Hash(unsigned char *utf8) {
    int hash = 0;

    while(*utf8) {
        short c;
        GET_UTF8_CHAR(utf8, c);
        hash = hash * 37 + c;
    }

    return hash;
}

int utf8Comp(unsigned char *ptr, unsigned char *ptr2) {
    while(*ptr && *ptr2) {
        short c, c2;

        GET_UTF8_CHAR(ptr, c);
        GET_UTF8_CHAR(ptr2, c2);
        if(c != c2)
            return FALSE;
    }
    if(*ptr || *ptr2)
        return FALSE;

    return TRUE;
}

unsigned char *findUtf8String(unsigned char *string) {
    unsigned char *interned;

    findHashEntry(hash_table, string, interned, TRUE, FALSE);

    return interned;
}

void initialiseUtf8() {
    initHashTable(hash_table, HASHTABSZE);
}

int utf8Len(unsigned char *utf8) {
    int count;

    for(count = 0; *utf8; count++) {
        int x = *utf8;
        utf8 += (x & 0x80) ? ((x & 0x20) ?  3 : 2) : 1;
    }

    return count;
}

unsigned char *slash2dots(unsigned char *utf8) {
    int len = utf8Len(utf8);
    unsigned char *conv = (unsigned char*)malloc(len+1);
    int i;

    for(i = 0; i <= len; i++)
        if(utf8[i] == '/')
            conv[i] = '.';
        else
            conv[i] = utf8[i];

    return conv;
}