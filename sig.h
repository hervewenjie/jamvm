//
// Created by chengwenjie on 2019/8/30.
//

//#ifndef UNTITLED2_SIG_H
//#define UNTITLED2_SIG_H
//
//#endif //UNTITLED2_SIG_H

#define SCAN_SIG(p, D, S)			\
   p++;               /* skip start ( */	\
   while(*p != ')') {				\
       if((*p == 'J') || (*p == 'D')) {		\
          D;					\
          p++;					\
      } else {					\
          S;					\
          if(*p == '[')				\
              for(p++; *p == '['; p++);		\
          if(*p == 'L')				\
              while(*p++ != ';');		\
          else					\
              p++;				\
      }						\
   }						\
   p++;               /* skip end ) */
