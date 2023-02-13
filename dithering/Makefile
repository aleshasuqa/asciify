# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

all: display dither 

SRC = display.c dither.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

display: display.o
dither: dither.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END
