#include "../hpp/bpsUARTData.hpp"

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
    fdes = open("/dev/serial0", O_RDWR | O_NONBLOCK);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
    bpsUARTSendDataTypeDef sendData, recvData;
    bpsUARTSendData(&sendData, sizeof(bpsUARTSendDataTypeDef));
    bpsUARTReceiveData(&recvData, sizeof(bpsUARTSendDataTypeDef));
    return BPS_OK;
}

bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData, int len)
{
    int rc = -1;
    do
    {
        rc = write(fdes, sendData, len);
        if (rc < 0)
        if (errno != EWOULDBLOCK)
            break;
        if (rc == 0)
        break;
    } 
    while (rc <= 0);
    if (rc <= 0) return BPS_ERROR;
    return BPS_OK;
}

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    int rc = -1;
    do
    {
        rc = read(fdes, recvData, len);
        if (rc < 0)
        if (errno != EWOULDBLOCK)
            break;
        if (rc == 0)
        break;
    } 
    while (rc <= 0);
    if (rc <= 0) return BPS_ERROR;
    return BPS_OK;
}
