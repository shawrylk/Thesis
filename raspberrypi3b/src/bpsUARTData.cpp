#include "../hpp/bpsUARTData.hpp"

int fdes;



bpsStatusTypeDef bpsUARTInit(void)
{
    fdes = serialOpen ("/dev/serial0", 9600);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;
}

int bpsUARTSendData(bpsUARTSendDataTypeDef* sendData, int len)
{
    int n;
    n = write(fdes, sendData, len);
    serialFlush(fdes);
    return n;
}

int bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    int n;
    do
    {
        n = read(fdes, recvData, len);
    }
    while (serialDataAvail(fdes));
    return n;
}

