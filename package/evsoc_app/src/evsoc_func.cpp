
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "evsoc_def.h"
#include "evsoc_func.h"

using namespace cv;
using namespace std;

// gst-launch-1.0 rtspsrc location=rtsp://localhost:8554/test latency=100 
//   ! queue ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! videoscale 
//   ! video/x-raw,width=640,height=480 ! autovideosink

bool fill_mat(Mat & mat, void * pBuf)
{
	int * lBuf = (int *)pBuf;
	for (int i = 0; i < mat.rows; ++i)
	{
		for (int j = 0; j < mat.cols; ++j)
		{
            int pos = i * mat.cols + j;
			#if 1
			Vec3b &rgba = mat.at<Vec3b>(i, j);
			rgba[2] = saturate_cast<uchar>(lBuf[pos] & 0xff);
			rgba[1] = saturate_cast<uchar>((lBuf[pos] >> 8) & 0xff);
			rgba[0] = saturate_cast<uchar>((lBuf[pos] >> 16) & 0xff);
			#else
			Vec4b &rgba = mat.at<Vec4b>(i, j);
			rgba[3] = saturate_cast<uchar>(lBuf[pos] & 0xff);
			rgba[2] = saturate_cast<uchar>((lBuf[pos] >> 8) & 0xff);
			rgba[1] = saturate_cast<uchar>((lBuf[pos] >> 16) & 0xff);
			rgba[0] = 0;
			#endif
		}
	}

	return true;
}

bool fill_mat_half(Mat & mat, void * pBuf)
{
	int * lBuf = (int *)pBuf;
	int lBufR = mat.rows * 2;
	int lBufC = mat.cols * 2;
	for (int i = 0; i < mat.rows; ++i)
	{
		for (int j = 0; j < mat.cols; ++j)
		{
            int pos = i *2 * lBufC + j * 2;

			#if 1
			Vec3b &rgba = mat.at<Vec3b>(i, j);
			rgba[2] = saturate_cast<uchar>(lBuf[pos] & 0xff);
			rgba[1] = saturate_cast<uchar>((lBuf[pos] >> 8) & 0xff);
			rgba[0] = saturate_cast<uchar>((lBuf[pos] >> 16) & 0xff);
			#else
			Vec4b &rgba = mat.at<Vec4b>(i, j);
			rgba[3] = saturate_cast<uchar>(lBuf[pos] & 0xff);
			rgba[2] = saturate_cast<uchar>((lBuf[pos] >> 8) & 0xff);
			rgba[1] = saturate_cast<uchar>((lBuf[pos] >> 16) & 0xff);
			rgba[0] = 0;
			#endif
		}
	}

	return true;
}



int crt_pic(const char * pFileName, int pWi, int pHi, void * pBuf)
{
	if ((pFileName == 0) || (pFileName[0] == 0))
	{
		printf("error file name\n");
		return 0;
	}
	
    // row, col
	string lName(pFileName);
	Mat mat(pHi, pWi, CV_8UC3);
    int * lBuf = (int *)pBuf;

	// if (!(endsWith(pFileName, ".png") || endsWith(pFileName, ".jpg") || endsWith(pFileName, ".jpeg") || endsWith(pFileName, ".bmp")))
	// {
		// printf("%s is error type, only support png, bmp, jpg, jpeg. \n", pFileName);
		// return 0;
	// }
    // 通常情况下，相同分辨率的JPEG图像文件比JPG文件大
	if (!(endsWith(pFileName, ".jpg") || endsWith(pFileName, ".jpeg") || endsWith(pFileName, ".bmp")))
	{
		printf("%s is error type, only support bmp, jpg, jpeg. \n", pFileName);
		return 0;
	}

	for (int i = 0; i < mat.rows; ++i)
	{
		for (int j = 0; j < mat.cols; ++j)
		{
            int pos = i * pWi + j;
			Vec3b &rgba = mat.at<Vec3b>(i, j);
			rgba[2] = saturate_cast<uchar>(lBuf[pos] & 0xff);
			rgba[1] = saturate_cast<uchar>((lBuf[pos] >> 8) & 0xff);
			rgba[0] = saturate_cast<uchar>((lBuf[pos] >> 16) & 0xff);
		}
	}

	vector<int>compression_params;
	compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(5);

	try
	{
		// imwrite(pFileName, mat, compression_params);
		imwrite(pFileName, mat);
		// printf("%s saved. \n", pFileName);
	}
	catch (runtime_error &ex)
	{
		printf("Error to generate:%s\n", ex.what());
		return 1;
	}
	return 0;
}


