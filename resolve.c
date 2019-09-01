//
// Created by chengwenjie on 2019/8/29.
//
#include <stdio.h>
#include "jam.h"

MethodBlock *findMethod(Class *class, char *methodname, char *type) {}

MethodBlock *lookupMethod(Class *class, char *methodname, char *type) { return NULL; }

Class *resolveClass(Class *class, int cp_index, int init) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    Class *resolved_class;

    retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            resolved_class = (Class *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Class: {
            char *classname;
            int name_idx = CP_CLASS(cp, cp_index);

            if(CP_TYPE(cp, cp_index) != CONSTANT_Class)
                goto retry;

            classname = CP_UTF8(cp, name_idx);
            resolved_class = findClassFromClass(classname, class);

            /* If we can't find the class an exception will already have
               been thrown */

            if(resolved_class == NULL)
                return NULL;

            CP_TYPE(cp, cp_index) = CONSTANT_Locked;
            CP_INFO(cp, cp_index) = (u4)resolved_class;
            CP_TYPE(cp, cp_index) = CONSTANT_Resolved;

            break;
        }
    }

    if(init)
        initClass(resolved_class);

    return resolved_class;
}
