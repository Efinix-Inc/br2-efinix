#include <iostream>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <future>
#include <thread>
#include <omp.h>
#include "mongoose.h"

#define DEMO_MAGIC              0x68
#define CMD_SET_MODE		_IOW(DEMO_MAGIC, 1, unsigned long)
#define CMD_SET_CAPTURE         _IOW(DEMO_MAGIC, 7, unsigned long)
#define CMD_SET_CAPTURE_FULL    1
#define CMD_SET_CAPTURE_PART    2

#define FRAME_WIDTH	1280
#define FRAME_HEIGHT	720

using namespace cv;
using namespace std;
using namespace std::chrono;

static int COUNTER = 0;

// using mongoose v7.2

#include <atomic>

/* improve version using c++ */
std::atomic<bool> stop(false);

void inthand(int signum)
{
    stop.store(true, std::memory_order_relaxed);
}

enum hw_accel_cmd {
	HW_ACCEL_NONE,
	HW_ACCEL_EROSION,
	HW_ACCEL_DIALATION,
	HW_ACCEL_SOBEL
};

struct hw_accel {
	enum hw_accel_cmd cmd;
	bool enable;
};

struct dev {
        struct hw_accel hw_accel;
        struct mg_mgr mgr;
	Mat *frame;
	int sw_grayscale;
	int capture;
};

bool fill_mat(Mat &mat, void *buffer)
{
    int *buf = static_cast<int*>(buffer);

    #pragma omp parallel for
    for (int i = 0; i < mat.rows; ++i)
    {
        for (int j = 0; j < mat.cols; ++j)
        {
            int pos = i * mat.cols + j;
            Vec3b &rgba = mat.at<Vec3b>(i, j);
            rgba[2] = saturate_cast<uchar>(buf[pos] & 0xff);
            rgba[1] = saturate_cast<uchar>((buf[pos] >> 8) & 0xff);
            rgba[0] = saturate_cast<uchar>((buf[pos] >> 16) & 0xff);
        }
    }

    return true;
}

Mat *video_capture_stream(struct hw_accel *hw_accel)
{
        void *map_buffer;
        int buffer_len = (FRAME_WIDTH * FRAME_HEIGHT * 4);
        unsigned long cmd = CMD_SET_CAPTURE_PART;
        int fd;

	// enable hw accelerator
        if (hw_accel->enable == true) {
                fd = open("/dev/evsoc", O_RDWR);
                ioctl(fd, CMD_SET_MODE, hw_accel->cmd);
                close(fd);
        }

        Mat *cap_mat_p = new Mat(FRAME_HEIGHT / 2, FRAME_WIDTH / 2, CV_8UC3);
        fd = open("/dev/evsoc", O_RDWR);
        map_buffer = mmap(NULL, buffer_len, PROT_READ, MAP_SHARED, fd, 0);
        fill_mat(*cap_mat_p, map_buffer);

        munmap(map_buffer, buffer_len);
        ioctl(fd, CMD_SET_CAPTURE, cmd);
        close(fd);

	return cap_mat_p;
}

// HTTP request handler function. It implements the following endpoints:
//   /api/camera - hangs forever, returns MJPEG video stream
//   all other URI - serves web_root/ directory
static void handle_request(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
	struct mg_http_message *hm = (struct mg_http_message *) ev_data;
	struct dev *dev = (struct dev *) fn_data;

	if (ev == MG_EV_HTTP_MSG) {

		if (mg_http_match_uri(hm, "/api/camera")) {
			c->label[0] = 'S';
			mg_printf(
				c, "%s",
				"HTTP/1.0 200 OK\r\n"
				"Cache-Control: no-cache\r\n"
				"Pragma: no-cache\r\nExpires: Thu, 01 Dec 1994 16:00:00 GMT\\r\n"
				"Content-Type: multipart/x-mixed-replace; boundary=--foo\r\n\r\n"
			);
		} else if (mg_vcmp(&hm->method, "POST") == 0) {
			if (mg_vcmp(&hm->uri, "/btn_erosion") == 0) {
                                dev->hw_accel.cmd = HW_ACCEL_EROSION;
				dev->sw_grayscale = 0;

			} else if (mg_vcmp(&hm->uri, "/btn_reset") == 0) {
                                dev->hw_accel.cmd = HW_ACCEL_NONE;
				dev->sw_grayscale = 0;

			} else if (mg_vcmp(&hm->uri, "/btn_sw_grayscale") == 0) {
				dev->sw_grayscale = 1;

			} else if (mg_vcmp(&hm->uri, "/btn_capture") == 0) {
				dev->capture = 1;

                        } else {
				mg_printf(
                                c, "%s",
                                "HTTP/1.1 200 OK\r\n"
                                "Transfer-Encoding: chunked\r\n\r\n"
                                );
                                mg_http_write_chunk(c, "Hello, client!", 15);
                                mg_http_write_chunk(c, "", 0); // send empty chunk, the end of response
			}

		} else {
			struct mg_http_serve_opts opts = {
				.root_dir = "/usr/share/evsoc_camera/web_root"
			};
			mg_http_serve_dir(c, (struct mg_http_message *)ev_data, &opts);
		}
	}
}