int crt_pic_test(int pWi, int pHi, void * pBuf)
{

	unsigned int lTmpTime1;
	lTmpTime1 = get_ms_time();
	Mat mat(pHi, pWi, CV_8UC3);
    int * lBuf = (int *)pBuf;
    printf("INDiff1:%d\n", get_ms_time() - lTmpTime1);
	setNumThreads(0);


    Mat mat_1(pHi, pWi, CV_8UC3);
    Mat mat_h(pHi / 2, pWi / 2, CV_8UC3);

    lTmpTime1 = get_ms_time();
	fill_mat(mat_1, pBuf);
    printf("Fill_1:%d\n", get_ms_time() - lTmpTime1); // Fill_1:575

    lTmpTime1 = get_ms_time();
	fill_mat_half(mat_h, pBuf);
    printf("Fill_1:%d\n", get_ms_time() - lTmpTime1); // Fill_1:64

	// if (!(endsWith(pFileName, ".png") || endsWith(pFileName, ".jpg") || endsWith(pFileName, ".jpeg") || endsWith(pFileName, ".bmp")))
	// {
		// printf("%s is error type, only support png, bmp, jpg, jpeg. \n", pFileName);
		// return 0;
	// }
    // 通常情况下，相同分辨率的JPEG图像文件比JPG文件大
    lTmpTime1 = get_ms_time();
	for (int i = 0; i < mat.rows; ++i)
	{
		for (int j = 0; j < mat.cols; ++j)
		{
            int pos = i * mat.cols + j;
			Vec3b &rgba = mat.at<Vec3b>(i, j);
			rgba[2] = saturate_cast<uchar>(lBuf[pos] & 0xff);
			rgba[1] = saturate_cast<uchar>((lBuf[pos] >> 8) & 0xff);
			rgba[0] = saturate_cast<uchar>((lBuf[pos] >> 16) & 0xff);
		}
	}
    printf("INDiff2:%d\n", get_ms_time() - lTmpTime1); // INDiff2:568

	// (A) Encoding with bmp : 20-40ms
    // (B) Encoding with jpg : 50-70 ms
    // (C) Encoding with png: 200-250ms

	std::vector<unsigned char> data;
    lTmpTime1 = get_ms_time();
	try
	{
		// imwrite(pFileName, mat, compression_params);
		// imwrite(pFileName, mat);
		// printf("%s saved. \n", pFileName);

		vector<int>compression_params;
		compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(10);

		Mat xframe;
		// 1280 720
        resize(mat, xframe, Size(640, 360));
		printf("resize:%d, %d\n", get_ms_time() - lTmpTime1, sizeof(xframe), INTER_NEAREST); // resize:365, 56
		lTmpTime1 = get_ms_time();
		imencode(".bmp", xframe, data); 
	}
	catch (runtime_error &ex)
	{
		printf("Error to generate:%s\n", ex.what());
		return 1;
	}
    printf("INDiff3:%d\n", get_ms_time() - lTmpTime1); // INDiff3:46
    printf("Len:%d K\n", data.size() / 1024); // Len:675 K

    lTmpTime1 = get_ms_time();
	try
	{
		vector<int>compression_params;
		// compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
		compression_params.push_back(100);

		// imwrite("test12.jpg", mat, compression_params);
		// imwrite(pFileName, mat);
		// printf("%s saved. \n", pFileName);
		Mat xframe;
        resize(mat, xframe, Size(640, 480));
		imencode(".jpg", xframe, data);
		// imencode(".bmp", mat, data);
	}
	catch (runtime_error &ex)
	{
		printf("Error to generate:%s\n", ex.what());
		return 1;
	}
    printf("INDiff4:%d\n", get_ms_time() - lTmpTime1); // INDiff4:2511


	return 0;
}

