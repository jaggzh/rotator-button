all: tests

tests: test_run

test_run: tests/test
	unbuffer tests/test

test_run_values: tests/test
	tests/test -n

v: test_run_values

tests/test: tests/test.c ringbuffer/ringbuffer.c
	gcc -ggdb3 -std=gnu99 -O0 -I. -I.. \
		-Wall -o "$@" \
		$^

vi:
	vim Makefile \
		mpu9250-test.ino \
		tests/test.c \
		ringbuffer/ringbuffer.c \
		ringbuffer/trb.h
