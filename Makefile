CC=gcc
CFLAGS=-I. -lglut -lGL -lm -lGLU

all:
	$(CC) -std=c99 main.c tinyspline.c $(CFLAGS) -o test.out
