//
// Created by shuzhao.cwj on 2019/9/2.
//

//#ifndef UNTITLED2_FRAME_H
//#define UNTITLED2_FRAME_H
//
//#endif //UNTITLED2_FRAME_H

#define CREATE_TOP_FRAME(ee, class, mb, sp, ret)                \
{                                                               \
    Frame *last = ee->last_frame;                               \
    Frame *dummy = (Frame *)(last->ostack+last->mb->max_stack); \
    Frame *new_frame;                                           \
                                                                \
    void* sp_tmp = (void*) sp;                                  \
    ret = sp_tmp = (u4*)(dummy+1);                              \
    new_frame = (Frame *)(sp + mb->max_locals);                 \
                                                                \
    dummy->mb = NULL;                                           \
    dummy->ostack = sp;                                         \
    dummy->prev = last;                                         \
                                                                \
    new_frame->mb = mb;                                         \
    new_frame->lvars = sp;                                      \
    new_frame->ostack = (u4 *)(new_frame + 1);                  \
                                                                \
    new_frame->prev = dummy;                                    \
    ee->last_frame = new_frame;                                 \
}

#define POP_TOP_FRAME(ee)                                       \
    ee->last_frame = ee->last_frame->prev->prev;
