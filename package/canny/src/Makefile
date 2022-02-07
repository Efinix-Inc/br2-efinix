CXX = $(TARGET_CXX)
PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
CFLAGS = -O2 `$(PKG_CFG) --cflags --libs opencv`

all: canny.c
	$(CXX) $(CFLAGS) $^ -o canny

clean:
	rm canny.o canny
