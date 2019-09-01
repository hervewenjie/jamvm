//
// Created by chengwenjie on 2019/8/29.
//
#include <stdio.h>
#include <string.h>
#include "jam.h"

MethodBlock *findMethod(Class *class, char *methodname, char *type) {}

// A class can't have two fields with the same name but different types -
// so we give up if we find a field with the right name but wrong type...
//
FieldBlock *findField(Class *class, char *fieldname, char *type) {
    ClassBlock *cb = CLASS_CB(class);
    FieldBlock *fb = cb->fields;
    int i;

    for(i = 0; i < cb->fields_count; i++,fb++)
        if(strcmp(fb->name, fieldname) == 0)
            if(strcmp(fb->type, type) == 0)
                return fb;
            else
                return NULL;

    return NULL;
}

MethodBlock *lookupMethod(Class *class, char *methodname, char *type) { return NULL; }

FieldBlock *lookupField(Class *class, char *fieldname, char *type) {
    FieldBlock *fb;

    if(fb = findField(class, fieldname, type))
        return fb;

    if (CLASS_CB(class)->super)
        return lookupField(CLASS_CB(class)->super, fieldname, type);

    return NULL;
}

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

MethodBlock *resolveMethod(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    MethodBlock *mb;

    retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            mb = (MethodBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Methodref: {
            Class *resolved_class;
            char *methodname, *methodtype;
            int cl_idx = CP_METHOD_CLASS(cp, cp_index);
            int name_type_idx = CP_METHOD_NAME_TYPE(cp, cp_index);

            if(CP_TYPE(cp, cp_index) != CONSTANT_Methodref)
                goto retry;

            methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            resolved_class = resolveClass(class, cl_idx, TRUE);

            if(exceptionOccured())
                return NULL;

            mb = lookupMethod(resolved_class, methodname, methodtype);

            if(mb) {
                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                CP_INFO(cp, cp_index) = (u4)mb;
                CP_TYPE(cp, cp_index) = CONSTANT_Resolved;
            } else
                signalException("java/lang/NoSuchMethodError", methodname);

            break;
        }
    }

    return mb;
}

MethodBlock *resolveInterfaceMethod(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    MethodBlock *mb;

    retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            mb = (MethodBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_InterfaceMethodref: {
            Class *resolved_class;
            char *methodname, *methodtype;
            int cl_idx = CP_METHOD_CLASS(cp, cp_index);
            int name_type_idx = CP_METHOD_NAME_TYPE(cp, cp_index);

            if(CP_TYPE(cp, cp_index) != CONSTANT_InterfaceMethodref)
                goto retry;

            methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            resolved_class = resolveClass(class, cl_idx, TRUE);

            if(exceptionOccured())
                return NULL;

            mb = lookupMethod(resolved_class, methodname, methodtype);

            if(mb) {
                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                CP_INFO(cp, cp_index) = (u4)mb;
                CP_TYPE(cp, cp_index) = CONSTANT_Resolved;
            } else
                signalException("java/lang/NoSuchMethodError", methodname);

            break;
        }
    }

    return mb;
}

FieldBlock *resolveField(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    FieldBlock *fb;

    retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            fb = (FieldBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Fieldref: {
            Class *resolved_class;
            char *fieldname, *fieldtype;
            int cl_idx = CP_FIELD_CLASS(cp, cp_index);
            int name_type_idx = CP_FIELD_NAME_TYPE(cp, cp_index);

            if(CP_TYPE(cp, cp_index) != CONSTANT_Fieldref)
                goto retry;

            fieldname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            fieldtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            resolved_class = resolveClass(class, cl_idx, TRUE);

            if(exceptionOccured())
                return NULL;

            fb = lookupField(resolved_class, fieldname, fieldtype);

            if(fb) {
                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                CP_INFO(cp, cp_index) = (u4)fb;
                CP_TYPE(cp, cp_index) = CONSTANT_Resolved;
            } else
                signalException("java/lang/NoSuchFieldError", fieldname);

            break;
        }
    }

    return fb;
}