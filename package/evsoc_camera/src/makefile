CXX=$(TARGET_CXX)
PKG_CFG = $(PKG_CONFIG_HOST_BINARY)
PROG ?= evsoc_camera                   # Program we are building
OUT ?= -o $(PROG)                 # Compiler argument for output file
SOURCES = main.c i2c.c cam_ops.c mongoose.c       # Source code files
CFLAGS = -std=c++11 -Wall -O3 -I. # Build options
CFLAGS_EXTRA = `$(PKG_CFG)  --cflags --libs opencv` -fopenmp -pthread

all: $(SOURCES)       # Build program from sources
	$(CXX) $(SOURCES) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(OUT)

clean:                    # Cleanup. Delete built program and all build artifacts
	rm -rf $(PROG) *.o *.obj
