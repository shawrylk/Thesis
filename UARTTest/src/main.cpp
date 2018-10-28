#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>

int msend(int);
int mrecv(int);
int fdes;

int main()
{
	fdes = serialOpen("/dev/serial0",9600);
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
		write(fdes, "hello con cac ajinomoto wtf troi oi la troi chay asdiasdxcxzc ", 52);
		usleep(1000);
	}
	return 0;
}

int mrecv(int fdes) {
 
	char* buff = new char[10];
	printf("Raspberry's receiving : \n");
 
	while(1) {
			read(fdes, buff, 52);
			std::cout << buff << std::endl;
		}
	
	return 0;
}

