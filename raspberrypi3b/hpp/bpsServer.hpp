
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <string.h>
#include <functional>

#define SERVER_PORT  22396

using pfunc = std::function<int (char *data, int len)>;

class Server
{
  private:
    int16_t recvLen;
    int16_t sendLen;
    char *loginString;
    char *sendData, *recvData;
    int sendSync(int fd, char* buff, int len);
    int recvSync(int fd, char* buff, int len);
    int recvAsync(int fd, char* buff, int len);
  public:
    Server(char *user, char *pass, int16_t sendLen = 8, int16_t recvLen = 52);
    int Start(pfunc sendFunc, pfunc recvFunc);
};
