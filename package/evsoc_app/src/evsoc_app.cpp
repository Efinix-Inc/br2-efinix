/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <queue>
#include "evsoc_def.h"
#include "evsoc_cli.h"
#include "evsoc_func.h"

using namespace std;
using namespace cv;

///
queue<unsigned int> worker_queue;
bool                ignored_key_event = false;
bool                capture_done_event = false;
pthread_mutex_t worker_mutex;

///
pthread_t worker_hdl;
int worker_stat = 10;

///
pthread_t worker2_hdl;
int worker2_stat = 10;

int  cap_vid_len = 0;
char cap_vid_name[256] = {0};

VideoWriter *video_write_hdl = NULL;

///
bool check_para_num(int argc, int pNum)
{
    if (argc != (pNum + 1))
    {
        printf("Error Para number %d, should %d\n", argc, pNum);
        return false;
    }
    return true;
}

char *get_para(char **argv, int pIdx)
{
    return argv[pIdx + 1];
}

int set_hwacc(int argc, char **argv)
{
    if (!check_para_num(argc, 1))
    {
        return CMD_RET_FAIL;
    }

    int lVal;
    if (!get_enum(argv[1], lVal) || (lVal >= 3))
    {
        printf("Error value: %s\n", argv[1]);
        return CMD_RET_FAIL;
    }

    int fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_SET_MODE, lVal);
    close(fd);

    return CMD_RET_OK;
}

int capture_view(int argc, char **argv)
{
    if (!check_para_num(argc, 1))
    {
        return CMD_RET_FAIL;
    }
    unsigned int lTmpTime1;
    const char *lFileName = argv[1];

    string lNameStr = lFileName;

    if (!endsWith(lNameStr, ".jpg"))
    {
        lNameStr += ".jpg";
    }
    
    {
        void *buffer;
        void *mapBuf;
        FILE *lFile;
        unsigned long lCmdPara = CMD_SET_CAPTURE_FULL;

        lTmpTime1 = get_ms_time();
        int lRet;
        int bufLen = (FRAME_WIDTH * FRAME_HEIGHT * 4);
        int fd = open("/dev/evsoc", O_RDWR);
        ioctl(fd, CMD_SET_CAPTURE, lCmdPara);
        close(fd);
        // printf("Diff1:%d\n", get_ms_time() - lTmpTime1); // Diff1:1
        usleep(200000);

        buffer = malloc(bufLen);
        lTmpTime1 = get_ms_time();
        fd = open("/dev/evsoc", O_RDWR);
        mapBuf = mmap(NULL, bufLen, PROT_READ, MAP_SHARED, fd, 0);
        memcpy(buffer, mapBuf, bufLen);
        munmap(mapBuf, bufLen);
        close(fd);

        lTmpTime1 = get_ms_time();
        crt_pic(lNameStr.c_str(), FRAME_WIDTH, FRAME_HEIGHT, buffer);
        // printf("Diff3:%d\n", get_ms_time() - lTmpTime1); // Diff3:4362
        // crt_pic_test(FRAME_WIDTH, FRAME_HEIGHT, buffer);
        free(buffer);

        return CMD_RET_OK;
    }
}

int prt_char(int argc, char **argv)
{
    int lCol;
    int lRow;
    int lType = 0;
    // unsigned char lChar;

    if ((argc != 4) && (argc != 5))
    {
        printf("Error Para number %d, should 3 or 4\n", argc);
        return CMD_RET_FAIL;
    }

    if (!get_int(argv[1], lRow))
    {
        return CMD_RET_FAIL;
    }

    if (!get_int(argv[2], lCol))
    {
        return CMD_RET_FAIL;
    }

    const char *lStr = argv[3];
    int lStrLen = strlen(lStr);
    if (lStrLen == 0)
    {
        return CMD_RET_FAIL;
    }

    if (argc == 5)
    {
        if (!get_enum(argv[4], lType) || (lType > 3))
        {
            return CMD_RET_FAIL;
        }
    }
    unsigned long lFlg = 0;
    int fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_SET_LAYER, lFlg);

    for (int lTmpIdx = 0; lTmpIdx < lStrLen; lTmpIdx++)
    {
        unsigned char lChar = lStr[lTmpIdx];
        unsigned long lVal = lChar | ((lType << 8) & 0xff00) | ((lRow << 16) & 0xff0000) | ((lCol << 24) & 0xff000000);
        ioctl(fd, CMD_SET_CHAR, lVal);
        lCol++;
    }
    lFlg = 1;
    ioctl(fd, CMD_SET_LAYER, lFlg);
    close(fd);

    printf("End point is %d, %d\n", lRow, lCol);
    return CMD_RET_OK;
}

