#include "../hpp/bpsUARTData.hpp"

int fdes;



bpsStatusTypeDef bpsUARTInit(void)
{
    fdes = serialOpen ("/dev/serial0", 4000000);
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
    while (n != len);
    return n;
}

int bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    int i = 0, n =0;
    char* buff = new char[len];
    while(i != len) 
    {
        do
        {
            read(fdes, &buff[i++], 1);
            if (i == len)
                break;
        }
        while (serialDataAvail(fdes));
        if (i == len)
            break;
	}
    memcpy(recvData, buff, len);
    return len;
}

