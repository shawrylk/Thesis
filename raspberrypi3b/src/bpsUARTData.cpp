
#include "../hpp/bpsUARTData.hpp"
int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
	if ((fdes= serialOpen ("/dev/serial0", 9600)) < 0 )
    {
        return BPS_ERROR;
	}
    return BPS_OK;

}

bpsStatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData, int len)
 {
    //int i = 0;
    char *buff = new char[len];
    memcpy(buff, sendData, len);
    std::cout << "about to send \n";
    for(int i = 0 ; i < 64; i+=8)
    {
        serialPuts(fdes,"cho_ma_bon_me_may_sao_deo_chay_thu_vien_ngu_nhu_cac");
    }
    // while (i < len)
    // {
    //     serialPutchar(fdes, *(buff+i++));
    //     serialFlush(fdes);
    //     std::cout << i << std::endl;
    // }
    // serialPuts(fdes, "disconmemay");
    // serialFlush(fdes);
	return BPS_OK;
}

bpsStatusTypeDef bpsUARTReceiveData	(bpsUARTSendDataTypeDef* recvData, int len)
{
    char *buff = new char[len];
    int i = 0;
    std::cout << "recv sth\n";
    do
    {
        buff[i++] = serialGetchar(fdes);
        std::cout << i << std::endl;
    }
    while(serialDataAvail(fdes));
    
    printf("%s",buff);
    memcpy(recvData, buff, len);
	return BPS_OK;
}

