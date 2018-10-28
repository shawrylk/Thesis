#include "../hpp/bpsUARTData.hpp"

int fdes;



bpsStatusTypeDef bpsUARTInit(void)
{
    fdes = serialOpen("/dev/serial0", 9600);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;
}

int bpsUARTSendData(bpsUARTSendDataTypeDef* sendData, int len)
{
    return write(fdes, sendData, len);
}

int bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
  
    return read(fdes, recvData, len);
 
}

