#include "../hpp/bpsUARTData.hpp"

int fd;

bpsStatusTypeDef bpsUARTInit()
{
    if((fd = serialOpen ("/dev/ttyAMA0", 1000000)) < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;
}


bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData)
{
	serialPuts(fd, sendData);
	serialFlush(fd);
    return BPS_OK;
}
