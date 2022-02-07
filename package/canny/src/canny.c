/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

Mat src, src_gray;
Mat dst, detected_edges;

int low_threshold = 0;
const int max_low_threshold = 100;
const int ratio = 3;
const int kernel_size = 3;

static bool save_image(const char *image_path, Mat *image)
{
	bool isSuccess;
	isSuccess = imwrite(image_path, *image);
	if (isSuccess == false)
        {
                std::cout << "Failed to save output image\n" << std::endl;
                return 0;
        }

        std::cout << "Save canny image as " << image_path << "\n" << std::endl;
	return 1;
}

static void canny_threshold(int, void *)
{
	blur(src_gray, detected_edges, cvSize(3,3));
	Canny(detected_edges, detected_edges, low_threshold, low_threshold * ratio, kernel_size);
	dst = Scalar::all(0);
	src.copyTo(dst, detected_edges);

	save_image("/root/output_canny.jpg", &dst);
	
}

int main()
{
	const char *image_path = "/usr/share/canny/image.jpg";

	std::cout << "Opening file " << image_path << "\n" << std::endl;
	src = imread(image_path);

	if (src.empty())
	{
		std::cout << "Could not open or find the " << image_path << "\n" << std::endl;
		return -1;
	}

	dst.create(src.size(), src.type());
	std::cout << "Convert image to gray scale\n" << std::endl;
	cvtColor(src, src_gray, CV_BGR2GRAY);
        save_image("/root/output_gray.jpg", &src_gray);
	std::cout << "Convert RGB image to canny\n" << std::endl;
	canny_threshold(0, 0);

	return 0;
}
