
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

bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData, int len)
 {
    //int i = 0;
    char *buff = new char[len];
    memcpy(buff, sendData, len);
    std::cout << "about to send \n";
    for(int i = 0 ; i < len; i+=8)
    {
        serialPuts(fdes,(buff + i));
    }
    // while (i < len)
    // {
    //     serialPutchar(fdes, *(buff+i++));
    //     serialFlush(fdes);
    //     std::cout << i << std::endl;
    // }
    // serialPuts(fdes, "disconmemay");
    // serialFlush(fdes);
	return BPS_OK;
}

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    char *buff = new char[len];
    static int i = 0;
    std::cout << "recv sth\n";
    do
    {
        buff[i++] = serialGetchar(fdes);
        std::cout << i << std::endl;
    }
    while(serialDataAvail(fdes));
    printf("%s",buff);
    if (i == 64)
    {
        memcpy(recvData, buff, len);
        i = 0;
    }
	return BPS_OK;
}

