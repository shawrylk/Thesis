#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "../hpp/bpsUARTData.hpp"
#include "../hpp/bpsServer.hpp"
#include "../hpp/bpsKalmanFilter.hpp"
using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
#define FRAME_WIDTH     480
#define FRAME_HEIGHT    480
#define FPS             90
#define H_MIN           26
#define H_MAX           46
#define S_MIN           22
#define S_MAX           187
#define V_MIN           217
#define V_MAX           256
#define MAX_NUM_OBJECTS 100
#define MIN_OBJECT_AREA 20*20
#define MAX_OBJECT_AREA FRAME_HEIGHT*FRAME_WIDTH/1.5
#define RECT_SIZE       130
#define THRESH_MAX      40
sem_t semCaptureFrameCplt, semProcessFrameCplt, semSendDataCplt, semContourFrameCplt, semTrackingObjectCplt;
bool bFoundObject = false;
Mat frame, gray, thresh, contour;
int8_t thresh_min = 6;
vector< vector<Point> > contours;
vector<Vec4i> hierarchy;
int x,y;

void captureFrame(void);
void processFrame(void);
void showImage(void);
void server(void);
int sendFunc (char *sendData, int sendLen);
int recvFunc (char *recvData, int recvLen);
::KalmanFilter KF(240, 0.01, 1);

