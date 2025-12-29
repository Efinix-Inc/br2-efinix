PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
CFLAGS = -O2 `$(PKG_CFG) --cflags --libs libgpiod`

all: gpiod.c
	$(CROSS_COMPILE)gcc $(CFLAGS) gpiod.c -o gpiod

clean:
	rm gpiod
