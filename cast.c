//
// Created by chengwenjie on 2019/8/30.
//
#include <stdio.h>
#include "jam.h"

char implements(Class *class, Class *test) {
    ClassBlock *test_cb = CLASS_CB(test);
    int i;

    for(i = 0; i < test_cb->interfaces_count; i++)
        if((class == test_cb->interfaces[i]) ||
           implements(class, test_cb->interfaces[i]))
            return TRUE;

    if(test_cb->super)
        return implements(class, test_cb->super);

    return FALSE;
}

char isSubClassOf(Class *class, Class *test) {
    for(; test != NULL && test != class; test = CLASS_CB(test)->super);
    return test != NULL;
}

char isInstOfArray(Class *class, Class *test) {
    if(isSubClassOf(class, test))
        return TRUE;
    else {
        ClassBlock *class_cb = CLASS_CB(class);
        ClassBlock *test_cb = CLASS_CB(test);

        if((class_cb->name[0] == '[') && (test_cb->element_class != NULL) &&
           (class_cb->element_class != NULL) && (class_cb->dim == test_cb->dim))
            return isInstanceOf(class_cb->element_class, test_cb->element_class);
        else
            return FALSE;
    }
}

char isInstanceOf(Class *class, Class *test) {
    if(class == test)
        return TRUE;

    if(CLASS_CB(class)->access_flags & ACC_INTERFACE)
        return implements(class, test);
    else
    if(CLASS_CB(test)->name[0] == '[')
        return isInstOfArray(class, test);
    else
        return isSubClassOf(class, test);
}