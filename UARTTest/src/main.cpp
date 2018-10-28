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
	int i = 0;
	char *buff = new char[10];
	ball[0] = 0x1111;
	ball[1] = 0xFFFF;
	memcpy(buff,(char*)&ball, sizeof(int16_t) * 2);
	printf("Raspberry's sending : \n");
 
	while(1) {
		i = 0;
		while (i < 4)
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

