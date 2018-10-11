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
static UMat frame;
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
	std::cout << "Running\n";
	first.join();
	second.join();
	return 0;
}

void captureVideoStream()
{
	
	 // open the default camera
	cap.set(CAP_PROP_FRAME_WIDTH, 480);
	cap.set(CAP_PROP_FRAME_HEIGHT, 480);
	cap.set(CV_CAP_PROP_FPS, 120);
	//cap.set(CV_CAP_PROP_CONVERT_RGB , false);
	if (cap.isOpened())
			std::cout << "cant open cam\n";
	while (1)
	{
		cap.read(frame);
		waitKey(1);
	}
}
void processFrame()
{
	UMat edges;
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
		cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);
		imshow("edges", edges);
		waitKey(1);
		
		if (count == 120)
		{
			std::time(&end);
			std::cout << "fps " << 120.0 / difftime(end, start) << std::endl;
			count = 0;
		}
		
	}
}