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
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <fnmatch.h>
#include <pthread.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <future>
#include <thread>
#include <omp.h>
#include "mongoose.h"
#include "cam_ops.h"

#define DEMO_MAGIC              0x68
#define CMD_SET_MODE			_IOW(DEMO_MAGIC, 1, unsigned long)
#define CMD_SET_CAPTURE         _IOW(DEMO_MAGIC, 7, unsigned long)
#define CMD_SET_CAPTURE_FULL    1
#define CMD_SET_CAPTURE_PART    2

#define FRAME_WIDTH		1280
#define FRAME_HEIGHT	720

#define IMAGE_PER_PAGE  4
#define MAX_STRING_LENGTH 256

using namespace cv;
using namespace std;
using namespace std::chrono;

static const char *web_root = "/usr/share/evsoc_camera/web_root";

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

struct img {
	char name[15];
	struct img *next;
};

struct dev {
    struct hw_accel hw_accel;
    struct mg_mgr mgr;
	Mat *frame;
	int sw_grayscale;
	int capture;
	struct img *head;
	pthread_mutex_t lock;
};

void parse_json(struct mg_http_message *hm, char *buf)
{
	struct mg_str *body = &hm->body;
	strncpy(buf, body->ptr, body->len);
	//printf("parse_json: %s, len = %d\n", buf, body->len);
}

void get_json_value(char *json, char *value)
{
	// only support single key value pari like this {"key": "value"}
	int i = 0, len = 0;
	while(json[++i] != '\0');
	len = i;
	i = 0;
	while(json[++i] != ':');
	snprintf(value, len -i -1, "%s", &json[i+1]);
}

int get_json_value_int(char *json)
{
	int val = 0;
	char value[257] = {0};

	get_json_value(json, value);

	// removing double quote
	if (value[0] == '\"') {
		memmove(value, value + 1, strlen(value));
	}

	val = atoi(value);

	return val;
}

int get_list_length(struct img *head)
{
	struct img *t = head;
	int count = 0;

	while (t != NULL) {
		t = t->next;
		count++;
	}

	return count;
}

void print_list(struct img *head)
{
	struct img *img_head = head;

	while (img_head != NULL) {
		printf("%s\n", img_head->name);
		img_head = img_head->next;
	}
}

void append_list(struct img **head, char *filename)
{
	struct img *new_img = (struct img *)malloc(sizeof(struct img));
    if(new_img == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    snprintf(new_img->name, sizeof(new_img->name), "%s", filename);
    new_img->next = *head;
    *head = new_img;
}

void free_list(struct img *head)
{
	struct img *temp = head;

	while(head != NULL) {
		temp = head;
		head = temp->next;
		free(temp);
	}
}

void get_list(struct img **head)
{
	struct dirent *dir;
	DIR *d;
	char img_dir[1023];

	snprintf(img_dir, sizeof(img_dir), "%s/images", web_root);

	d = opendir(img_dir);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (fnmatch("*.jpg", dir->d_name, 0) == 0) {
				 append_list(head, dir->d_name);
			}
		}
		closedir(d);
	}
}

int get_total_pages(struct img *head)
{
    int total_pages = 1;
    int length = 0;

    length = get_list_length(head);

	if (length > 0) {
		total_pages = length / IMAGE_PER_PAGE;
		if ((length % IMAGE_PER_PAGE) > 0)
			total_pages++;
	}

    return total_pages;
}

void create_json_string(char items[IMAGE_PER_PAGE][MAX_STRING_LENGTH], char *respond, int respond_size, int total_pages, int num_images)
{
    int i;
	char item_str[256] = {0};
    char temp[256] = {0};

	for (i = 0; i < num_images; i++) {
		snprintf(temp, sizeof(temp), "\"%s\"", items[i]);
		strncat(item_str, temp, sizeof(item_str) - strlen(item_str) - 1);

		if (i < (num_images - 1)) {
			strncat(item_str, ", ", sizeof(item_str) - strlen(item_str) - 1);
		}
	}

	snprintf(respond, respond_size, "{\"capture\": [%s], \"page\": %d}", item_str, total_pages);
}

void update_page(struct mg_connection *c, struct dev *dev)
{
    char items[IMAGE_PER_PAGE][MAX_STRING_LENGTH];
    char respond[256];
    struct img *head = dev->head;
    int total_pages = 0;
	int i = 0;

    total_pages = get_total_pages(dev->head);

    while(head != NULL) {
		strncpy(items[i++], head->name, strlen(head->name) + 1);
		head = head->next;
		if (i == IMAGE_PER_PAGE) break;
	}

	create_json_string(items, respond, sizeof(respond), total_pages, i);
    mg_http_reply(c, 200, "", "%s\n", respond);
}