static void send_frame(struct dev *dev, struct mg_connection *c, string video_data)
{
	struct mg_str data;
	struct mg_mgr *mgr = &dev->mgr;

	data.ptr = video_data.c_str();
        data.len = video_data.size();

        for (c = mgr->conns; c != NULL; c = c->next) {
                if (c->label[0] != 'S') continue;
                if (data.ptr == NULL) continue;

                mg_printf(
                        c,
                        "--foo\r\nContent-Type: image/jpeg\r\n"
                        "Content-Length: %lu\r\n\r\n",
                        data.len
                );

                mg_send(c, data.ptr, data.len);
                mg_send(c, "\r\n", 2);
        }
}

static void canny_threshold(Mat *frame)
{
	int low_threshold = 0;
	int ratio = 3;
	int kernel_size = 5;
	Mat detected_edges;

	blur(*frame, *frame, cvSize(3,3));
	Canny(*frame, *frame,
		low_threshold, low_threshold * ratio, kernel_size);
}

static void capture_photo(Mat *frame)
{
	char filename[30];

	snprintf(filename, sizeof(filename), "img_%d.jpg", COUNTER++);
	imwrite(filename, *frame);

	printf("saving image as %s\n", filename);
}

static void broadcast_mjpeg_camera_frame(struct dev *dev)
{
	Mat *frame;
	vector<uchar> encoded_frame;
	struct mg_connection *c;
	vector<int> params = {CV_IMWRITE_JPEG_QUALITY, 95};

	frame = video_capture_stream(&dev->hw_accel);

	if (dev->sw_grayscale == 1) {
		cv::cvtColor(*frame, *frame, COLOR_BGR2GRAY);
		canny_threshold(frame);
	}

	imencode(".jpg", *frame, encoded_frame, params);
	string video_data(encoded_frame.begin(), encoded_frame.end());

	send_frame(dev, c, video_data);

	if (dev->capture == 1) {
		capture_photo(frame);
		dev->capture = 0;
	}

	delete frame;
        encoded_frame.clear();
}

static void timer_callback(void *arg)
{
	broadcast_mjpeg_camera_frame((struct dev *)arg);
}

char *get_ip_address()
{
	int sock, i;
	struct ifreq ifreqs[20];
	struct ifconf ic;
	char *iface;
	char *ip = NULL;

	ic.ifc_len = sizeof(ifreqs);
	ic.ifc_req = ifreqs;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	if (ioctl(sock, SIOCGIFCONF, &ic) < 0) {
		perror("SIOCGIFCONF");
		exit(1);
	}

	for (i = 0; i < (int)(ic.ifc_len/sizeof(struct ifreq)); i++) {
		iface = ifreqs[i].ifr_name;
		ip = inet_ntoa(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr);
		if (strcmp("lo", iface) == 0)
			continue;
	}

	return  ip;
}

int main(void)
{
        struct dev dev;
	dev.hw_accel.enable = true;
	dev.hw_accel.cmd = HW_ACCEL_NONE;
	char *ip_address = get_ip_address();

        printf("mongoose version %s\n", MG_VERSION);
        mg_mgr_init(&dev.mgr);
        mg_http_listen(&dev.mgr, "0.0.0.0:8000", handle_request, &dev);

        struct mg_timer t;
        mg_timer_init(&t, 10, MG_TIMER_REPEAT, timer_callback, &dev);
	printf("Go to http://%s:8000 for live camera stream\n", ip_address);

	signal(SIGINT, inthand);
        printf("Press CTRL + C to stop the streaming\n");

	while (!stop) {
		mg_mgr_poll(&dev.mgr, 1);
	}

        mg_mgr_free(&dev.mgr);
	printf("Stream server shutdown\n");

        return 0;
}
