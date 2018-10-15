#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
#define FRAME_WIDTH     640
#define FRAME_HEIGHT    480
#define H_MIN           26
#define H_MAX           46
#define S_MIN           22
#define S_MAX           187
#define V_MIN           217
#define V_MAX           256
#define MAX_NUM_OBJECTS 50
#define MIN_OBJECT_AREA 20*20
#define MAX_OBJECT_AREA FRAME_HEIGHT*FRAME_WIDTH/1.5
#define RECT_SIZE       130
sem_t semCaptureFrameCplt, semProcessFrameCplt;
bool bFoundObject = false, bInit = true;
UMat frame, HSV, thresh, contour;
Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
//dilate with larger element so make sure object is nicely visible
Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
//these two vectors needed for output of findContours
vector< vector<Point> > contours;
vector<Vec4i> hierarchy;
int x,y;

void captureFrame(void);
void processFrame(void);
void trackingObject(void);


int main()
{
    sem_init(&semCaptureFrameCplt, 0, 0);
    sem_init(&semProcessFrameCplt, 0, 0);
    std::thread thread1(captureFrame);
    std::thread thread2(processFrame);
    std::thread thread3(trackingObject);
    thread1.join();
    thread2.join();
    thread3.join();
    return 0;
}
void captureFrame(void)
{
    VideoCapture video(0);
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl; 
    } 
    // Read frame continiously
    video.set(CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	video.set(CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    video.set(CAP_PROP_FPS, 90);
    sleep(1);
    int count = 0;
    auto start = std::chrono::system_clock::now();
    float fps;
    while (1)
    {
        bool ok = video.read(frame); 
        if (count == 0)
            start = std::chrono::system_clock::now();
        count++;
        if (ok)
        {
            std::cout << fps << "\n";       
            sem_post(&semCaptureFrameCplt);
        }
        if (count == 120)
        {
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = end-start;
            fps = 120 / diff.count();
            std::cout << fps << "\n";
            count = 0;
        }
    }
}

void processFrame(void)
{
    while(1)
    {     
        sem_wait(&semCaptureFrameCplt);     
        std::cout << "thread 2\n";
        //process frame to find object;
        //convert frame from BGR to HSV colorspace
        cvtColor(frame,HSV,COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),thresh);
        //imshow("xcvxcv", thresh);
        //waitKey(1);
        // erode(thresh,thresh,erodeElement);
        // erode(thresh,thresh,erodeElement);

        // dilate(thresh,thresh,dilateElement);
        // dilate(thresh,thresh,dilateElement);

        
        //find contours of filtered image using openCV findContours function
        findContours(thresh,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
        
        //use moments method to find our filtered object
	    
        sem_post(&semProcessFrameCplt);
    }
}
void trackingObject(void)
{
   
    Rect2d bbox;
    while(1)
    {
        sem_wait(&semProcessFrameCplt);
        double refArea = 0;
	    bool objectFound = false;
        if (hierarchy.size() > 0) 
        {
            int numObjects = hierarchy.size();
            //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
            if(numObjects<MAX_NUM_OBJECTS)
            {
                for (int index = 0; index >= 0; index = hierarchy[index][0]) 
                {
                    Moments moment = moments((cv::Mat)contours[index]);
                    double area = moment.m00;
                    //if the area is less than 20 px by 20px then it is probably just noise
                    //if the area is the same as the 3/2 of the image size, probably just a bad filter
                    //we only want the object with the largest area so we safe a reference area each
                    //iteration and compare it to the area in the next iteration.
                    if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea)
                    {
                        x = moment.m10/area;
                        y = moment.m01/area;
                        bFoundObject = true;
                        refArea = area;
                    }
                    else 
                        bFoundObject = false;


                }

            }
            else 
            putText(frame,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
        }
        Rect2d bbox(x - RECT_SIZE/2, y - RECT_SIZE/2, RECT_SIZE, RECT_SIZE); 
 
    // Uncomment the line below to select a different bounding box 
    // bbox = selectROI(frame, false); 
    // Display bounding box. 
        rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
        imshow("asdasd", frame);
        waitKey(1);
        std::cout << "thread 3\n";
    }
}

// void thread4(void)
// {
    
// }

// void drawObject(int x, int y,UMat &frame)
// {

// 	//use some of the openCV drawing functions to draw crosshairs
// 	//on your tracked image!

//     //UPDATE:JUNE 18TH, 2013
//     //added 'if' and 'else' statements to prevent
//     //memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

// 	circle(frame,Point(x,y),20,Scalar(0,255,0),2);
//     if(y-25>0)
//     line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
//     else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
//     if(y+25<FRAME_HEIGHT)
//     line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
//     else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
//     if(x-25>0)
//     line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
//     else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
//     if(x+25<FRAME_WIDTH)
//     line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
//     else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);

// 	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);

// }

// string intToString(int number)
// {


// 	std::stringstream ss;
// 	ss << number;
// 	return ss.str();
// }