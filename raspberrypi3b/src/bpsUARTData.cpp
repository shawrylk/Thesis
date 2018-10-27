#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <thread>

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
	if ((fdes= serialOpen ("/dev/serial0", 9600)) < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;

}

bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData)
 {
    serialPuts(fdes, (char*)sendData);
    serialFlush(fdes);
    fflush(stdout);
	return BPS_OK;
}

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTReceiveData* recvData, int len)
{
	char c;
    char buff[] = new char(len);
    int i = 0;
    do
    {
        if (i = len)
            break;
        buff[i++] = serialGetchar(fdes);
        fflush (stdout);
    }
    while(serialDataAvail(fdes));
	return BPS_OK;
}