int main()
{
    cv::ocl::setUseOpenCL(false);
    sem_init(&semCaptureFrameCplt, 0, 0);
    sem_init(&semProcessFrameCplt, 0, 0);
    sem_init(&semSendDataCplt, 0, 0);
    sem_init(&semContourFrameCplt, 0, 0);
    sem_init(&semTrackingObjectCplt, 0, 0);
    std::thread thread1(captureFrame);
    std::thread thread2(processFrame);
    std::thread thread3(server);
    //std::thread thread4(contourFrame);
    //std::thread thread5(showImage);
    thread1.join();
    thread2.join();
    thread3.join();
    //thread4.join();
    //thread5.join();
    return 0;
}
void captureFrame(void)
{
    VideoCapture video(0);
    if(!video.isOpened())
        std::cout << "Could not read video file" << endl; 
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

void processFrame(void)
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
        cvtColor(frame,gray,COLOR_BGR2GRAY);
        threshold(gray,thresh,16,255,0);
        findContours(thresh,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE );
        double refArea = 0;
	    bool objectFound = false;
        double area;
        int index;
        if (hierarchy.size() > 0) 
        {
            int numObjects = hierarchy.size();
            //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
            if(numObjects<MAX_NUM_OBJECTS)
            {
                for (index = 0; index >= 0; index = hierarchy[index][0]) 
                {
                    Moments moment = moments((cv::Mat)contours[index]);
                    area = moment.m00;
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
            {
                putText(frame,"TOO MUCH NOISE!",Point(0,50),1,2,Scalar(0,0,255),2);
                thresh_min++;
                if (thresh_min == THRESH_MAX)
                    thresh_min = THRESH_MAX;
            }
        }
        sem_post(&semProcessFrameCplt);
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

void showImage(void)
{
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    Rect2d bbox;
    sleep(1);
    while(1)
    {
        sem_wait(&semSendDataCplt);
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        Rect2d bbox(x - RECT_SIZE/2, y - RECT_SIZE/2, RECT_SIZE, RECT_SIZE); 
        rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
        imshow("frame", frame);
        waitKey(1);
        if (count == 1000)
        {
            auto end = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread 5 " << fps << "\n";
            count = 0;
        }
    }
}

void server(void)
{
    Server server((char *)"pi",(char *)"raspberry",4,60);
    std::cout << server.Start(sendFunc, recvFunc) << "\n";
}

int sendFunc (char *sendData, int sendLen)
{
    sem_wait(&semProcessFrameCplt);
    bpsPointTypeDef *data = (bpsPointTypeDef*)sendData;
    data->setpointCoordinate[BPS_X_AXIS] = (int)KF.predict(x);
    data->setpointCoordinate[BPS_Y_AXIS] = (int)KF.predict(y);
    sem_post(&semSendDataCplt);
}

int recvFunc (char *recvData, int recvLen)
{
    bpsUARTReceiveDataTypeDef *data = (bpsUARTReceiveDataTypeDef*)recvData ;
    switch (data->command)
    {
        case  BPS_MODE_CIRCLE:
            std::cout << "circle mode \n";
            std::cout << "x: " << data->content.circleProperties.centerCoordinate[BPS_X_AXIS] << " -- ";
            std::cout << "y: " << data->content.circleProperties.centerCoordinate[BPS_Y_AXIS] << std::endl;
            std::cout << "r: " << data->content.circleProperties.radius << std::endl;
            std::cout << "s: " << data->content.circleProperties.speed << std::endl;
            break;
        case BPS_MODE_SETPOINT:
            std::cout << "setpoint mode \n";
            std::cout << "x: " << data->content.pointProperties.setpointCoordinate[BPS_X_AXIS] << " -- ";
            std::cout << "y: " << data->content.pointProperties.setpointCoordinate[BPS_Y_AXIS] << std::endl;
            break;
        case BPS_MODE_RECTANGLE:
            std::cout << "rectangle mode \n";
            std::cout << "TL x: " << data->content.rectangleProperties.vertexCoordinate[BPS_TOP_LEFT][BPS_X_AXIS] << " -- ";
            std::cout << "TL y: " << data->content.rectangleProperties.vertexCoordinate[BPS_TOP_LEFT][BPS_Y_AXIS] << std::endl;

            std::cout << "TR x: " << data->content.rectangleProperties.vertexCoordinate[BPS_TOP_RIGHT][BPS_X_AXIS] << " -- ";
            std::cout << "TR y: " << data->content.rectangleProperties.vertexCoordinate[BPS_TOP_RIGHT][BPS_Y_AXIS] << std::endl;

            std::cout << "BL x: " << data->content.rectangleProperties.vertexCoordinate[BPS_BOT_LEFT][BPS_X_AXIS] << " -- ";
            std::cout << "BL y: " << data->content.rectangleProperties.vertexCoordinate[BPS_BOT_LEFT][BPS_Y_AXIS] << std::endl;

            std::cout << "BR x: " << data->content.rectangleProperties.vertexCoordinate[BPS_BOT_RIGHT][BPS_X_AXIS] << " -- ";
            std::cout << "BR y: " << data->content.rectangleProperties.vertexCoordinate[BPS_BOT_RIGHT][BPS_Y_AXIS] << std::endl;

            break;
        case BPS_UPDATE_PID:
            std::cout << "update PID \n";

            std::cout << "Kp Inner x: " << data->content.PIDProperties.Kp[BPS_INNER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kp Inner y: " << data->content.PIDProperties.Kp[BPS_INNER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Kp Outer x: " << data->content.PIDProperties.Kp[BPS_OUTER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kp Outer y: " << data->content.PIDProperties.Kp[BPS_OUTER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Ki Inner x: " << data->content.PIDProperties.Ki[BPS_INNER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Ki Inner y: " << data->content.PIDProperties.Ki[BPS_INNER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Ki Outer x: " << data->content.PIDProperties.Ki[BPS_OUTER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Ki Outer y: " << data->content.PIDProperties.Ki[BPS_OUTER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Kd Inner x: " << data->content.PIDProperties.Kd[BPS_INNER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kd Inner y: " << data->content.PIDProperties.Kd[BPS_INNER_PID][BPS_Y_AXIS] << std::endl;
            
            std::cout << "Kd Outer x: " << data->content.PIDProperties.Kd[BPS_OUTER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kd Outer y: " << data->content.PIDProperties.Kd[BPS_OUTER_PID][BPS_Y_AXIS] << std::endl;

            break;
        default:
            break;
    }
    return 0;
}