void get_tv_time(long & pSec, long & pUsec)
{
    struct  timeval    lTv;
	gettimeofday(&lTv, 0);
	pSec = lTv.tv_sec;
	pUsec = lTv.tv_usec;
}

unsigned int get_ms_time(void)
{
	unsigned int lRet = 0;
    struct  timeval    lTv;
	gettimeofday(&lTv, 0);
	lRet = lTv.tv_sec * 1000 + (lTv.tv_usec / 1000);
	return lRet;
}

static char jpg_file_name[64]; 
char * crt_auto_pic(int pWi, int pHi, void * pBuf)
{
	long lSec, lUsec;
	get_tv_time(lSec, lUsec);
    sprintf(jpg_file_name, "pic_%x_%x.jpg", lSec, lUsec);
	crt_pic(jpg_file_name, pWi, pHi, pBuf);
	return jpg_file_name;
}

char * crt_idx_pic(int pIdx, int pWi, int pHi, void * pBuf)
{
    sprintf(jpg_file_name, "No_%d_Part.jpg", pIdx);
	crt_pic(jpg_file_name, pWi, pHi, pBuf);
	return jpg_file_name;
}

char title_msg_buf[256] ;
int  title_msg_delay = 0;

void capture_auto_pic(void) 
{
	void *buffer;  
	void *mapBuf;  
	FILE *lFile;
	unsigned long lCmdPara = 1;

    title_msg("Start To Capture pic ...");
	int lRet;
	int  bufLen = (FRAME_WIDTH * FRAME_HEIGHT * 4);
	int fd = open("/dev/evsoc", O_RDWR);
	ioctl(fd, CMD_SET_CAPTURE, lCmdPara);
	sleep(1);
	buffer = malloc(bufLen);  
	mapBuf = mmap(NULL, bufLen, PROT_READ, MAP_SHARED, fd, 0);
	memcpy(buffer, mapBuf, bufLen);
	munmap(mapBuf, bufLen);
	close(fd);
	char * lFileName = crt_auto_pic(FRAME_WIDTH, FRAME_HEIGHT, buffer);

	title_msg("Done: save as %s!", lFileName);
}

void title_msg(const char* format, ...)
{
    title_msg_buf[0] = 0;
	va_list struAp;
	va_start(struAp, format);
	int lRet = vsnprintf(title_msg_buf, 256, format, struAp); 
	
	int lCol  = 0;
    int lRow  = 0;
    int lType = 0;
	const char * lStr = title_msg_buf;
	unsigned long lFlg = 3;
	int lStrLen = strlen(lStr);
    if (lStrLen == 0)
	{
		return;
	}
    int fd = open("/dev/evsoc", O_RDWR);
    ioctl(fd, CMD_CLR_MSG, lFlg);
    
    for (int lTmpIdx = 0; lTmpIdx < lStrLen; lTmpIdx++)
    {
        unsigned char lChar = lStr[lTmpIdx];
        unsigned long lVal = lChar | ((lType << 8) & 0xff00) | ((lRow << 16) & 0xff0000) | ((lCol << 24) & 0xff000000);
        ioctl(fd, CMD_SET_MSG, lVal);
        lCol++;
    }
    lFlg = 1;
    ioctl(fd, CMD_CLR_MSG, lFlg);
    close(fd);  
	title_msg_delay = 1500;
	va_end(struAp);
}

void poll_title_msg(int pForce)
{
	if (title_msg_delay == 0){
		return;
	}

	if (pForce != 0){
        title_msg_delay = 0;
	}
	else {
        title_msg_delay--;
	}
	if (title_msg_delay == 0){
		unsigned long lFlg = 3;
		int fd = open("/dev/evsoc", O_RDWR);
        ioctl(fd, CMD_CLR_MSG, lFlg);
		close(fd);  
	}
}
