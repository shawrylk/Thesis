//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <ctime>
#include <unistd.h>
using namespace std;
using namespace cv;

void captureVideoStream(void);
void processFrame(void);
void Blur(void);
void Blob(void);
static UMat frame, color, mblur;
static VideoCapture cap(0);
int main( int, char** ) 
{

	// static int fps;
	// VideoCapture cap(0); // open the default camera
	// cap.set(CAP_PROP_FRAME_WIDTH, 320);
	// cap.set(CAP_PROP_FRAME_HEIGHT, 320);
	// cap.set(CV_CAP_PROP_FPS, 60);
	// if (!cap.isOpened())  // check if we succeeded
	// 	return -1;
	// UMat edges;
	// UMat frame;
	// namedWindow("edges", 1);
	// std::time_t start = std::time(0);
	// for (int i = 0; i <120; i++)
	// {
	// 	cap >> frame; // get a new frame from camera
	// 	fps++;
	// 	cvtColor(frame, edges, COLOR_BGR2GRAY);
	// 	GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
	// 	Canny(edges, edges, 0, 30, 3);
	// 	imshow("edges", edges);
	// 	waitKey(1);
		
	// }
	// std::time_t end = std::time(0);
	// std::cout << "fps " << 120/(end - start) << std::endl;
	// waitKey(5);
	std::thread first(captureVideoStream);
	std::thread second(processFrame);
	std::thread third(Blob);
	std::cout << "Running\n";
	first.join();
	second.join();
	third.join();
	return 0;
}

void captureVideoStream()
{
	int count = 0;
	std::time_t start, end;
	 // open the default camera
	cap.set(CAP_PROP_FRAME_WIDTH, 480);
	cap.set(CAP_PROP_FRAME_HEIGHT, 480);
	cap.set(CV_CAP_PROP_FPS, 180);
	//cap.set(CV_CAP_PROP_CONVERT_RGB , false);
	if (cap.isOpened())
			std::cout << "cant open cam\n";
	while (1)
	{
		cap.read(frame);
		//waitKey(1);
		if (count == 0)
			std::time(&start);
		count++;
		if (count == 1200)
		{
			std::time(&end);
			std::cout << "read frame " << 1200.0 / difftime(end, start) << std::endl;
			count = 0;
		}
	}
}
void processFrame()
{
	
	sleep(1);
	int count = 0;
	std::time_t start, end;
	while(1)
	{
		if (frame.empty())
			continue;
		if (count == 0)
			std::time(&start);
		count++;
		cvtColor(frame, color, COLOR_BGR2GRAY);
		//GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		//Canny(edges, edges, 0, 30, 3);
		//imshow("edges", color);
		//waitKey(1);
		
		if (count == 1200)
		{
			std::time(&end);
			std::cout << "color " << 1200.0 / difftime(end, start) << std::endl;
			count = 0;
		}
		
	}
}

void Blur()
{
    sleep(1);
	int count = 0;
	std::time_t start, end;
	while(1)
	{
		if (color.empty())
			continue;
		if (count == 0)
			std::time(&start);
		count++;
		//cvtColor(color, blur, COLOR_BGR2GRAY);
		GaussianBlur(color, mblur, Size(7, 7), 1.5, 1.5);
		//Canny(edges, edges, 0, 30, 3);
		imshow("edges", mblur);
		waitKey(1);
		
		if (count == 1200)
		{
			std::time(&end);
			std::cout << "blur" << 1200.0 / difftime(end, start) << std::endl;
			count = 0;
		}
		
	}
}

void Blob()
{
	while (1)
	{
	// Read image
	if (color.empty())
			continue;
	// Setup SimpleBlobDetector parameters.
	SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 50;
	params.maxThreshold = 200;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = 1500;

	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.1;

	// Filter by Convexity
	params.filterByConvexity = true;
	params.minConvexity = 0.87;

	// Filter by Inertia
	params.filterByInertia = true;
	params.minInertiaRatio = 0.01;


	// Storage for blobs
	vector<KeyPoint> keypoints;


#if CV_MAJOR_VERSION < 3   // If you are using OpenCV 2

	// Set up detector with params
	SimpleBlobDetector detector(params);

	// Detect blobs
	detector.detect( im, keypoints);
#else 

	// Set up detector with params
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);   

	// Detect blobs
	detector->detect( color, keypoints);
#endif 

	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
	// the size of the circle corresponds to the size of blob

	UMat im_with_keypoints;
	drawKeypoints( color, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

	// Show blobs
	imshow("keypoints", im_with_keypoints );
	waitKey(1);
	}

}