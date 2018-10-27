#include "../hpp/bpsUARTData.hpp"

int fdes;

bpsStatusTypeDef bpsUARTInit(void)
{
    struct termios term;
    int n;
    fdes = open("/dev/serial0", O_RDWR | O_NONBLOCK);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
    close(fdes);
    fdes = open("/dev/serial0", O_RDWR | O_NONBLOCK);
	if (fdes < 0 )
    {
        return BPS_ERROR;
	}
     if (n = tcgetattr(fdes, &term) == -1)
    {
        perror("tcgetattr");
        return BPS_ERROR;
    }

    if (n = cfsetispeed(&term, B115200) == -1)
    {
        perror("cfsetispeed");
        return BPS_ERROR;
    }

    if (n = cfsetospeed(&term, B115200) == -1)
    {
        perror("cfsetospeed");
        return BPS_ERROR;
    }

    term.c_cflag |= (CLOCAL | CREAD);
    term.c_cflag &= ~PARENB;
    term.c_cflag &= ~CSTOPB;
    term.c_cflag &= ~CSIZE;
    term.c_cflag |= CS8;
    term.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    term.c_iflag &= ~(IXON | IXOFF | IXANY);
    term.c_cflag &= ~CRTSCTS;
    term.c_oflag &= ~OPOST;

    if (n = tcsetattr(fdes, TCSANOW, &term) == -1)
    {
        perror("tcsetattr");
        return BPS_ERROR;
    }
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
