CC=gcc
CFLAGS=-I. -lglut -lGL -lm -lGLU

all:
	$(CC) -std=c99 main.c tinyspline.c $(CFLAGS) -o test.out
	$(CC) -std=c99 -g calculate_distance.c tinyspline.c $(CFLAGS) -o distance.out
