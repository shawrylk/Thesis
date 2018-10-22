
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include "./hpp/bpsUARTData.hpp"
#include <sys/ioctl.h>
#include <unistd.h>
#define PORT 22396
   
bpsUARTSendDataTypeDef sendData;
 int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread, on = 1; 
    struct sockaddr_in serv_addr; 
    char *hello = "LOGIN:pi:raspberry:"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    // int rc = ioctl(sock, FIONBIO, (char *)&on);
    // if (rc < 0)
    // {
    //     printf( "ioctl() failed");
    //     close(sock);
    //     exit(-1);
    // }
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    send(sock , hello , strlen(hello) , 0 ); 
    printf("Login message sent\n"); 
    valread = read( sock , buffer, 1024); 
    printf("%s\n",buffer ); 
    sendData.command = BPS_MODE_CIRCLE;
    sendData.content.circleProperties.centerCoordinate[BPS_X_AXIS] = 0;
    sendData.content.circleProperties.centerCoordinate[BPS_Y_AXIS] = 0;
    sendData.content.circleProperties.radius = 0;
    sendData.content.circleProperties.speed  = 1000;
    send(sock , &sendData , sizeof(bpsUARTSendDataTypeDef) , 0 ); 
    int count = 0;
    while(1)
    {
        // if (count == 200)
        // {
            sendData.content.circleProperties.centerCoordinate[BPS_X_AXIS]++;
            sendData.content.circleProperties.centerCoordinate[BPS_Y_AXIS]++;
            sendData.content.circleProperties.radius+=0.01;
            sendData.content.circleProperties.speed+=1;
            send(sock , &sendData , sizeof(bpsUARTSendDataTypeDef) , 0 ); 
        //    count = 0;
        // }
        // count++;
        //printf("size of UART %ld",sizeof(bpsUARTSendDataTypeDef));
        //send(sock , &sendData , sizeof(bpsUARTSendDataTypeDef) , 0 ); 
        valread = recv( sock , &ballCoordinate, sizeof(ballCoordinate),0); 
        printf("x = %d; y = %d\n" ,ballCoordinate[BPS_X_AXIS] ,ballCoordinate[BPS_Y_AXIS] );
        //usleep(1000000);
    }
    return 0; 
} 

