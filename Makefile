all: tests

tests: test_run

test_run: tests/test
	tests/test

tests/test: tests/test.c  # tests/millis.c capsense.c capsense.h
	gcc -ggdb3 -I. -I.. \
		-Wall -o "$@" \
		"$^" \
		../ringbuffer/ringbuffer.c

vi:
	vim Makefile \
		mpu9250-test.ino \
		tests/test.c \
		../ringbuffer/ringbuffer.c \
		../ringbuffer/ringbuffer.h
