
#include "../hpp/bpsUARTData.hpp"
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

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTReceiveDataTypeDef* recvData, int len)
{
	char c;
    char *buff = new char[len];
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

