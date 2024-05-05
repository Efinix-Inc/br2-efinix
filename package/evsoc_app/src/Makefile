CXX = $(TARGET_CXX)
PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
CFLAGS = -O2 -lpthread `$(PKG_CFG)  --cflags --libs opencv`

all: evsoc_app.cpp evsoc_func.cpp evsoc_cli.cpp
	$(CXX) $(CFLAGS) $^ -o evsoc_app

clean:
	rm *.o evsoc_app
