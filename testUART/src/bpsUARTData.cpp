#include "../hpp/bpsUARTData.hpp"

static int fd;

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
    //serialPuts(fd, "hello");
	serialFlush(fd);
    return BPS_OK;
}
