#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <thread>
#include <iostream>

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
		write(fdes, "hello", 5);
		sleep(1);
	}
	return 0;
}

int mrecv(int fdes) {
 
	char* buff = new char[10];
	printf("Raspberry's receiving : \n");
 
	while(1) {
			read(fdes, buff, 5);
			std::cout << buff << std::endl;
		}
	
	return 0;
}