void get_page(struct mg_connection *c, struct dev *dev, int page)
{
	struct img *head = dev->head;
	int img_per_page = 4;
	char items[img_per_page][MAX_STRING_LENGTH] = {0};
	int total_pages = 0;
	int i = 0;
	int j = 0;
	char respond[256] = {0};

	total_pages = get_total_pages(head);

	while (head != NULL) {
		if ((i >= ((page * img_per_page) - img_per_page)) && (i < (page * img_per_page))) {
			strncpy(items[j], head->name, strlen(head->name) + 1);
			j++;
		}

		head = head->next;
		i++;
	}

	create_json_string(items, respond, sizeof(respond), total_pages, j);
    mg_http_reply(c, 200, "", "%s\n", respond);
}

void on_load(struct mg_connection *c, struct mg_http_message *hm, struct dev *dev)
{
	int total_pages = 0;
	char buff[1024] = {0};
	int current_page = 1;

	parse_json(hm, buff);
	current_page = get_json_value_int(buff);

	if (dev->head == NULL)
		get_list(&dev->head);

	total_pages = get_total_pages(dev->head);

	if (current_page > total_pages)
		current_page = 1;

	get_page(c, dev, current_page);
}

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
	char buff[256] = {0};
	int num = 0;

	pthread_mutex_lock(&dev->lock);
	if (ev == MG_EV_HTTP_MSG) {
		if (mg_http_match_uri(hm, "/api/camera")) {
			c->label[0] = 'S';
			mg_printf(
				c, "%s",
				"HTTP/1.0 200 OK\r\n"
				"Cache-Control: no-cache\r\n"
				"Pragma: no-cache\r\nExpires: Thu, 01 Dec 2023 00:00:00 GMT\\r\n"
				"Content-Type: multipart/x-mixed-replace; boundary=--foo\r\n\r\n"
			);
		} else if (mg_vcmp(&hm->uri, "/load") == 0) {
			on_load(c, hm, dev);

		} else if (mg_vcmp(&hm->uri, "/btn_capture") == 0) {
			parse_json(hm, buff);
			num = get_json_value_int(buff);
			if (num == 1) {
				dev->capture = 1;
			}

		} else if (mg_vcmp(&hm->uri, "/btn_normal") == 0) {
			parse_json(hm, buff);
			num = get_json_value_int(buff);
			if (num == 1) {
				dev->hw_accel.cmd = HW_ACCEL_NONE;
				dev->sw_grayscale = 0;
			}

		} else if (mg_vcmp(&hm->uri, "/btn_sw_grayscale") == 0) {
			parse_json(hm, buff);
			num = get_json_value_int(buff);
			if (num == 1) {
				dev->sw_grayscale = 1;
			}

		} else if (mg_vcmp(&hm->uri, "/btn_erosion") == 0) {
			parse_json(hm, buff);
			num = get_json_value_int(buff);
			if (num == 1) {
				dev->hw_accel.cmd = HW_ACCEL_EROSION;
				dev->sw_grayscale = 0;
			}

		} else if (mg_vcmp(&hm->method, "POST") == 0) {

			if (mg_vcmp(&hm->uri, "/digital_gain") == 0) {
				parse_json(hm, buff);
				num = get_json_value_int(buff);
				printf("digital gain = %d\n", num);
				camera_set_digital_gain(num);

            } else if (mg_vcmp(&hm->uri, "/exposure") == 0) {
				parse_json(hm, buff);
				num = get_json_value_int(buff);
				printf("exposure = %d\n", num);
				camera_set_exposure(num);

			} else if (mg_vcmp(&hm->uri, "/btn_next") == 0) {
				parse_json(hm, buff);
				num = get_json_value_int(buff);
				printf("btn_next = %d\n", num);
				get_page(c, dev, num);

			} else if (mg_vcmp(&hm->uri, "/btn_prev") == 0) {
				parse_json(hm, buff);
				num = get_json_value_int(buff);
				printf("btn_prev = %d\n", num);
				get_page(c, dev, num);

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
				.root_dir = web_root
			};
			mg_http_serve_dir(c, (struct mg_http_message *)ev_data, &opts);
		}
	}

	pthread_mutex_unlock(&dev->lock);
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

