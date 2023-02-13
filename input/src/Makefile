# Makefile for split.c
CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image SDL2_ttf`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf`

all: split

split: split.o ssim.o

ssim: ssim.o

clean:
	rm -f split ssim
	rm -f *.o
	rm -f *.d

# End of Makefile