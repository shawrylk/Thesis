#include "../hpp/bpsUARTData.hpp"

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
    fdes = open("/dev/serial0", O_RDWR | O_NONBLOCK);
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
