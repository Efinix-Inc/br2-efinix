CC = $(CROSS_COMPILE)gcc
CFLAGS += -O2 -Wall -Wextra

all:
	$(CC) $(CFLAGS) spi.c spi_app.c -o spi_app

clean:
	rm -rf spi_app
