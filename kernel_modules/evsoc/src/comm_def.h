#ifndef _COMM_DEF_H_
#define _COMM_DEF_H_

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"

#define FRAME_WIDTH    1280
#define FRAME_HEIGHT   720

#define DEMO_MAGIC           0x68
#define CMD_SET_MODE         _IOW(DEMO_MAGIC, 1, unsigned long)
#define CMD_SET_CAM_SIZE     _IOW(DEMO_MAGIC, 3, unsigned long)
#define CMD_SET_CAM_BINNING  _IOW(DEMO_MAGIC, 5, unsigned long)
#define CMD_SET_CAPTURE      _IOW(DEMO_MAGIC, 7, unsigned long)
#define CMD_SET_CHAR         _IOW(DEMO_MAGIC, 9, unsigned long)
#define CMD_SET_LAYER        _IOW(DEMO_MAGIC, 11, unsigned long)
#define CMD_SET_PID          _IOW(DEMO_MAGIC, 13, unsigned long)
#define CMD_SET_MSG          _IOW(DEMO_MAGIC, 15, unsigned long)
#define CMD_CLR_MSG          _IOW(DEMO_MAGIC, 17, unsigned long)

/////////
#define SIG_TYPE_NONE             0
#define SIG_TYPE_CAPTURE_DONE     1
#define SIG_TYPE_KEY_ACT          2

/////////
#define CMD_SET_CAPTURE_FULL 1
#define CMD_SET_CAPTURE_PART 2


#endif
