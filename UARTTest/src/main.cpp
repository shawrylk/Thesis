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
	fdes = open("/dev/serial0", O_RDWR);
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
 
	while(1) {
		send(fdes, "hello", 5, MSG_DONTWAIT );
		sleep(1);
	}
	return 0;
}

int mrecv(int fdes) {
 
	char* buff = new char[10];
	printf("Raspberry's receiving : \n");
 
	while(1) {
			recv(fdes, buff, 5, 0);
			std::cout << buff << std::endl;
		}
	
	return 0;
}

