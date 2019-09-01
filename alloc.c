//
// Created by chengwenjie on 2019/8/29.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "jam.h"
#include "alloc.h"
#include "thread.h"

/* Trace GC heap mark/sweep phases - useful for debugging heap
 * corruption */
#ifdef TRACEGC
#define TRACE_GC(x) printf x
#else
#define TRACE_GC(x)
#endif

/* Trace class, object and array allocation */
#ifdef TRACEALLOC
#define TRACE_ALLOC(x) printf x
#else
#define TRACE_ALLOC(x)
#endif

/* Trace object finalization */
#ifdef TRACEFNLZ
#define TRACE_FNLZ(x) printf x
#else
#define TRACE_FNLZ(x)
#endif

#define OBJECT_GRAIN		8
#define	ALLOC_BIT		1

#define HEADER(ptr)		*((unsigned int*)ptr)
#define HDR_SIZE(hdr)		(hdr & ~(ALLOC_BIT|FLC_BIT))
#define HDR_ALLOCED(hdr)	(hdr & ALLOC_BIT)

/* 1 word header format
  31                                       210
   -------------------------------------------
  |              block size               |   |
   -------------------------------------------
                                             ^ alloc bit
                                            ^ flc bit
*/

static int verbosegc;

typedef struct chunk {
    unsigned int header;
    struct chunk *next;
} Chunk;

static Chunk *freelist;
static Chunk **chunkpp = &freelist;

static char *heapbase;
static char *heaplimit;
static char *heapmax;

static int heapfree;

static unsigned int *markBits;
static int markBitSize;

static Object **has_finaliser_list = NULL;
static int has_finaliser_count    = 0;
static int has_finaliser_size     = 0;

static Object **run_finaliser_list = NULL;
static int run_finaliser_start    = 0;
static int run_finaliser_end      = 0;
static int run_finaliser_size     = 0;

static VMLock heap_lock;
static VMLock has_fnlzr_lock;
static VMWaitLock run_fnlzr_lock;

static Object *oom;

#define LIST_INCREMENT		1000

#define LOG_BYTESPERBIT		LOG_OBJECT_GRAIN /* 1 mark bit for every OBJECT_GRAIN bytes of heap */
#define LOG_MARKSIZEBITS	5
#define MARKSIZEBITS		32

#define MARKENTRY(ptr)	((((char*)ptr)-heapbase)>>(LOG_BYTESPERBIT+LOG_MARKSIZEBITS))
#define MARKOFFSET(ptr)	(((((char*)ptr)-heapbase)>>LOG_BYTESPERBIT)&(MARKSIZEBITS-1))
#define MARK(ptr)	markBits[MARKENTRY(ptr)]|=1<<MARKOFFSET(ptr)
#define IS_MARKED(ptr)	(markBits[MARKENTRY(ptr)]&(1<<MARKOFFSET(ptr)))

#define IS_OBJECT(ptr)	(((char*)ptr) > heapbase) && \
                        (((char*)ptr) < heaplimit) && \
                        !(((unsigned int)ptr)&(OBJECT_GRAIN-1))

void allocMarkBits() {
    int no_of_bits = (heaplimit-heapbase)>>LOG_BYTESPERBIT;
    markBitSize = (no_of_bits+MARKSIZEBITS-1)>>LOG_MARKSIZEBITS;

    markBits = (unsigned int *) malloc(markBitSize*sizeof(*markBits));

    TRACE_GC(("Allocated mark bits - size is %d\n", markBitSize));
}

