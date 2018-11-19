
#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "bpsUART.hpp"
#include "bpsServer.hpp"
#include "bpsKalmanFilter.hpp"
//*******************************//
using namespace cv;
using namespace std;
 //*******************************//
#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()
#define FRAME_WIDTH     480
#define FRAME_HEIGHT    480
#define FPS             90
#define MAX_NUM_OBJECTS 10
#define MIN_OBJECT_AREA 80*80
#define MAX_OBJECT_AREA 120*120
#define RECT_SIZE       2
//*******************************//
Mat frame, gray, mblur, thresh, contour;
int B, C, S, G, T;
//*******************************//
bpsUARTSendDataTypeDef STMData;
bpsUARTReceiveDataTypeDef RaspiEncoderCnt;
bpsSocketSendDataTypeDef AppData;
//*******************************//
std::mutex STMMutex;
std::mutex AppMutex;
sem_t semCaptureFrameCplt, semProcessFrameCplt;
//*******************************//
void captureFrame(void);
void processFrame(void);
void showImage(void);
int recvFunc (char *recvData, int recvLen);
void onTrackbarChanged(int, void*);
//*******************************//
::KalmanFilter KF(240, 0.01, 1);
bpsServer server(22396);
bpsUART UART("/dev/serial0",1000000);
VideoCapture video(0);
//*******************************//
int main( int argc, char *argv[] )
{
    //*******************************//
    bool showFrame = false;
    if( argc == 2 ) {
        if (strcmp(argv[1], "showFrame") == 0)
        {
            printf("Run show frame thread\n");
            showFrame = true;
        }
   }
    //*******************************//
    STMData.command = BPS_MODE_SETPOINT;
    STMData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 240;
    STMData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 240;
    sem_init(&semCaptureFrameCplt, 0, 0);
    sem_init(&semProcessFrameCplt, 0, 0);
    //*******************************//
    std::thread thread1(captureFrame);
    std::thread thread2(processFrame);
    if (showFrame)
        std::thread thread3(showImage);
    //*******************************//
    server.attach(recvFunc);
    server.poll();
    //*******************************//
    thread1.join();
    thread2.join();
    if (showFrame)
        thread3.join();
    return 0;
}
//*******************************//
void captureFrame(void)
{
    //*******************************//
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    //*******************************//
    if(!video.isOpened())
        std::cout << "Could not read video file" << endl; 
    video.set(CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	video.set(CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    video.set(CAP_PROP_FPS, FPS);
    //*******************************//
    sleep(1);
    while (1)
    {
        //*******************************//
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        //*******************************//
        video.read(frame); 
        sem_post(&semCaptureFrameCplt);
        //*******************************//
        if (count == 1000)
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread capture frame " << fps << "\n";
            count = 0;
        }
        //*******************************//
    }
}

void processFrame(void)
{
    //*******************************//
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    bool bFoundObject = false;  
    int x = 240,y =240;
    int xKF, yKF;
    double refArea;
    bool objectFound = false;
    int index;
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    //*******************************//
    sleep(1);
    while(1)
    {     
        //*******************************//
        sem_wait(&semCaptureFrameCplt);
        //*******************************//
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        //*******************************//
        cvtColor(frame,gray,COLOR_BGR2GRAY);
        threshold(gray,thresh,T,255,0);
        findContours(thresh,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE );
        //*******************************//
        if (hierarchy.size() > 0) 
        {
            refArea = 0;
            if(hierarchy.size() < MAX_NUM_OBJECTS)
            {
                for (index = 0; index >= 0; index = hierarchy[index][0]) 
                {
                    Moments moment = moments((cv::Mat)contours[index]);
                    if(moment.m00 > MIN_OBJECT_AREA && moment.m00 < MAX_OBJECT_AREA && moment.m00 > refArea)
                    {
                        x = moment.m10/moment.m00;
                        y = moment.m01/moment.m00;
                        bFoundObject = true;
                        refArea = moment.m00;
                    }
                    else 
                        bFoundObject = false;
                }
            }
            else 
            {
                putText(frame,"TOO MUCH NOISE!",Point(0,50),1,2,Scalar(0,0,255),2);
                bFoundObject = false;
            }
        }
        sem_post(&semProcessFrameCplt);
        //*******************************//
        xKF = KF.predict(x);
        yKF = KF.predict(y);
        STMMutex.lock();
        if (bFoundObject)
            STMData.detectedBall = 1;
        else
            STMData.detectedBall = 0;
        STMData.ballCoordinate[BPS_X_AXIS] = xKF;
        STMData.ballCoordinate[BPS_Y_AXIS] = yKF;
        UART.send(&STMData, sizeof(bpsUARTSendDataTypeDef));
        STMMutex.unlock();
        AppMutex.lock();
        AppData.ballCoordinate[BPS_X_AXIS] = xKF;
        AppData.ballCoordinate[BPS_Y_AXIS] = yKF;
        server.send((char *)&AppData, sizeof(bpsSocketSendDataTypeDef));
        AppMutex.unlock();
        //*******************************//
        if (count == 1000)
        {         
            auto end = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread process frame " << fps << "\n";
            count = 0;
        }
        //*******************************//
    }
}

void showImage(void)
{
    //*******************************//
    Rect2d bbox;
    char winName[6] = "frame";
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    float fps;
    //*******************************//
    B=70;
    C=70;
    S=100;
    T=90;
    namedWindow(winName);
    createTrackbar( "Brightness",winName, &B, 100, onTrackbarChanged );
    createTrackbar( "Contrast",winName, &C, 100,onTrackbarChanged );
    createTrackbar( "Saturation",winName, &S, 100,onTrackbarChanged);
    createTrackbar( "Thres",winName, &T, 255,onTrackbarChanged);
    //*******************************//
    sleep(1);
    while(1)
    {
        //*******************************//
        sem_wait(&semProcessFrameCplt);
        //*******************************//
        if (count == 0)
            start = std::chrono::high_resolution_clock::now();
        count++;
        //*******************************//
        Rect2d bbox(STMData.ballCoordinate[BPS_X_AXIS], 
                    STMData.ballCoordinate[BPS_Y_AXIS], RECT_SIZE, RECT_SIZE); 
        rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
        imshow("frame", frame);
        imshow("thresh", thresh);
        waitKey(1);
        //*******************************//
        if (count == 1000)
        {
            auto end = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<chrono::seconds>(end - start);
            fps = 1000 / static_cast<double>(diff.count());
            std::cout << "thread show frame " << fps << "\n";
            count = 0;
        }
        //*******************************//
    }
}


int recvFunc (char *recvData, int recvLen)
{
    //*******************************//
    bpsSocketReceiveDataTypeDef *data = (bpsSocketReceiveDataTypeDef*)recvData ;
    STMMutex.lock();
    memcpy(&STMData.command, data, sizeof(bpsSocketReceiveDataTypeDef));
    STMMutex.unlock();
    //*******************************//
    if (UART.dataAvailable())
    {
        UART.recv(&RaspiEncoderCnt,sizeof(bpsUARTReceiveDataTypeDef));
        AppMutex.lock();
        memcpy(&AppData.encoderCnt, &RaspiEncoderCnt, sizeof(bpsUARTReceiveDataTypeDef));
        AppMutex.unlock();
        std::cout << "encoderCnt x:= " << AppData.encoderCnt[0] << std::endl;
        std::cout << "encoderCnt y:= " << AppData.encoderCnt[1] << std::endl;
    }
    //*******************************//
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

            std::cout << "Kp Outer x: " << data->content.PIDProperties.Kp[BPS_OUTER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kp Outer y: " << data->content.PIDProperties.Kp[BPS_OUTER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Kp Inner x: " << data->content.PIDProperties.Kp[BPS_INNER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kp Inner y: " << data->content.PIDProperties.Kp[BPS_INNER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Ki Outer x: " << data->content.PIDProperties.Ki[BPS_OUTER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Ki Outer y: " << data->content.PIDProperties.Ki[BPS_OUTER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Ki Inner x: " << data->content.PIDProperties.Ki[BPS_INNER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Ki Inner y: " << data->content.PIDProperties.Ki[BPS_INNER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Kd Outer x: " << data->content.PIDProperties.Kd[BPS_OUTER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kd Outer y: " << data->content.PIDProperties.Kd[BPS_OUTER_PID][BPS_Y_AXIS] << std::endl;

            std::cout << "Kd Inner x: " << data->content.PIDProperties.Kd[BPS_INNER_PID][BPS_X_AXIS] << " -- ";
            std::cout << "Kd Inner y: " << data->content.PIDProperties.Kd[BPS_INNER_PID][BPS_Y_AXIS] << std::endl;

            break;
        default:
            std::cout << "error data\n";
            break;
    }
    //*******************************//
    return 0;
}

void onTrackbarChanged(int, void*)
{
    //*******************************//
    video.set(CAP_PROP_BRIGHTNESS, float(B)/100);
    video.set(CAP_PROP_CONTRAST, float(C)/100);
    video.set(CAP_PROP_SATURATION, float(S)/100);
    //*******************************//
}