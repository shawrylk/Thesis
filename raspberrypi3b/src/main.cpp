#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
//#include "../hpp/bpsUARTData.hpp"

using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
#define FRAME_WIDTH     640
#define FRAME_HEIGHT    480
#define FPS             90
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

sem_t semCaptureFrameCplt, semCvtColorFrameCplt, semThreshFrameCplt, semContourFrameCplt, semTrackingObjectCplt;
bool bFoundObject = false;
Mat frame, HSV, thresh, contour;
//Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
//dilate with larger element so make sure object is nicely visible
//Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
//these two vectors needed for output of findContours
vector< vector<Point> > contours;
vector<Vec4i> hierarchy;
int x,y;

void captureFrame(void);
void cvtColorFrame(void);
void threshFrame(void);
void contourFrame(void);
//void trackingObject(void);
void sendUARTData(void);

int main()
{
    cv::ocl::setUseOpenCL(false);
    sem_init(&semCaptureFrameCplt, 0, 0);
    sem_init(&semCvtColorFrameCplt, 0, 0);
    sem_init(&semThreshFrameCplt, 0, 0);
    sem_init(&semContourFrameCplt, 0, 0);
    sem_init(&semTrackingObjectCplt, 0, 0);
    std::thread thread1(captureFrame);
    std::thread thread2(cvtColorFrame);
    std::thread thread3(threshFrame);
    std::thread thread4(contourFrame);
    //std::thread thread5(trackingObject);
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    //thread5.join();
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
    video.set(CAP_PROP_FPS, FPS);
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    sleep(1);
    while (1)
    {
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        bool ok = video.read(frame); 
        count++;
        if (ok)
        {    
            sem_post(&semCaptureFrameCplt);
        }
        if (count == 1000)
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread 1 " << fps << "\n";
            count = 0;
        }
    }
}

void cvtColorFrame(void)
{
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    sleep(1);
    while(1)
    {     
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        sem_wait(&semCaptureFrameCplt);     
        cvtColor(frame,HSV,COLOR_BGR2HSV);
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),thresh);
        sem_post(&semCvtColorFrameCplt);
        if (count == 1000)
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread 2 " << fps << "\n";
            count = 0;
        }
    }
}
void threshFrame(void)
{
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    Rect2d bbox;
    sleep(1);
    while(1)
    {
        sem_wait(&semCvtColorFrameCplt);
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        //inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),thresh);
        sem_post(&semThreshFrameCplt);
        if (count == 1000)
        {
            auto end = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread 3 " << fps << "\n";
            count = 0;
        }
    }
}

void contourFrame(void)
{
//     bpsUARTSendDataTypeDef sendData;
//     sendData.command = BPS_UPDATE_PID;
//     sendData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_X_AXIS] = 0;
//     sendData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_X_AXIS] = 0;
//     sendData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_X_AXIS] = 0;
//     sendData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_Y_AXIS] = 0;
//     sendData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_Y_AXIS] = 0;
//     sendData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_Y_AXIS] = 0;
//     bpsUARTSendData(&sendData);
//     sleep(1);
//     sendData.command = BPS_MODE_SETPOINT;
//     sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 255;
//     sendData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 255;
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    while(1)
    {
        sem_wait(&semThreshFrameCplt);
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        findContours(thresh,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE );
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

            }
            else 
            putText(frame,"TOO MUCH NOISE!",Point(0,50),1,2,Scalar(0,0,255),2);
        }
        sem_post(&semContourFrameCplt);
        if (count == 1000)
        {
            auto end = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread 4 " << fps << "\n";
            count = 0;
        }
    }
    
}

// void trackingObject(void)
// {
//     int count = 0;
//     auto start = std::chrono::high_resolution_clock::now();
//     float fps;
//     Rect2d bbox;
//     sleep(1);
//     while(1)
//     {
//         sem_wait(&semContourFrameCplt);
//         if (count == 0)
//             start = std::chrono::high_resolution_clock::now();
//         count++;
//         //use moments method to find our filtered object
//         double refArea = 0;
// 	    bool objectFound = false;
//         if (hierarchy.size() > 0) 
//         {
//             int numObjects = hierarchy.size();
//             //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
//             if(numObjects<MAX_NUM_OBJECTS)
//             {
//                 for (int index = 0; index >= 0; index = hierarchy[index][0]) 
//                 {
//                     Moments moment = moments((cv::Mat)contours[index]);
//                     double area = moment.m00;
//                     //if the area is less than 20 px by 20px then it is probably just noise
//                     //if the area is the same as the 3/2 of the image size, probably just a bad filter
//                     //we only want the object with the largest area so we safe a reference area each
//                     //iteration and compare it to the area in the next iteration.
//                     if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea)
//                     {
//                         x = moment.m10/area;
//                         y = moment.m01/area;
//                         bFoundObject = true;
//                         refArea = area;
//                     }
//                     else 
//                         bFoundObject = false;


//                 }

//             }
//             else 
//             putText(frame,"TOO MUCH NOISE!",Point(0,50),1,2,Scalar(0,0,255),2);
//         }
//         // Rect2d bbox(x - RECT_SIZE/2, y - RECT_SIZE/2, RECT_SIZE, RECT_SIZE); 
//         // rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
//         // imshow("frame", frame);
//         // waitKey(1);
//         // sem_post(&semTrackingObjectCplt);
//         if (count == 1000)
//         {
//             auto end = std::chrono::system_clock::now();
//             auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
//             fps = 1000 / static_cast<double>(diff.count());
//             std::cout << "thread 5 " << fps << "\n";
//             count = 0;
//         }
//     }
// }

