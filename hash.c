//
// Created by chengwenjie on 2019/8/30.
//
#include <string.h>
#include "jam.h"
#include "hash.h"

void resizeHash(HashTable *table, int new_size) {
    // malloc new size
    HashEntry *new_table = (HashEntry*)malloc(sizeof(HashEntry)*new_size);
    int i;
    // memset
    memset(new_table, 0, sizeof(HashEntry)*new_size);
    // foreach calculate new index
    for(i = table->hash_size-1; i >= 0; i--) {
        void *ptr = table->hash_table[i].data;
        if(ptr != NULL) {
            int hash = table->hash_table[i].hash;
            int new_index = hash & (new_size - 1);

            while(new_table[new_index].data != NULL)
                new_index = (new_index+1) & (new_size - 1);

            new_table[new_index].hash = hash;
            new_table[new_index].data = ptr;
        }
    }

    free(table->hash_table);
    table->hash_table = new_table;
    table->hash_size = new_size;
}