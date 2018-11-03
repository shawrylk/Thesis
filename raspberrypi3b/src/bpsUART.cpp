
#include "bpsUART.hpp"



bpsUART::bpsUART(const char *device, const int baud)
{
  struct termios options ;
  speed_t myBaud;
  int     status;
  switch (baud)
  {
    case      50:	myBaud =      B50 ; break ;
    case      75:	myBaud =      B75 ; break ;
    case     110:	myBaud =     B110 ; break ;
    case     134:	myBaud =     B134 ; break ;
    case     150:	myBaud =     B150 ; break ;
    case     200:	myBaud =     B200 ; break ;
    case     300:	myBaud =     B300 ; break ;
    case     600:	myBaud =     B600 ; break ;
    case    1200:	myBaud =    B1200 ; break ;
    case    1800:	myBaud =    B1800 ; break ;
    case    2400:	myBaud =    B2400 ; break ;
    case    4800:	myBaud =    B4800 ; break ;
    case    9600:	myBaud =    B9600 ; break ;
    case   19200:	myBaud =   B19200 ; break ;
    case   38400:	myBaud =   B38400 ; break ;
    case   57600:	myBaud =   B57600 ; break ;
    case  115200:	myBaud =  B115200 ; break ;
    case  230400:	myBaud =  B230400 ; break ;
    case  460800:	myBaud =  B460800 ; break ;
    case  500000:	myBaud =  B500000 ; break ;
    case  576000:	myBaud =  B576000 ; break ;
    case  921600:	myBaud =  B921600 ; break ;
    case 1000000:	myBaud = B1000000 ; break ;
    case 1152000:	myBaud = B1152000 ; break ;
    case 1500000:	myBaud = B1500000 ; break ;
    case 2000000:	myBaud = B2000000 ; break ;
    case 2500000:	myBaud = B2500000 ; break ;
    case 3000000:	myBaud = B3000000 ; break ;
    case 3500000:	myBaud = B3500000 ; break ;
    case 4000000:	myBaud = B4000000 ; break ;

    default:
      std::cout << "baud rate " << baud << " is not supported" << std::endl;
  }

  if ((fdes = open (device, O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
    std::cout << "cant open device " << device << std::endl;

  fcntl (fdes, F_SETFL, O_RDWR) ;
  tcgetattr (fdes, &options) ;

  cfmakeraw   (&options) ;
  cfsetispeed (&options, myBaud) ;
  cfsetospeed (&options, myBaud) ;

  options.c_cflag |= (CLOCAL | CREAD) ;
  options.c_cflag &= ~PARENB ;
  options.c_cflag &= ~CSTOPB ;
  options.c_cflag &= ~CSIZE ;
  options.c_cflag |= CS8 ;
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;
  options.c_oflag &= ~OPOST ;

  options.c_cc [VMIN]  =   0 ;
  options.c_cc [VTIME] = 100 ;	// Ten seconds (100 deciseconds)

  tcsetattr (fdes, TCSANOW, &options) ;

  ioctl (fdes, TIOCMGET, &status);

  status |= TIOCM_DTR ;
  status |= TIOCM_RTS ;

  ioctl (fdes, TIOCMSET, &status);
  usleep (10000) ;	// 10mS
}

int bpsUART::send(bpsUARTSendDataTypeDef* sendData, int len)
{
    int n;
    usleep(100);
    n = write(fdes, sendData, len);
    while(n != len);
    return n;
}

int bpsUART::recv(bpsUARTReceiveDataTypeDef* recvData, int len)
{
    int i = 0;
    char* buff = new char[len];
    //tcflush (fdes, TCIOFLUSH);
    while(i != len) 
    {
      read(fdes, &buff[i], 1);
      i = i + 1;
	  }
    memcpy(recvData, buff, len);
    return len;
}

int bpsUART::dataAvailable()
{
  int result ;

  if (ioctl (fdes, FIONREAD, &result) == -1)
    return -1 ;

  return result ;
}

void bpsUART::flush()
{
  tcflush (fdes, TCIOFLUSH) ;
}

void bpsUART::close()
{
  ::close(fdes);
}