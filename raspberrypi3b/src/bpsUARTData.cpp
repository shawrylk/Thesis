#include "../hpp/bpsUARTData.hpp"

int fdes;



bpsStatusTypeDef bpsUARTInit(void)
{
    fdes = serialOpen ("/dev/serial0", 1000000);
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
    int i = 0, n =0;
    char* buff = new char[len];
    while(i != len) {
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
    memccpy(recvData, buff, len);
    return len;
}

