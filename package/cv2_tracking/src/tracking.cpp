#include <iostream>
#include <fstream>
#include <cstdlib>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Blob.h"

using namespace std;
using namespace cv;

const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);
const Scalar SCALAR_GREEN = Scalar(0.0, 255.0, 0.0);
const Scalar SCALAR_BLUE = Scalar(255.0, 0.0, 0.0);


int main(int argc, char **argv)
{
    int cpu = getNumThreads();
    cpu = (argc > 1) ? cpu = stoi(argv[1]) : cpu;
    std::cout << "use " << to_string(cpu) << " cpus" << std::endl;
    cv::setNumThreads(cpu);

    int count = 0;
    char *inputVideo = "/usr/share/cv2_tracking/sample_video_320p_6fps_5sec.mp4";
    VideoCapture video(inputVideo);
    cout << "Reading video file " << inputVideo << "\n" << endl;
    
    int frameWidth = video.get(CAP_PROP_FRAME_WIDTH);
    int frameHeight = video.get(CAP_PROP_FRAME_HEIGHT);
    int frameCount = video.get(CAP_PROP_FRAME_COUNT);
    
    char *outputVideo = "/root/output_tracking.avi";
    VideoWriter videoOutput(outputVideo, CV_FOURCC('M','J','P','G'), 6.0, Size(frameWidth, frameHeight));

    if (!video.isOpened())
    {
        cout << "Could not read video file" << endl;
        return 1;
    }
    
    cout << "Video resolution: " << frameWidth << "x" << frameHeight << "\n" << endl;
    cout << "Total frame: " << frameCount << "\n" << endl;

    Mat frame1;
    Mat frame2;
    
    video.read(frame1);
    video.read(frame2);
    
    while(video.isOpened()) {
    vector<Blob> blobs;
    Mat frame1Copy = frame1.clone();
    Mat frame2Copy = frame2.clone();
        
    Mat imgDiff;
    Mat imgThresh;
        
    cvtColor(frame1Copy, frame1Copy, CV_BGR2GRAY);
    cvtColor(frame2Copy, frame2Copy, CV_BGR2GRAY);
        
    GaussianBlur(frame1Copy, frame1Copy, Size(5,5), 0);
    GaussianBlur(frame2Copy, frame2Copy, Size(5,5), 0);
        
    absdiff(frame1Copy, frame2Copy, imgDiff);
    threshold(imgDiff, imgThresh, 30, 255.0, CV_THRESH_BINARY);
    //imshow("imgThresh", imgThresh);
    
    Mat structuringElement9x9 = getStructuringElement(MORPH_RECT, Size(9,9));
    
    dilate(imgThresh, imgThresh, structuringElement9x9);
    dilate(imgThresh, imgThresh, structuringElement9x9);
    erode(imgThresh, imgThresh, structuringElement9x9);
        
    Mat imgThreshCopy = imgThresh.clone();
    
    vector<vector<Point>> contours;
    findContours(imgThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    Mat imgContours(imgThresh.size(), CV_8UC3, SCALAR_BLACK);
    drawContours(imgContours, contours, -1, SCALAR_WHITE, -1);
    //imshow("imgContours", imgContours);
    
    vector<vector<Point>> convexHulls(contours.size());
    
    for (unsigned int i = 0; i < contours.size(); i++) {
        convexHull(contours[i], convexHulls[i]);
    }
    
    for (auto &convexHull: convexHulls) {
        Blob possibleBlob(convexHull);
        
        if (possibleBlob.boundingRect.area() > 200 &&
            possibleBlob.dblAspectRatio >= 1.0 &&
            possibleBlob.dblAspectRatio <= 10.0 &&
            possibleBlob.boundingRect.width > 30 &&
            possibleBlob.boundingRect.height > 30 &&
            possibleBlob.dblDiagonalSize > 10.0) {
                blobs.push_back(possibleBlob);
            }
    }
    
    Mat imgConvexHulls(imgThresh.size(), CV_8UC3, SCALAR_BLACK);
    convexHulls.clear();
    
    for (auto &blob: blobs) {
        convexHulls.push_back(blob.contour);
    }
    
    drawContours(imgConvexHulls, convexHulls, -1, SCALAR_WHITE, -1);
    //imshow("imgConvexHulls", imgConvexHulls);
    
    frame2Copy = frame2.clone();
    
    for (auto &blob: blobs) {
        rectangle(frame2Copy, blob.boundingRect, SCALAR_RED, 2); // draw a red box arond the blob
    }
    
    //imshow("frame2Copy", frame2Copy);
    frame1 = frame2.clone();

    if ((video.get(CV_CAP_PROP_POS_FRAMES) + 1) < video.get(CV_CAP_PROP_FRAME_COUNT)) {
        video.read(frame2);
        
        videoOutput.write(frame2Copy);
        cout << "Processing frame " << count << " of " << frameCount << "\n" << endl;
        count++;
        
    } else {
        cout << "end of video\n" << endl;
        break;
    }
    
    }
    
    cout << "Tracking video store in " << outputVideo << "\n" << endl;
    
    video.release();
    videoOutput.release();
    
    return 0;
}
