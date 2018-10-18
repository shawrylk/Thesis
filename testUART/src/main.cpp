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
    // bpsUARTSendDataTypeDef sendData;
    // sendData.ballCoordinate[BPS_X_AXIS] = 0x1234;
    // sendData.ballCoordinate[BPS_Y_AXIS] = 0x5678;
    // sendData.command = BPS_MODE_SETPOINT;
    // sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 0x9ABC;
    // sendData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 0xDEF0;
    // sendData.nullTerminated = '\0';
    // std::cout << "SIZE: \n";
    // std::cout << sizeof(bpsUARTSendDataTypeDef) << "\n";
    // if (bpsUARTInit() != BPS_OK)
    // {
    //     std::cout << "open UART fails \n";
    //     return -1;
    // }
    // while (1)
    // {
    //     bpsUARTSendData(&sendData);
    //     std::cout << "sent \n";
    //     sleep(1);
    // }
    int fd;
 
	printf("Raspberry's sending : \n");
 
	while(1) {
		if((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0 ){
			fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno));
		}
		serialPuts(fd, "hello");
		serialFlush(fd);
		printf("%s\n", "hello");
		fflush(stdout);
		sleep(1);
	}
    return 0;
}