int clr_disp(int argc, char **argv)
{
    unsigned long lFlg = 3;
    int fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_SET_LAYER, lFlg);
    close(fd);
    return CMD_RET_OK;
}

int set_binning(int argc, char **argv)
{
    if (!check_para_num(argc, 1))
    {
        return CMD_RET_FAIL;
    }
    int lVal;
    if (!get_enum(argv[1], lVal))
    {
        return CMD_RET_FAIL;
    }

    int fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_SET_CAM_BINNING, lVal);
    close(fd);
    return CMD_RET_OK;
}

int capture_video(int argc, char **argv)
{
    if (!check_para_num(argc, 2))
    {
        return CMD_RET_FAIL;
    }

    if (0 != cap_vid_len)
    {
        printf("Capturing video is running.");
        return CMD_RET_FAIL;
    }

    char *lPara;
    int lInVal;

    lPara = get_para(argv, 0);
    strncpy(cap_vid_name, lPara, sizeof(cap_vid_name));

    lPara = get_para(argv, 1);
    if (!get_int(lPara, lInVal))
    {
        return CMD_RET_FAIL;
    }

    if (lInVal > 30)
    {
        lInVal = 30;
    }
    cap_vid_len = lInVal;
    return CMD_RET_OK;
}

void capture_auto_video(int pSec)
{
    if (0 != cap_vid_len)
    {
        return;
    }
    long lSec, lUsec;
	get_tv_time(lSec, lUsec);
    sprintf(cap_vid_name, "video_%x_%x.avi", lSec, lUsec);
    cap_vid_len = pSec;
}

static void signal_handler(int signum, siginfo_t *info, void *context)
{
    // 打印接收到的信号值
    // printf("signal_handler\n");
    int lVal = info->si_value.sival_int;
    if ((lVal & 0xff) == SIG_TYPE_CAPTURE_DONE)
    {
        capture_done_event = true;
        return;
    }

    if ((lVal & 0xff) == SIG_TYPE_KEY_ACT)
    {
        if (ignored_key_event)
        {
            return;
        }
    }

    pthread_mutex_lock(&worker_mutex);
    worker_queue.push(lVal);
    pthread_mutex_unlock(&worker_mutex);
}

void *worker_run(void *data)
{
    unsigned int lVal;
    while (worker_stat > 0)
    {
        poll_title_msg(0);
        if (worker_queue.empty())
        {
            usleep(10000);
            ignored_key_event = false;
            continue;
        }

        pthread_mutex_lock(&worker_mutex);
        lVal = worker_queue.front();
        worker_queue.pop();
        pthread_mutex_unlock(&worker_mutex);

        if ((lVal & 0xff) == SIG_TYPE_KEY_ACT)
        {
            usleep(500000);
            if (worker_queue.empty())
            {
                printf("\nstart capture pic\n");
                ignored_key_event = true;
                capture_auto_pic();
                ignored_key_event = false;
            }
            else
            {
                pthread_mutex_lock(&worker_mutex);
                lVal = worker_queue.front();
                worker_queue.pop();
                pthread_mutex_unlock(&worker_mutex);
                if ((lVal & 0xff) == SIG_TYPE_KEY_ACT)
                {
                    ignored_key_event = true;
                    printf("\nstart capture video\n"); 
                    capture_auto_video(10);
                    ignored_key_event = false;
                }
                else
                {
                    printf("\nstart capture pic\n");
                    ignored_key_event = true;
                    capture_auto_pic();
                    ignored_key_event = false;
                }
            }
        }
    }
    return NULL;
}

