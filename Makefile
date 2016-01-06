CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -I. -lglut -lGL -lm -lGLU

all:
	$(CC) main.c tinyspline.c $(CFLAGS) -o test.out
	$(CC) calculate_distance.c tinyspline.c $(CFLAGS) -o distance.out
