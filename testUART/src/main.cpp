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
    int fds;
	char c;
	printf("Raspberry's receiving : \n");
 
	while(1) {
		if((fds = serialOpen ("/dev/serial1", 9600)) < 0 ){
			fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		}else{
			do{
				c = serialGetchar(fds);
				printf("%c",c);
				fflush (stdout);
			}while(serialDataAvail(fds));
		}
	};
}




