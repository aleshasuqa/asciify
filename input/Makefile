# Makefile for split.c
CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

all: split

split: split.o

clean:
	rm -f split
	rm -f *.o
	rm -f *.d

# End of Makefile