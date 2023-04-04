# makefile for testPipeFF using sdl

# Compiler and Linker
CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2` `pkg-config --cflags SDL2_ttf`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2` `pkg-config --libs SDL2_ttf` -lm

all: testPipeFF

testPipeFF: testPipeFF.o

clean:
	rm -f *.o testPipeFF


