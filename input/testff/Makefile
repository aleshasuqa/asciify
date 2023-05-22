# makefile for testPipeFF using sdl and gtk

# Compiler and Linker
CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2` `pkg-config --cflags SDL2_ttf` `pkg-config --cflags --libs gtk+-3.0`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2` `pkg-config --libs SDL2_ttf` -lm

all: testPipeFF

testPipeFF: testPipeFF.o

main: main.o

main.o: main.c

clean:
	rm -f *.o testPipeFF
	rm -f *.o main


