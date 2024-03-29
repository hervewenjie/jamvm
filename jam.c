#include <stdio.h>
#include <string.h>

#include "jam.h"

/* Setup default values for command line args */

static int noasyncgc = FALSE;
static int verbosegc = FALSE;
static int verboseclass = FALSE;

#define KB 1024
#define MB (KB*KB)
#define MIN_HEAP 4*KB
#define MIN_STACK 2*KB

static int java_stack = 128*KB;

static int min_heap   = 512*MB;
static int max_heap   = 1024*MB;

char VM_initing = TRUE;

void initVM() {
    Class *class;
    MethodBlock *mb;

    initialiseAlloc(min_heap, max_heap, verbosegc);  // init mem
    initialiseClass(verboseclass);                   // init classpath & hash table
//    initialiseDll();                                // init dll hash table
    initialiseUtf8();                                // init utf8 hash table
    initialiseMonitor();                             // init monitor hash table
    initialiseMainThread(java_stack);                // little fucking complicated
    initialiseString();                              // init java.lang.String
    initialiseGC(noasyncgc);                         // init GC threads
    initialiseJNI();                                 // init JNI lock

    // No need to check for exception - if one occurs, signalException aborts VM
    findSystemClass("java/lang/NoClassDefFoundError");
    findSystemClass("java/lang/ClassFormatError");
    findSystemClass("java/lang/NoSuchFieldError");
    findSystemClass("java/lang/NoSuchMethodError");

    // nasty nasty hack needed with Sun's system classes
    class = findSystemClass("java/lang/System");

    mb = findMethod(class, "initializeSystemClass", "()V");
    if(mb && (mb->access_flags & ACC_STATIC))
        executeStaticMethod(class, mb);

    VM_initing = FALSE;
}

void showUsage(char *name) {}

int parseMemValue(char *str) {}

int parseCommandLine(int argc, char *argv[]) {
    int i;

    for(i = 1; i < argc; i++) {
        if(*argv[i] != '-') {
            if(min_heap > max_heap) {
                printf("Minimum heap size greater than max!\n");
                exit(0);
            }
            return i;
        }

        if(strcmp(argv[i], "-help") == 0)
            break;

        else if(strcmp(argv[i], "-version") == 0) {
            printf("JamVM version %s\n", "1.0");
            printf("Copyright (C) 2003 Robert Lougher <rob@lougher.demon.co.uk>\n\n");
            printf("This program is free software; you can redistribute it and/or\n");
            printf("modify it under the terms of the GNU General Public License\n");
            printf("as published by the Free Software Foundation; either version 2,\n");
            printf("or (at your option) any later version.\n\n");
            printf("This program is distributed in the hope that it will be useful,\n");
            printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
            printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
            printf("GNU General Public License for more details.\n");
            exit(0);
        } else if(strcmp(argv[i], "-verbose") == 0)
            verboseclass = TRUE;

        else if(strcmp(argv[i], "-verbosegc") == 0)
            verbosegc = TRUE;

        else if(strcmp(argv[i], "-noasyncgc") == 0)
            noasyncgc = TRUE;

        else if(strncmp(argv[i], "-ms", 3) == 0) {
            min_heap = parseMemValue(argv[i]+3);
            if(min_heap < MIN_HEAP) {
                printf("Invalid minimum heap size: %s (min %dK)\n", argv[i], MIN_HEAP/KB);
                exit(0);
            }
        } else if(strncmp(argv[i], "-mx", 3) == 0) {
            max_heap = parseMemValue(argv[i]+3);
            if(max_heap < MIN_HEAP) {
                printf("Invalid maximum heap size: %s (min is %dK)\n", argv[i], MIN_HEAP/KB);
                exit(0);
            }
        } else if(strncmp(argv[i], "-ss", 3) == 0) {
            java_stack = parseMemValue(argv[i]+3);
            if(java_stack < MIN_STACK) {
                printf("Invalid Java stack size: %s\n", argv[i]);
                exit(0);
            }
        } else {
            printf("Unrecognised command line option: %s\n", argv[i]);
            break;
        }
    }

    showUsage(argv[0]);
    exit(0);
}

int main(int argc, char *argv[]) {
    printf("Hello, World!\n");

    Class *class;        // main class
    MethodBlock *mb;     // main method
    Object *array;
    char *cpntr;
    u4 *args;
    int i;

    // command line
    int class_arg = parseCommandLine(argc, argv);

    initVM();            // init vm

    for(cpntr = argv[class_arg]; *cpntr; cpntr++)
        if(*cpntr == '.')
            *cpntr = '/';

    // find entry class
    class = findSystemClass(argv[class_arg]);
    if(exceptionOccured()) {
        printException();
        exit(0);
    }

    // lookup public static void main
    mb = lookupMethod(class, "main", "([Ljava/lang/String;)V");
    if(!mb || !(mb->access_flags & ACC_STATIC)) {
        printf("Static method \"main\" not found in %s\n", argv[class_arg]);
        exit(0);
    }

    // Create the String array holding the command line args
    array = allocArray(findSystemClass("java/lang/String"), argc-class_arg-1, 4);
    args = INST_DATA(array)-class_arg;

    for(i = class_arg+1; i < argc; i++)
        args[i] = (u4)Cstr2String(argv[i]);

    // Call the main method
    executeStaticMethod(class, mb, array);

    if(exceptionOccured())
        printException();

    // main thread condition wait
    mainThreadWaitToExitVM();


    printf("Bye, World!\n");
    return 0;
}