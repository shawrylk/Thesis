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
 
	int16_t ball[10];
	int i = 0;
	char *buff = new char[10];
	ball[0] = 1;
	ball[1] = 2;
	ball[2] = 3;
	ball[3] = 4;
	ball[4] = 5;
	ball[5] = 6;
	ball[6] = 7;
	ball[7] = 8;
	ball[8] = 9;
	ball[9] = 10;
	memcpy(buff,(char*)&ball, sizeof(int16_t) * 10);
	printf("Raspberry's sending : \n");
 
	while(1) {
		i = 0;
		while (i < 10)
		{
			serialPuts(fdes,buff+i);
			i++;
		}
		serialFlush(fdes);
		//printf("%s\n", "hello");
		fflush(stdout);
		delay(1000);
	}
	return 0;
}

int recv(int fdes) {
 
	char c;
	int i = 0;
	printf("Raspberry's receiving : \n");
	char *buff = new char[10];
	int16_t ball[10];
	while(1) {
			i = 0;
			memset(buff,0,10);
			do{
				buff[i++] = serialGetchar(fdes);
				
			}while(serialDataAvail(fdes));
			memcpy(ball, buff, 10);
			printf("number: %d", i);
			fflush(stdout);
		}
	
	return 0;
}

