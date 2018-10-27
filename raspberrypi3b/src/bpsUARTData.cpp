#include "../hpp/bpsUARTData.hpp"

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
    struct termios oldtio,newtio;
    fdes = open("/dev/serial0", O_RDWR | O_NONBLOCK);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
    tcgetattr(fdes,&oldtio); /* save current port settings */
        
        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        
        /* set input mode (non-canonical, no echo,...) */
        newtio.c_lflag = 0;
         
        newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
        
        tcflush(fdes, TCIFLUSH);
        tcsetattr(fdes,TCSANOW,&newtio);
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
