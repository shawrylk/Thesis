#include "../hpp/bpsUARTData.hpp"

int fd;

bpsStatusTypeDef bpsUARTInit()
{
    if((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;
}


bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData)
{
	serialPuts(fd, (const char*)sendData);
    //serialPuts(fd, "hello");
	serialFlush(fd);
    return BPS_OK;
}
