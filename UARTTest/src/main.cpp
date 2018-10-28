#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <thread>
int send(int);
int recv(int);
int fdes;
int main()
{
	if((fdes= serialOpen ("/dev/serial0", 9600)) < 0 )
    {
        return -1;
	}
  std::thread thread1(send,fdes);
  std::thread thread2(recv,fdes);
  thread1.join();
  thread2.join();
      return 0;

}

int send(int fdes) {
 
	int16_t ball[2];
	char *buff = new buff[10];
	ball[0] = 0x11;
	ball[1] = 0xFF;
	memcpy(buff,(char*)ball, sizeof(ball));
	printf("Raspberry's sending : \n");
 
	while(1) {
		serialPuts(fdes,(char*)&ball[0]);
		serialFlush(fdes);
		//printf("%s\n", "hello");
		fflush(stdout);
		delay(1000);
	}
	return 0;
}

int recv(int fdes) {
 
	char c;
	printf("Raspberry's receiving : \n");
 
	while(1) {
			do{
				c = serialGetchar(fdes);
				printf("%x",c);
				fflush (stdout);
			}while(serialDataAvail(fdes));
		}
	
	return 0;
}

