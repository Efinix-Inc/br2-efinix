CFLAGS = -lm -O2  -fno-stack-protector -w -Wl,--no-relax -ffast-math

all:
	$(CROSS_COMPILE)gcc $(CFLAGS) -o raystone raystone.c

clean:
	rm raystone
