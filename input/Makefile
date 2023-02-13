# Makefile for split.c
CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image` -g
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

all: split

resize: resize.o
resize1: resize1.o
split: split.o

clean:
	rm -f split resize resize1
	rm -f *.o
	rm -f *.d

# End of Makefile
