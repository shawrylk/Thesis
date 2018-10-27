
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

bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData1, int len)
 {
    //int i = 0;
    // char *buff = new char[len];
    // memcpy((void *)buff, (void *)sendData, len);
    // std::cout << "about to send \n";
    bpsUARTSendDataTypeDef sendData;
    sendData.ballCoordinate[BPS_X_AXIS] = 111;
    sendData.ballCoordinate[BPS_Y_AXIS] = 222;
    sendData.command = BPS_MODE_SETPOINT;
    sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 123;
    sendData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 456;
    sendData.nullTerminated = '\0';
    serialPuts(fdes,(const char*)&sendData);
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
    int i = 0;
    std::cout << "recv sth\n";
    do
    {
        buff[i++] = serialGetchar(fdes);
    }
    while(serialDataAvail(fdes));   
    printf("%s\n",buff);
    memcpy(recvData, buff, len);
	return BPS_OK;
}

