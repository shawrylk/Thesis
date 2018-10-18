#include "../hpp/bpsUARTData.hpp"

int fd;

bpsStatusTypeDef bpsUARTInit()
{
    if((fd = serialOpen ("/dev/serial0", 9600)) < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;
}


bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData)
{
	serialPuts(fd, (const char*)sendData);
	serialFlush(fd);
    return BPS_OK;
}
