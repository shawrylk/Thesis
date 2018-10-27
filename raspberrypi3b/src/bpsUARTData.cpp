#include "../hpp/bpsUARTData.hpp"

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
    struct termios tio;
    


    // make the reads non-blocking

    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;

    fdes = open("/dev/serial0", O_RDWR | O_NONBLOCK);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
    bpsUARTSendDataTypeDef sendData, recvData;
    cfsetospeed(&tio,B115200);            // 115200 baud
    cfsetispeed(&tio,B115200);            // 115200 baud
    tcsetattr(fdes,TCSANOW,&tio);
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
