
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
    serialPuts(fdes, "hello"); //(char*)sendData);
    serialFlush(fdes);
    fflush(stdout);
	return BPS_OK;
}

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTReceiveDataTypeDef* recvData, int len)
{
	char c;
    char *buff = new char[len];
    int i = 0;
    std::cout << "recv sth\n";
    do
    {
        buff[i++] = serialGetchar(fdes);
        fflush (stdout);
    }
    while(serialDataAvail(fdes));
    printf("%s\n",*buff);
	return BPS_OK;
}