int lDbgTime[100];
int lDbgIdx = 0;

void *worker2_run(void *data)
{
    unsigned int lVal;
    int fd;
    int bufLen = (FRAME_WIDTH * FRAME_HEIGHT * 4);
    int lRet;
    void *buffer = NULL;
    void *mapBuf;

    unsigned int lCurTime = 0;
    unsigned int lPreTime = 0;
    unsigned int lDiffTime = 0;
    unsigned long lCmdPara = CMD_SET_CAPTURE_PART;

    unsigned int lTmpTime1 = 0;
    unsigned int lTmpTime2 = 0;
    Mat mat(FRAME_HEIGHT / 2, FRAME_WIDTH / 2, CV_8UC3);

    int lCapTimer = 0;
    vector<Mat *> mCapVect; 
    while (worker2_stat > 0)
    {
        if (cap_vid_len == 0)
        {
            usleep(200000);
            continue;
        }
        else if (lCapTimer == 0)
        {
            lCapTimer = cap_vid_len * 5 + 1;
        }

        lCurTime = get_ms_time();
        lDiffTime = lCurTime - lPreTime;
        if (lDiffTime < 200)
        {
            usleep(10);
            continue;
        }
        lPreTime = lCurTime;
        lCapTimer--;

        if (video_write_hdl == NULL)
        {
            string lNameStr = cap_vid_name;

            if (!endsWith(lNameStr, ".avi"))
            {
                lNameStr += ".avi";
                strncpy(cap_vid_name, lNameStr.c_str(), sizeof(cap_vid_name));
            }
            
            lTmpTime1 = get_ms_time();
            printf("start to capture video: %s\n", lNameStr.c_str());
            lDbgTime[lDbgIdx++] = get_ms_time() - lTmpTime1; // 0

            lTmpTime1 = get_ms_time();
            title_msg("start to capture video ...");
            lDbgTime[lDbgIdx++] = get_ms_time() - lTmpTime1; // 2

            lTmpTime1 = get_ms_time();
            Size sizeFrame(FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
            video_write_hdl = new VideoWriter();
            video_write_hdl->open(lNameStr.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 5, sizeFrame, true);
            if (!video_write_hdl->isOpened())
            {
                lCapTimer = 0;
                cap_vid_len = 0;
                delete video_write_hdl;
                video_write_hdl = NULL;
                continue;
            }
            else
            {
                fd = open("/dev/evsoc", O_RDWR);
                ioctl(fd, CMD_SET_CAPTURE, lCmdPara);
                close(fd);
            }
            lDbgTime[lDbgIdx++] = get_ms_time() - lTmpTime1; // 5

            lPreTime = lCurTime;
            continue;
        }
        
        Mat * lCapMatPtr = new Mat(FRAME_HEIGHT / 2, FRAME_WIDTH / 2, CV_8UC3);

        lTmpTime1 = get_ms_time();
        fd = open("/dev/evsoc", O_RDWR);
        mapBuf = mmap(NULL, bufLen, PROT_READ, MAP_SHARED, fd, 0);
        // fill_mat(mat, mapBuf);
        fill_mat(*lCapMatPtr, mapBuf);
        munmap(mapBuf, bufLen);
        ioctl(fd, CMD_SET_CAPTURE, lCmdPara);
        close(fd);
        lDbgTime[lDbgIdx++] = get_ms_time() - lTmpTime1; // 367 ~ 460 ==> 115
        
        #if 0
        lTmpTime1 = get_ms_time();
        video_write_hdl->write(mat);
        lDbgTime[lDbgIdx++] = get_ms_time() - lTmpTime1; // 2049 ~ 3066  ==> 564
        #else
        mCapVect.push_back(lCapMatPtr);
        #endif

        if (lCapTimer == 0)
        {
            int lLen = mCapVect.size();
            // printf("save to video:%d\n", lLen);
            printf("save video to file:%d", lLen);

            if (lLen > 0)
            {
                lTmpTime1 = get_ms_time();
                for (int lTmpI = 0; lTmpI < lLen; lTmpI++)
                {
                    video_write_hdl->write(*mCapVect[lTmpI]);
                    if (0 == (lTmpI % 3))
                    { 
                        title_msg("Save:%d", (lLen - lTmpI) / 3);
                    }
                }
                printf("writetime:%d\r\n", get_ms_time() - lTmpTime1);

                lTmpTime1 = get_ms_time();
                for (int lTmpI = 0; lTmpI < lLen; lTmpI++)
                {
                    delete mCapVect[lTmpI];
                }
                mCapVect.clear();
                printf("delete:%d\r\n", get_ms_time() - lTmpTime1);
            }
            ///
            usleep(10000);
            video_write_hdl->release();
            usleep(10000);
            delete video_write_hdl;
            video_write_hdl = NULL;
            {
                printf("Done to capture video\n");
                title_msg("Done to capture video:%s", cap_vid_name);
            }

            ///
            lDbgIdx = 0;
            lPreTime = 0;
            lCurTime = 0;
            lCapTimer = 0;
            cap_vid_len = 0;

            ///
            for (int lTmpI = 0; lTmpI < 20; lTmpI++)
            {
                // printf("%d:%d\r\n", lTmpI, lDbgTime[lTmpI]);
            }
        }
        else
        {
            if ((lCapTimer % 5) == 0)
            {
                title_msg("Capture:%d", int(lCapTimer / 5));
            }
        }
    }

    if (video_write_hdl)
    {
        video_write_hdl->release();
        delete video_write_hdl;
    }
    if (buffer)
    {
        free(buffer);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    // /sbin/modprobe
    printf("#\n");
    system("modprobe evsoc");

    pthread_mutex_init(&worker_mutex, NULL);
    pthread_create(&worker_hdl, NULL, worker_run, (void *)&worker_stat);
    pthread_create(&worker2_hdl, NULL, worker2_run, (void *)&worker2_stat);

    ///////
    reg_cmd("hwacc", set_hwacc, "[None|Erosion|Dilation]", "hardware accelerate");
    reg_enum("None", 0);
    reg_enum("Erosion", 1);
    reg_enum("Dilation", 2);

    /////
    reg_cmd("cappic", capture_view, "[file_name.jpg]", "capture video to file.");
    reg_cmd("capvideo", capture_video, "[file_name][len]", "capture video to avi.");

    /////
    reg_cmd("binning", set_binning, "[X1Bin|X2Bin|X4Bin]", "set camera binning.");
    reg_enum("X1Bin", 0);
    reg_enum("X2Bin", 1);
    reg_enum("X4Bin", 2);

    /////
    reg_cmd("clrchar", clr_disp, "", "clear all the char.");

    /////
    reg_cmd("prtchar", prt_char, "[row][col][string][Green|Blue|White|Black]", "print char to monitor.");
    reg_enum("Green", 0);
    reg_enum("Blue", 1);
    reg_enum("White", 2);
    reg_enum("Black", 3);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    int pid = getpid();
    int fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_SET_PID, pid);
    close(fd);

    /////
    if (argc > 1)
    {
        char **lArgv = &argv[1];
        int lArgc = argc - 1;
        cmd_proc(lArgc, lArgv);
    }
    else
    {
        cli_main();
        printf("Bye\n");
    }

    poll_title_msg(1);

    ///
    pid = 0;
    fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_SET_PID, pid);
    close(fd);

    ///
    worker_stat = 0;
    worker2_stat = 0;
    pthread_mutex_destroy(&worker_mutex);

    ///////////
    system("sync");
    printf("\n");

    return 1;
}
