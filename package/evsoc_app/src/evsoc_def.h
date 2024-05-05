#ifndef _EVSOC_DEF_H_
#define _EVSOC_DEF_H_

#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdint.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "comm_def.h"

static bool endsWith(const std::string& str, const std::string suffix) {
    if (suffix.length() > str.length()) { return false; }
    
    return (str.rfind(suffix) == (str.length() - suffix.length()));
}

#endif
