CC = gcc
CFLAGS = -g -Wall -Wextra
LDFLAGS = -lSDL2 -lSDL2_image

all: error_diff

error_diff: error_diff.o
error_diff.o: error_diff.c

.PHONY: clean

clean:
	${RM} error_diff error_diff.o output.png
