CXX = $(TARGET_CXX)
PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
CFLAGS = -O2 `$(PKG_CFG) --cflags --libs opencv`

all: Blob.cpp tracking.cpp
	$(CXX) $(CFLAGS) $^ -o cv2_tracking

clean:
	rm cv2_tracking.o cv2_tracking
