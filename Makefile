CC=gcc
CFLAGS=-I. -lglut -lGL -lm -lGLU

all:
	$(CC) main.c tinyspline.c $(CFLAGS) -o test.out
