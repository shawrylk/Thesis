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
#include "../hpp/bpsUARTData.hpp"
#include <iostream>
#include <string>
#define SERVER_PORT  22396

#define TRUE             1
#define FALSE            0

main (int argc, char *argv[])
{
  int    len, rc, on = 1;
  int    listen_sd = -1, new_sd = -1;
  int    desc_ready, end_server = FALSE, compress_array = FALSE;
  int    close_conn;
  char   buff[40];
  int size = 40;
  struct sockaddr_in6   addr;
  struct pollfd fds[3];
  int    nfds = 1, current_size = 0, i, j;
  std::string   loginString = "LOGIN:pi:raspberry:";
  bpsUARTReceiveDataTypeDef recvData;
  int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS] = { 0, 0};

  listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
  if (listen_sd < 0)
  {
    std::cerr << "socket() failed";
    exit(-1);
  }

  rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                  (char *)&on, sizeof(on));
  if (rc < 0)
  {
    std::cerr << "setsockopt() failed";
    close(listen_sd);
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin6_family      = AF_INET6;
  memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
  addr.sin6_port        = htons(SERVER_PORT);
  rc = bind(listen_sd,
            (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0)
  {
    std::cerr << "bind() failed";
    close(listen_sd);
    exit(-1);
  }

  rc = listen(listen_sd, 32);
  if (rc < 0)
  {
    std::cerr << "listen() failed";
    close(listen_sd);
    exit(-1);
  }

  memset(fds, 0 , sizeof(fds));
  fds[0].fd = listen_sd;
  fds[0].events = POLLIN;
  do
  {
    std::cout << "Waiting on poll()...\n";
    rc = poll(fds, nfds, -1);
    if (rc < 0)
    {
      std::cerr << "Poll() failed \n";
      break;
    }
    current_size = nfds;
    for (i = 0; i < current_size; i++)
    {
      std::cout << "fd " << i << "\n";
      if(fds[i].revents == 0)
        continue;
      if(fds[i].revents != POLLIN)
      {
        std::cout <<"  Error! revents = " << fds[i].revents << "\n";
        end_server = TRUE;
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
              std::cout << "  accept() failed";
              end_server = TRUE;
              break;
            }
            
          }
          std::cout << "about to recv\n";
          do
          {
            rc = recv(new_sd, buff, sizeof(buff), 0);
            std::cout << "again\n";
            if (rc < 0)
            {
              if (errno != EWOULDBLOCK)
              {
                std::cout << "  recv() failed";
                close_conn = TRUE;
                break;
              }			
            }

            if (rc == 0)
            {
            std::cout << "  Connection closed\n";
            close_conn = TRUE;
            break;
            }
		      } while (rc <= 0);
          
          std::cout << " adbout here\n";
          printf("%s",(char*)buff);
          std::cout << "get here\n";
          try {
            if (strncmp("LOGIN:pi:raspberry:" , (char*)buff, 19) == 0)
            {
              std::cout<< "cant get here\n";
              len = 8;
              strncpy(buff,"SUCCEED:",len);
              fds[nfds].fd = new_sd;
              fds[nfds].events = POLLIN;
              nfds++;
              std::cout << "SUCCEED\n";
            }
            else
            {
              len = 8;
              strncpy(buff,"LOGFAIL:",len);
              std::cout << "FAIL\n";
              close_conn = TRUE;
            }
          }
          catch (std::exception e)
          {
            std::cout << "exception occurrs \n";
          }
          do {
            rc = send(new_sd, buff, len, 0);
            if (rc < 0)
            {
              if (errno != EWOULDBLOCK)
              {
                std::cout << "  send() failed";
                close_conn = TRUE;
                break;
              }
            }
          } while (rc <= 0);
          memset(buff, 0, size);
          std::cout << "sent\n";
          new_sd = -1;
        } while (new_sd != -1);
      }
      else
      {
        printf("  Descriptor %d is readable\n", fds[i].fd);
        close_conn = FALSE;
        do
        {
          rc = recv(fds[i].fd, &recvData, 60, 0);
          if (rc < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              std::cerr << "  recv() failed";
              close_conn = TRUE;
              break;
            }

          }
          if (rc == 0)
          {
            printf("  Connection closed\n");
            close_conn = TRUE;
            break;
          }
          // len = rc;
          // printf("  %d bytes received\n", len);
          if (rc > 0)
          {
            switch (recvData.command)
            {
              case  BPS_MODE_CIRCLE:
                std::cout << "circle mode \n";
                std::cout << "x: " << recvData.content.circleProperties.centerCoordinate[BPS_X_AXIS] << std::endl;
                std::cout << "y: " << recvData.content.circleProperties.centerCoordinate[BPS_Y_AXIS] << std::endl;
                std::cout << "r: " << recvData.content.circleProperties.radius << std::endl;
                std::cout << "s: " << recvData.content.circleProperties.speed << std::endl;
                break;
              default:
                std::cout << "other mode \n";
                break;
            }
          }
          do
          {
            std::cout << ballCoordinate[BPS_X_AXIS]++ << "\n";
            std::cout << ballCoordinate[BPS_Y_AXIS]++ << "\n";
            rc = send(fds[i].fd, &ballCoordinate, sizeof(ballCoordinate), 0);
            if (rc < 0)
            {
              if (errno != EWOULDBLOCK)
              {
                std::cerr << "  send() failed";
                close_conn = TRUE;
                break;
              }
            }
          } while (rc <= 0);
          usleep(16000);
        } while(TRUE);

        if (close_conn)
        {
          close(fds[i].fd);
          fds[i].fd = -1;
          compress_array = TRUE;
          printf("closed con\n");
        }


      }  /* End of existing connection is readable             */
    } /* End of loop through pollable descriptors              */
    if (compress_array)
    {
      std::cout << "before nfds = " << nfds << " i = " << i << "\n";
      compress_array = FALSE;
      for (i = 0; i < nfds; i++)
      {
        if (fds[i].fd == -1)
        {
          for(j = i; j < nfds; j++)
          {
            fds[j].fd = fds[j+1].fd;
          }
          i--;
          nfds--;
          std::cout << "after nfds = " << nfds << " i = " << i << "\n";
        }
      }
      printf("compressed\n");
    }

  } while (end_server == FALSE); /* End of serving running.    */
  for (i = 0; i < nfds; i++)
  {
    if(fds[i].fd >= 0)
      close(fds[i].fd);
  }
}

