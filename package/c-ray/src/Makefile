CC = $(CROSS_COMPILE)gcc
CFLAGS = -O3 -ffast-math

all:
	$(CC) $(CFLAGS) -o c-ray-mt c-ray-mt.c -lm -lpthread
	$(CC) $(CFLAGS) -o c-ray-f c-ray-f.c -lm

