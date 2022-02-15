CXX = $(TARGET_CXX)
PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
CFLAGS = -O2 `$(PKG_CFG)`

all: apb3app.c
	$(CXX) $(CFLAGS) $^ -o apb3app

clean:
	rm apb3app.o apb3app
