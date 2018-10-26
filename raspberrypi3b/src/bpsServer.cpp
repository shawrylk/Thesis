
#include "../hpp/bpsServer.hpp"

Server::Server(char *user, char *pass, int sendLen, int recvlen)
    : recvLen(recvLen), sendLen(sendLen)
{
    loginString = new char[40];
    memcpy(loginString,"LOGIN:",40);
    strcat(loginString,user);
    strcat(loginString,":");
    strcat(loginString,pass);
    strcat(loginString,":");
};

int Server::Start(pfunc sendFunc, pfunc recvFunc)
{
    char* recvData = new char(recvLen);
    char* sendData = new char(sendLen);
    int on = 1, nfds = 1, listen_sd = -1, new_sd = -1;
    struct sockaddr_in6   addr;
    struct pollfd fds[3];
    char   buff[40];
    bool    close_conn, end_server = false, compress_array = false;

    listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sd < 0)
        return -1;

    if (setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        close(listen_sd);
        return -2;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family      = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port        = htons(SERVER_PORT);

    if (bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(listen_sd);
        return -3;
    }

    if (listen(listen_sd, 32) < 0)
    {
        close(listen_sd);
        return -4;
    }
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;
    int current_size;
    do 
    {
        std::cout << "Waiting on poll()...\n";

        if (poll(fds, nfds, -1) < 0)
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
                do
                {
                    new_sd = accept4(listen_sd, NULL, NULL, SOCK_NONBLOCK);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            end_server = true;
                            break;
                        }
                    }
                    if (recvSync(new_sd, buff, sizeof(buff)) <= 0)
                        close_conn = true;
                        std::cout << (char*)buff << "\n";
                    if (strncmp(loginString , (char*)buff, strlen(loginString)) == 0)
                    {
                        strncpy(buff,"SUCCEED:",8);
                        fds[nfds].fd = new_sd;
                        fds[nfds].events = POLLIN;
                        nfds++;
                    }
                    else
                    {
                        strncpy(buff,"LOGFAIL:",8);
                        close_conn = true;
                    }
                    if(sendSync(new_sd, buff, 8) <= 0)
                        close_conn = true;
                    new_sd = -1;
                } while (new_sd != -1);
            }
            else
            {
                std::cout <<"  Descriptor " << fds[i].fd << " is readable\n";
                close_conn = false;
                do
                {
                    int rc = recvAsync(fds[i].fd, recvData, recvLen);
                    if (rc <= -1)
                    {
                        close_conn = true;
                        break;
                    }
                    else if (rc > 0)
                        {
                            if (recvFunc != NULL)
                                recvFunc(recvData, recvLen);
                        }
                    if (sendFunc != NULL)
                            sendFunc(sendData, sendLen);
                    if (sendSync(fds[i].fd, sendData, sendLen) <= 0)
                    {
                        close_conn = true;
                        break;
                    }
                    usleep(16000);
                } while(true);

                if (close_conn)
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
    return -5;
}
int Server::sendSync(int fd, char* buff, int len)
{
    int rc = -1;
    do 
    {
        rc = send(fd, buff, len, 0);
        if (rc < 0)
        if (errno != EWOULDBLOCK)
            break;
        if (rc == 0)
        break;
    } 
    while (rc <= 0);
    if (rc < 0) return -1;
    if (rc == 0) return 0;
    return rc;
}

int Server::recvSync(int fd, char* buff, int len)
{
    int rc = -1;
    do
    {
        rc = recv(fd, buff, len, 0);
        if (rc < 0)
        if (errno != EWOULDBLOCK)
            break;
        if (rc == 0)
        break;
    } 
    while (rc <= 0);
    if (rc < 0) return -1;
    if (rc == 0) return 0;
    return rc;
}
  
int Server::recvAsync(int fd, char* buff, int len)
{
    int rc = -1;
    rc = recv(fd, buff, len, 0);
    if (rc < 0)
        if (errno != EWOULDBLOCK)
        return -1;
        else
        return 0;
    if (rc == 0)
        return -2;
    return rc;
}
