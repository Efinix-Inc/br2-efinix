CFLAGS = -L.-O2

all: gpio_app.c
	$(CROSS_COMPILE)gcc $(CFLAGS) gpio_app.c gpio.c -o gpio_app

clean:
	rm gpio_app.o gpio_app gpio.o