void initialiseAlloc(int min, int max, int verbose) {
    #ifdef USE_MALLOC
        /* Don't use mmap - malloc max heap size */
        char *mem = (char*)malloc(max);
        min = max;
    #else
        // just mmap
        char *mem = (char*)mmap(0, max, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    #endif

    if((int)mem <= 0) {
        printf("Couldn't allocate the heap.  Aborting.\n");
        exit(0);
    }
    // Align heapbase so that start of heap + HEADER_SIZE is object aligned
    // (n+m-1)&~(m-1)
    // 把 mem+HEADER_SIZE 这个地址调整到  OBJECT_GRAIN（8） 对齐
    // 这样 heapbase + HEADER_SIZE 也就是和 OBJECT_GRAIN 是对齐的
    heapbase = (char*)(((uintptr_t)mem+HEADER_SIZE+OBJECT_GRAIN-1)&~(OBJECT_GRAIN-1))-HEADER_SIZE;

    // Ensure size of heap is multiple of OBJECT_GRAIN
    // (args->min_heap-(heapbase-mem))&~(OBJECT_GRAIN-1) 的作用
    // 是将 min_heap 调整到能被 OBJECT_GRAIN 整除，且不会超过自身
    heaplimit = heapbase+((min-(heapbase-mem))&~(OBJECT_GRAIN-1));

    heapmax = heapbase+((max-(heapbase-mem))&~(OBJECT_GRAIN-1));

    freelist = (Chunk*)heapbase;
    freelist->header = heapfree = heaplimit-heapbase;
    freelist->next = NULL;

    TRACE_GC(("Alloced heap size 0x%x\n",heaplimit-heapbase));
    // 用来标记内存, 和 gc 有关
    allocMarkBits();

    initVMLock(heap_lock);
    initVMLock(has_fnlzr_lock);
    initVMWaitLock(run_fnlzr_lock);

    verbosegc = verbose;
}

static void doMark(Thread *self) {}

static int doSweep(Thread *self) {}

/* Run all outstanding finalizers.  This is called synchronously
 * if we run out of memory and asyncronously by the finalizer
 * thread.  In both cases suspension is disabled, to reduce
 * back-to-back enable/disable... */

static Thread *runningFinalizers = NULL;
static int runFinalizers() {}

Object *allocObject(Class *class) {}

Object *allocArray(Class *class, int size, int el_size) {}


int gc0() {}

int gc1() {}

// expand heap with min
// don't need malloc, just move pointer
void expandHeap(int min) {
    Chunk *chunk, *new;
    int delta;

    if(verbosegc)
        printf("<GC: Expanding heap - minimum needed is %d>\n", min);

    delta = (heaplimit-heapbase)/2;
    delta = delta < min ? min : delta;

    if((heaplimit + delta) > heapmax)
        delta = heapmax - heaplimit;

    // Ensure new region is multiple of object grain in size
    delta = (delta&~(OBJECT_GRAIN-1));

    if(verbosegc)
        printf("<GC: Expanding heap by %d bytes>\n", delta);

    // The freelist is in address order - find the last free chunk and add the new area to the end
    // chunk to end
    for(chunk = freelist; chunk->next != NULL; chunk = chunk->next);

    new = (Chunk*)heaplimit;
    new->header = delta;
    new->next = 0;

    chunk->next = new;
    heaplimit += delta;
    heapfree += delta;

    // The heap has increased in size - need to reallocate the mark bits to cover new area
    free(markBits);
    allocMarkBits();
}

// gc malloc
void *gcMalloc(int len) {
    static int state = 0; /* allocation failure action */

    int n = (len+HEADER_SIZE+OBJECT_GRAIN-1)&~(OBJECT_GRAIN-1);
    Chunk *found;
    int largest;
    Thread *self;
#ifdef TRACEALLOC
    int tries;
#endif

    /* See comment below */
    char *ret_addr;

    disableSuspend(self = threadSelf());  // disable suspend
    lockVMLock(heap_lock, self);          // lock heap

    // Scan freelist looking for a chunk big enough to satisfy allocation request
    for(;;) {
#ifdef TRACEALLOC
        tries = 0;
#endif
        while(*chunkpp) {
            int len = (*chunkpp)->header;

            if(len == n) {
                found = *chunkpp;
                *chunkpp = found->next;
                goto gotIt;
            }

            if(len > n) {
                Chunk *rem;
                found = *chunkpp;
                rem = (Chunk*)((char*)found + n);
                rem->header = len - n;
                rem->next = found->next;
                *chunkpp = rem;
                goto gotIt;
            }
            chunkpp = &(*chunkpp)->next;
#ifdef TRACEALLOC
            tries++;
#endif
        }

        if(verbosegc)
            printf("<GC: Alloc attempt for %d bytes failed (state %d).>\n", n, state);

        switch(state) {

            case 0:
                largest = gc0();
                if(n <= largest)
                    break;

                state = 1;

            case 1: {
                int res;

                unlockVMLock(heap_lock, self);
                res = runFinalizers();
                lockVMLock(heap_lock, self);

                if(state == 1) {
                    if(res) {
                        largest = gc0();
                        if(n <= largest) {
                            state = 0;
                            break;
                        }
                    }
                    state = 2;
                }
                break;

                case 2:
                    if(heaplimit < heapmax) {
                        expandHeap(n);
                        state = 0;
                        break;
                    } else {
                        if(verbosegc)
                            printf("<GC: Stack at maximum already - completely out of heap space>\n");

                        state = 3;
                        /* Reset next allocation block
                 * to beginning of list */
                        chunkpp = &freelist;
                        enableSuspend(self);
                        unlockVMLock(heap_lock, self);
                        signalException("java/lang/OutOfMemoryError", NULL);
                        return NULL;
                    }
                break;
            }

            case 3:
                /* Already throwing an OutOfMemoryError in some thread.  In both
                 * cases, throw an already prepared OOM (no stacktrace).  Could have a
                 * per-thread flag, so we try to throw a new OOM in each thread, but
                 * if we're this low on memory I doubt it'll make much difference.
                 */

                state = 0;
                enableSuspend(self);
                unlockVMLock(heap_lock, self);
                setException(oom);
                return NULL;
                break;
        }
    }

    // space found
    gotIt:
#ifdef TRACEALLOC
    printf("<ALLOC: took %d tries to find block.>\n", tries);
#endif

    heapfree -= n;

    /* Mark found chunk as allocated */
    found->header = n | ALLOC_BIT;

    /* Found is a block pointer - if we unlock now, small window
     * where new object ref is not held and will therefore be gc'ed.
     * Setup ret_addr before unlocking to prevent this.
     */

    ret_addr = ((char*)found)+HEADER_SIZE;
    memset(ret_addr, 0, n-HEADER_SIZE);
    enableSuspend(self);
    unlockVMLock(heap_lock, self);

    return ret_addr;
}

void initialiseGC(int noasyncgc) {}

Object *allocMultiArray(Class *array_class, int dim, int *count) {

    int i;
    Object *array;

    if(dim > 1) {

        Class *aclass = findArrayClassFromClass(CLASS_CB(array_class)->name+1, array_class);
        array = allocArray(array_class, *count, 4);

        if(array == NULL)
            return NULL;

        for(i = 1; i <= *count; i++)
            INST_DATA(array)[i] = (u4)allocMultiArray(aclass, dim-1, count+1);
    } else {
        int el_size;

        switch(CLASS_CB(array_class)->name[1]) {
            case 'B':
            case 'Z':
                el_size = 1;
                break;

            case 'C':
            case 'S':
                el_size = 2;
                break;

            case 'I':
            case 'F':
            case 'L':
                el_size = 4;
                break;

            default:
                el_size = 8;
                break;
        }
        array = allocArray(array_class, *count, el_size);
    }

    return array;
}

Class *allocClass() {
    // gc malloc
    Class *class = (Class*)gcMalloc(sizeof(ClassBlock)+sizeof(Class));
    TRACE_ALLOC(("<ALLOC: allocated class object @ 0x%x>\n", class));
    return class;
}

void markObject(Object *object) {}

Object *allocTypeArray(int type, int size) {
    Class *class;
    int el_size;

    switch(type) {
        case T_BYTE:
        case T_BOOLEAN:
            class = findArrayClass("[B");
            el_size = 1;
            break;

        case T_CHAR:
            class = findArrayClass("[C");
            el_size = 2;
            break;

        case T_SHORT:
            class = findArrayClass("[S");
            el_size = 2;
            break;

        case T_INT:
            class = findArrayClass("[I");
            el_size = 4;
            break;

        case T_FLOAT:
            class = findArrayClass("[F");
            el_size = 4;
            break;

        case T_DOUBLE:
            class = findArrayClass("[D");
            el_size = 8;
            break;

        case T_LONG:
            class = findArrayClass("[J");
            el_size = 8;
            break;

        default:
            printf("Invalid array type %d - aborting VM...\n", type);
            exit(0);
    }

    return allocArray(class, size, el_size);
}