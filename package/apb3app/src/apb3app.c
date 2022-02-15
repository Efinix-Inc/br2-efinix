/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv)
{
	char msg[32];
	int fd; 
    int i=0;

	printf("apb3 device driver demo\n");

    for (i=0; i < 3; i++) {
        fd = open("/dev/apb", O_RDWR);
	    read(fd, msg, 32);
	    printf("%s\n", msg);
        memset(msg, 0, sizeof(msg));
        sleep(1);
        close(fd);
    }
    i=0;
    
    printf("stop generating pseudorandom number\n");
	sprintf(msg, "0x01\n");
    fd = open("/dev/apb", O_RDWR);
	write(fd, msg, strlen(msg)+1);
    close(fd);

    for (i=0; i < 3; i++) {
        fd = open("/dev/apb", O_RDWR);
	    read(fd, msg, 32);
	    printf("%s\n", msg);
        memset(msg, 0, sizeof(msg));
        sleep(1);
        close(fd);
    }
    i=0;

    printf("resume to generate pseudorandom number\n");
    fd = open("/dev/apb", O_RDWR);
	sprintf(msg, "0x00\n");
	write(fd, msg, strlen(msg)+1);
    close(fd);

    for (i=0; i < 3; i++) {
        fd = open("/dev/apb", O_RDWR);
	    read(fd, msg, 32);
	    printf("%s\n", msg);
        memset(msg, 0, sizeof(msg));
        sleep(1);
        close(fd);
    }

    return 0;
}
