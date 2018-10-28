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
	char *buff = new char[20];
	ball[0] = 0x01;
	ball[1] = 0x02;
	ball[2] = 0x03;
	ball[3] = 0x04;
	ball[4] = 0x05;
	ball[5] = 0x06;
	ball[6] = 0x07;
	ball[7] = 0x08;
	ball[8] = 0x09;
	ball[9] = 0x0A;
	for (int j = 0; j< 10; j++)
		snprintf(&buff[j*sizeof(int16_t)],sizeof(int16_t),"%d",ball[j]);
	printf("Raspberry's sending : \n");
	printf("buff number \n");
	for (int j =0; j < 20; j++)
		printf("%x ",buff[j]);
	printf("\n");
	printf("ball number \n");
	for (int j =0; j < 10; j++)
		printf("%x ",ball[j]);
	printf("\n");
	fflush(stdout);
	while(1) {
		i = 0;
		while (i < 20)
		{
			serialPuts(fdes,buff+i);
			serialFlush(fdes);
			i++;
		}
		
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
	char *buff = new char[20];
	int16_t ball[10];
	while(1) {
			
			
			do{
				buff[i++] = serialGetchar(fdes);
				
			}while(serialDataAvail(fdes));
			if (i == 20)
			{
				
				memcpy(ball, buff, 20);
				printf("number: %d\n", i);
				for (int j =0; j < 20; j++)
					printf("%x ",buff[j]);
				printf("\n");
				for (int j =0; j < 10; j++)
					printf("%x ",ball[j]);
				printf("\n");
				fflush(stdout);
				memset(buff,0,20);
				i = 0;
			}
		}
	
	return 0;
}

