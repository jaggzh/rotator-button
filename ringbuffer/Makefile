all: tests

tests: ringbuffer_test_run

ringbuffer_test_run: tests/ringbuffer_test
	tests/ringbuffer_test

tests/ringbuffer_test: tests/ringbuffer_test.c ringbuffer.c ringbuffer.h
	gcc -I. -Wall -o tests/ringbuffer_test tests/ringbuffer_test.c ringbuffer.c

vi:
	vim Makefile tests/ringbuffer_test.c ringbuffer.c ringbuffer.h
