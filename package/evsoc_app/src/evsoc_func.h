/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */

#ifndef _EVSOC_API_H_
#define _EVSOC_API_H_
#include<opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>

int crt_pic(const char * pFileName, int pWi, int pHi, void * pBuf);
int crt_pic_test(int pWi, int pHi, void * pBuf);
char * crt_auto_pic(int pWi, int pHi, void * pBuf);
char * crt_idx_pic(int pIdx, int pWi, int pHi, void * pBuf);
void capture_auto_pic(void);
void title_msg(const char* format, ...);
void poll_title_msg(int pForce);
bool fill_mat(cv::Mat & mat, void * pBuf);
bool fill_mat_half(cv::Mat & mat, void * pBuf);
unsigned int get_ms_time(void);
void get_tv_time(long & pSec, long & pUsec);
#endif
