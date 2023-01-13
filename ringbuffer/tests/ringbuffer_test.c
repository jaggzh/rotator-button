#include <stdio.h>
#include "ringbuffer.h"

#define RB_BUF_SIZE 10

void print_rb(rb_stp rb) {
	for (int i=0; i<rb->sz; i++) {
		printf("[%d] %d\n", i, rb->d[i]);
	}
}

int main(int argc, char *argv[]) {
	struct ringbuffer_st rbs;
	ringbuffer_init(&rbs, RB_BUF_SIZE);
	ringbuffer_setall(&rbs, 0);
	for (int i=1; i<20; i++) {
		ringbuffer_add(&rbs, i);
		print_rb(&rbs);
	}
}
