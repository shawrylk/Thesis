#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <thread>
#include <iostream>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
int msend(int);
int mrecv(int);
int fdes;

int main()
{
	fdes = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fdes < 0 )
    {
        return -1;
	}
  std::thread thread1(msend,fdes);
  std::thread thread2(mrecv,fdes);
  thread1.join();
  thread2.join();
      return 0;

}

int msend(int fdes) {
 
 
	printf("Raspberry's sending : \n");
	int n;
	while(1) {
		n = write(fdes, "hello", 5);
		if (n < 0)
  			fputs("write() of 5 bytes failed!\n", stderr);
		sleep(1);
	}
	return 0;
}

int mrecv(int fdes) {
 
	char* buff = new char[10];
	printf("Raspberry's receiving : \n");
 
	while(1) {
			read(fdes, buff, 5);
			std::cout << buff[0] << buff[1] << buff[2] << buff[3] << buff[4] << std::endl;
		}
	
	return 0;
}

