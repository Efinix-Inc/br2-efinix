CC = $(CROSS_COMPILE)gcc
CFLAGS = -O2

all:
	$(CC) $(CFLAGS) -o tcp_server server.c
	$(CC) $(CFLAGS) -o tcp_client client.c

clean:
	rm tcp*
