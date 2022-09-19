#CXX = $(TARGET_CXX)
#PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
CXX=g++
PKG_CFG=/usr/bin/pkg-config
CFLAGS = -O2 `$(PKG_CFG) --cflags --libs opencv`

all: detection.cpp
	$(CXX) $(CFLAGS) $^ -o cv2_detection

clean:
	rm cv2_detection.o cv2_detection
