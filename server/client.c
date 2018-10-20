
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include "./hpp/bpsUARTData.hpp"
#include <iostream>
#define PORT 22396
   
bpsUARTSendDataTypeDef sendData;
 int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "LOGIN:pi:raspberry:"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "192.168.0.16", &serv_addr.sin_addr)<=0)  
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
    sendData.command = BPS_MODE_SETPOINT;
    sendData.ballCoordinate[BPS_X_AXIS] = 120;
    sendData.ballCoordinate[BPS_Y_AXIS] = 120;
    send(sock , &sendData , sizeof(bpsUARTSendDataTypeDef) , 0 ); 
    while(1)
    {
        valread = read( sock , &ballCoordinate, sizeof(ballCoordinate); 
        std::cout << "x = " << ballCoordinate[BPS_X_AXIS] << " y = " << ballCoordinate[BPS_Y_AXIS] << "\n";
    }
    return 0; 
} 

