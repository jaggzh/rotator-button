#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H
#ifndef _IN_RINGBUFFER_C

#include <stdint.h>

#define RB_DTYPE   float
#define RB_IDXTYPE uint8_t

struct ringbuffer_st {
    RB_DTYPE *d;          // data
    RB_IDXTYPE hd, tl;    // head tail indices
    RB_IDXTYPE sz;        // *total* buffer element count
	#ifdef RB_OPT_MINMAX
		RB_DTYPE mn, mx;  // min max
	#endif
};
typedef struct ringbuffer_st rb_st;

void ringbuffer_init(rb_st *rb, RB_IDXTYPE len);
void ringbuffer_setall(rb_st *rb, RB_DTYPE v);
void ringbuffer_minmax(cp_st *cp, RB_DTYPE *mn, RB_DTYPE *mx);

#endif
