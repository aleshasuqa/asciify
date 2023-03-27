CC = gcc
CFLAGS = -g -Wall -Wextra

all: video_manager

video_manager: video_manager.o
video_manager.o: video_manager.c

.PHONY: clean

clean:
	${RM} video_manager video_manager.o *.png
