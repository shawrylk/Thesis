#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
#define FRAME_WIDTH     480
#define FRAME_HEIGHT    480
#define H_MIN           22
#define H_MAX           42
#define S_MIN           83
#define S_MAX           176
#define V_MIN           56
#define V_MAX           164
#define MAX_NUM_OBJECTS 50
#define MIN_OBJECT_AREA 20*20
#define MAX_OBJECT_AREA FRAME_HEIGHT*FRAME_WIDTH/1.5

sem_t semCaptureFrameCplt, semProcessFrameCplt;
bool bFoundObject = false, bInit = true;
UMat frame, HSV, thresh, contour;
Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
//dilate with larger element so make sure object is nicely visible
Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
int x,y;
void captureFrame(void);
void processFrame(void);
void trackingObject(void);
void drawObject(int x, int y,UMat &frame);
string intToString(int number);

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
    sleep(1);
    while (1)
    {
        bool ok = video.read(frame); 
        if (ok)
        {
            std::cout << "thread 1\n";       
            if (bFoundObject)
            {
                sem_post(&semProcessFrameCplt);
            }
            else
            {
                sem_post(&semCaptureFrameCplt);
            }
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
        erode(thresh,thresh,erodeElement);
        erode(thresh,thresh,erodeElement);

        dilate(thresh,thresh,dilateElement);
        dilate(thresh,thresh,dilateElement);

        //these two vectors needed for output of findContours
        vector< vector<Point> > contours;
        vector<Vec4i> hierarchy;
        //find contours of filtered image using openCV findContours function
        findContours(thresh,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
        
        //use moments method to find our filtered object
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
                //let user know you found an object
                // if(bFoundObject ==true)
                // {
                //     putText(frame,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
                //     //draw object location on screen
                //     drawObject(x,y,frame);
                // }

            }
            else 
            putText(frame,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
        }
        sem_post(&semProcessFrameCplt);
    }
}
void trackingObject(void)
{
    Ptr<Tracker> tracker;
    tracker = TrackerMOSSE::create();
    Rect2d bbox;
    while(1)
    {
        sem_wait(&semProcessFrameCplt);
        
        waitKey(1);
        if (bFoundObject)
        {
            // tracking object
            if (bInit)
            {
                bbox = Rect2d(x - 90, y - 90, 180, 180);
                rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
                tracker->init(frame, bbox);
                bInit = false;
            }
            bool ok = tracker->update(frame, bbox);
            if (ok)
            {
                rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
                putText(frame, "Tracking object", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,255,0),2);
            }
            else
            {
                // Tracking failure detected.
                putText(frame, "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
                bFoundObject = false;
                bInit = true;
            }

        }
        imshow("asdasd", frame);
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