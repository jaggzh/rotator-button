#define _IN_RINGBUFFER_C
#include "ringbuffer.h"
#include <stdlib.h>

void ringbuffer_init(rb_st *rb, RB_IDXTYPE len) {
	//struct *rb = malloc(sizeof struct ringbuffer_st);
	rb->d = malloc(sizeof(RB_DTYPE) * len);
	rb->hd=rb->tl=0;
	rb->sz = len;
}

void ringbuffer_setall(rb_st *rb, RB_DTYPE v) {
	for (int i=0; i<rb->sz; i++) rb->d[i] = v;
}

void ringbuffer_add(rb_st *rb, RB_DTYPE v) {
	rb->d[rb->hd] = v;
	rb->hd++;
	if (rb->hd >= rb->sz) rb->hd = 0;
}

void ringbuffer_minmax(rb_st *cp, RB_DTYPE *mn, RB_DTYPE *mx) {
	if (mn) mn=rb->d[0];
	if (mx) mx=rb->d[0];
	for (int i=1; i<rb->sz; i++) {
		if (mn && *mn > rb->d[i]; i++) *mn = rb->d[i];
		if (mx && *mx < rb->d[i]; i++) *mx = rb->d[i];
	}
}
