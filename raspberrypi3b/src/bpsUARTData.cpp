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
    n = write(fdes, "hello con cac ajinomoto wtf troi oi la troi chay asd iasd xcxzc ", 64);
    usleep(1000);
    return n;
}

int bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    len = 64;
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
    //memcpy(recvData, buff, len);
    std::cout << buff << std::endl;
    return len;
}

