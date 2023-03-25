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

tags: *.ino *.cpp *.h
	ctags *.ino *.cpp *.h

vi:
	vim Makefile \
		defines.h \
		rotational-button-for-feeding-pump.ino \
		tone.cpp \
		tone.h \
		pitches.h \
		~/Arduino/libraries/libtrb/src/ringbuffer.c \
		~/Arduino/libraries/libtrb/src/trb.h
