
#pragma once

#include "bpsDefine.hpp"

#define LOGIN_STRING "LOGIN:pi:raspberry:"
using pfunc = std::function<int (char *data, int len)>;

class bpsServer
{
  private:
    const char *loginString;
    const int port;
    int clientFd;   
    pfunc recvFunc = NULL;
    bpsStatusTypeDef login(int fd);
    bpsStatusTypeDef processClient();
  public:
    bpsServer(const int port);
    void poll();
    void attach(pfunc recv);
    void send(char *data, int len);
};
