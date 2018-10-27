
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
	return BPS_OK;
}

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    char *buff = new char[len];
    int i = 0;
    std::cout << "recv sth\n";
    do
    {
        buff[i++] = serialGetchar(fdes);
        std::cout << i << std::endl;
    }
    while(serialDataAvail(fdes));
    memcpy(recvData, buff, len);
	return BPS_OK;
}

