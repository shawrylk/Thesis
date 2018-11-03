
#include "bpsServer.hpp"

bpsServer::bpsServer(const int port)
    :port(port), loginString(LOGIN_STRING)
{
}

void bpsServer::poll()
{
    char* recvData = new char[sizeof(bpsSocketReceiveDataTypeDef)];
    char* sendData = new char[sizeof(bpsSocketSendDataTypeDef)];
    int listen_sd = -1;
    bool    close_conn, end_server = false, compress_array = false;

    listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sd < 0)
        std::cout << "can't open socket\n";

    int on = 1;
    if (setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        close(listen_sd);
        std::cout << "can't set socket option\n";
    }

    struct sockaddr_in6   addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family      = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port        = htons(port);
    if (bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(listen_sd);
        std::cout << "can't bind socket\n";
    }

    if (listen(listen_sd, 3) < 0)
    {
        close(listen_sd);
        std::cout << "can't listen on this socket\n";
    }

    struct pollfd fds[3];
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;

    int current_size, nfds = 1;
    do 
    {
        std::cout << "waiting on poll()...\n";

        if (::poll(fds, nfds, -1) < 0)
        {
            std:: cout << "poll failed\n";
            break;
        }
        current_size = nfds;
        for (int i = 0; i < current_size; i++)
        {
            if(fds[i].revents == 0)
                continue;
            if(fds[i].revents != POLLIN)
            {
                end_server = true;
                break;
            }
            
            if (fds[i].fd == listen_sd)
            {
                std::cout << " Incoming connection \n";
                int new_sd = -1;
                new_sd = accept4(listen_sd, NULL, NULL, 0);
                if (new_sd < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        end_server = true;
                        break;
                    }
                }
                if (login(new_sd) == BPS_OK)
                {
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    std::cout << "login ok\n";
                }
            }
            else
            {
                std::cout <<"  Descriptor " << fds[i].fd << " is readable\n";               
                if (processClient(fds[i].fd) != BPS_OK)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = true;
                }
            }  /* End of existing connection is readable             */
        } /* End of loop through pollable descriptors              */
        if (compress_array)
        {
            compress_array = false;
            for (int i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(int j = i; j < nfds; j++)
                        fds[j].fd = fds[j+1].fd;
                    i--;
                    nfds--;
                }
            }
        }
    } while (end_server == false); /* End of serving running.    */

    for (int i = 0; i < nfds; i++)
    {
    if(fds[i].fd >= 0)
        close(fds[i].fd);
    }
}


bpsStatusTypeDef bpsServer::login(int fd)
{
    char buff[strlen(loginString)];
    if (::recv(fd, buff, sizeof(buff),0) <= 0)
        return BPS_ERROR;
    if (strncmp(loginString , (char*)buff, strlen(loginString)) == 0)
    {
        strncpy(buff,"SUCCEED:",8);
        if(::send(fd, buff, 8, 0) <= 0)
            return BPS_ERROR;
    }
    else
    {
        strncpy(buff,"LOGFAIL:",8);
        ::send(fd, buff, 8, 0);
        return BPS_ERROR;
    }
    
    clientFd = fd;
    return BPS_OK;
}

bpsStatusTypeDef bpsServer::processClient(int fd)
{
    //bpsSocketReceiveDataTypeDef *recvData;
    char *buff = new char[52];
    if (::recv(fd, buff, 52, 0) <= 0)
    {
        std::cout << "errno " << errno << std::endl;
        return BPS_ERROR;
    }
    else
        if (recvFunc != NULL)
        {
            //memcpy(recvData, buff, 52);
            recvFunc(buff, sizeof(bpsSocketReceiveDataTypeDef));
        }
        else
            std::cout << "recvFunc is NULL\n";
    return BPS_OK;
}

void bpsServer::attach(pfunc recv)
{
    recvFunc = recv;
}

void bpsServer::send(char *data, int len)
{
    ::send(clientFd, data, len, 0);
}