static void capture_photo(struct dev *dev, Mat *frame)
{
	struct mg_mgr *mgr = &dev->mgr;
	struct mg_connection *c;
	struct stat st = {0};
	c = mgr->conns;
	char filename[15];
	char image_path[1024];
	char full_path[2048];
	int ret;
	int num = 0;
	struct img *img = dev->head;

	snprintf(image_path, sizeof(image_path), "%s/images", web_root);
	if (stat(image_path, &st) == -1) {
		ret = mkdir(image_path, 0744);
		if (ret) {
			printf("Failed to create a image path directory at %s\n", image_path);
			return;
		}
	}

	// get the latest filename
	if (dev->head != NULL) {
		snprintf(filename, 5, "%s", &img->name[4]);
		num = atoi(filename);

		if (num >= 9999)
			num = 0;
		else
			num++;
	}

	snprintf(filename, sizeof(filename), "img_%04d.jpg", num);
	snprintf(full_path, sizeof(full_path), "%s/%s", image_path, filename);
	printf("saving image as %s\n", full_path);

	append_list(&dev->head, filename);
	imwrite(full_path, *frame);
	update_page(c, dev);
}

Mat* capture_frame(struct dev *dev) {
    pthread_mutex_lock(&dev->lock);
    Mat *frame = video_capture_stream(&dev->hw_accel);
    pthread_mutex_unlock(&dev->lock);
    return frame;
}

void process_frame(struct dev *dev, Mat *frame) {
    pthread_mutex_lock(&dev->lock);
    if (dev->sw_grayscale == 1) {
        cv::cvtColor(*frame, *frame, COLOR_BGR2GRAY);
        canny_threshold(frame);
    }
    pthread_mutex_unlock(&dev->lock);
}

vector<uchar> encode_frame(Mat *frame) {
    vector<uchar> encoded_frame;
    vector<int> params = {CV_IMWRITE_JPEG_QUALITY, 95};
    imencode(".jpg", *frame, encoded_frame, params);
    return encoded_frame;
}

void send_encoded_frame(struct dev *dev, vector<uchar> &encoded_frame) {
    pthread_mutex_lock(&dev->lock);
    string video_data(encoded_frame.begin(), encoded_frame.end());
    send_frame(dev, NULL, video_data);
    pthread_mutex_unlock(&dev->lock);
}

void check_and_capture_photo(struct dev *dev, Mat *frame) {
    pthread_mutex_lock(&dev->lock);
    if (dev->capture == 1) {
        capture_photo(dev, frame);
        dev->capture = 0;
    }
    pthread_mutex_unlock(&dev->lock);
}

void broadcast_mjpeg_camera_frame(struct dev *dev) {
    Mat *frame = capture_frame(dev);
    process_frame(dev, frame);
    vector<uchar> encoded_frame = encode_frame(frame);
    send_encoded_frame(dev, encoded_frame);
    check_and_capture_photo(dev, frame);
    delete frame;
    encoded_frame.clear();
}

static void timer_callback(void *arg)
{
	struct dev *dev = (struct dev *)arg;

	broadcast_mjpeg_camera_frame(dev);
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

void *listen_thread(void *arg)
{
	struct dev *dev = (struct dev *)arg;

	printf("listen thread started\n");
	mg_http_listen(&dev->mgr, "0.0.0.0:8000", handle_request, dev);

	return NULL;
}

void *stream_thread(void *arg)
{
	struct dev *dev = (struct dev *)arg;
	struct mg_timer t;

	printf("stream thread started\n");
    mg_timer_init(&t, 10, MG_TIMER_REPEAT, timer_callback, dev);

	return NULL;
}

int main(void)
{
    struct dev dev;
	dev.hw_accel.enable = true;
	dev.hw_accel.cmd = HW_ACCEL_NONE;
	char *ip_address = get_ip_address();
	struct img *head = NULL;
	dev.head = head;
	pthread_t t1, t2;

    printf("mongoose version %s\n", MG_VERSION);
    mg_mgr_init(&dev.mgr);

	pthread_create(&t1, NULL, &listen_thread, &dev);
	pthread_create(&t2, NULL, &stream_thread, &dev);

	printf("Go to http://%s:8000 for live camera stream\n", ip_address);

	signal(SIGINT, inthand);
    printf("Press CTRL + C to stop the streaming\n");

	while (!stop) {
		mg_mgr_poll(&dev.mgr, 1);
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

    mg_mgr_free(&dev.mgr);
	free_list(dev.head);
	printf("Stream server shutdown\n");

    return 0;
}
