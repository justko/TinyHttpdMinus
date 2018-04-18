#include"MicroClient.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include<string.h>
MicroClient::MicroClient(unsigned short port):port(port){
    
}

void MicroClient::startup(){
    client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    int len = sizeof(address);
    int result = connect(client, (struct sockaddr *)&address, len);
    if (result == -1)
    {
        perror("Oops: Client Connection\n");
        exit(1);
    }
}

void MicroClient::recvResponse(){
    char buff[1024];
    while (recv(client, buff, sizeof(buff), 0) > 0)
        std::cout<<buff;
}
void MicroClient::sendRequest(){
    char buff[1024];
    sprintf(buff,"GET / HTTP/1.1\r\n");
    send(client,buff,strlen(buff),0);
    sprintf(buff,"Host: localhost:10086\r\n");
    send(client,buff,strlen(buff),0);
    sprintf(buff,"User-Agent: MicroClient/1.4.0\r\n");
    send(client,buff,strlen(buff),0);
    sprintf(buff,"Accept: */*\r\n");
    send(client,buff,strlen(buff),0);
    sprintf(buff,"\r\n");
    send(client,buff,strlen(buff),0);
}



void MicroClient::shutdown(){
    close(client);
}