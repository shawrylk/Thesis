#include "../hpp/bpsUARTData.hpp"

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
    struct termios tio;
    struct termios stdio;
    struct termios old_stdio;
    int tty_fd;

    unsigned char c='D';
    tcgetattr(STDOUT_FILENO,&old_stdio);

    memset(&stdio,0,sizeof(stdio));
    stdio.c_iflag=0;
    stdio.c_oflag=0;
    stdio.c_cflag=0;
    stdio.c_lflag=0;
    stdio.c_cc[VMIN]=1;
    stdio.c_cc[VTIME]=0;
    tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
    tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking

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
    tcsetattr(tty_fd,TCSANOW,&tio);
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
