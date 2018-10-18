#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "../hpp/bpsUARTData.hpp"

using namespace cv;
using namespace std;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()


int main()
{
    cv::ocl::setUseOpenCL(false);
    bpsUARTSendDataTypeDef sendData;
    sendData.ballCoordinate[BPS_X_AXIS] = 640;
    sendData.ballCoordinate[BPS_Y_AXIS] = 480;
    sendData.command = BPS_MODE_SETPOINT;
    sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 320;
    sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 240;
    sendData.nullTerminated = NULL;
    bpsUARTInit();
    while (1)
    {
        bpsUARTSendData(&sendData);
        sleep(1);
    }

    return 0;
}




