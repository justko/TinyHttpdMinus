#include "MicroHttpd.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

MicroHttpd::MicroHttpd(unsigned short port) : port(port), httpd(0) {}
void MicroHttpd::errorDie(const char *sc)
{
    perror(sc);
    exit(0);
}

void MicroHttpd::startup()
{
    int on = 1;
    sockaddr_in name;
    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1)
        errorDie("ERROR at socket");
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        errorDie("ERROR at setsockopt");
    if (bind(httpd, (sockaddr *)&name, sizeof(name)) < 0)
        errorDie("ERROR at bind");
    if (port == 0)
    {
        socklen_t namelen = sizeof(name);
        if (getsockname(httpd, (sockaddr *)&name, &namelen) == -1)
            errorDie("ERROR at getsockname");
        port = ntohs(name.sin_port);
    }
    if (listen(httpd, 5) < 0)
        errorDie("ERROR at listen");
    printf("MicroHttpd is running on port %d\n", port);
}



void MicroHttpd::looping()
{
    startup();
    int clientSock = -1;
    sockaddr_in clientName;
    socklen_t clientNameLen = sizeof(clientName);

    while (true)
    {
        clientSock = accept(httpd, (sockaddr *)&clientName, &clientNameLen);
        if (clientSock == -1)
            errorDie("ERROR at accept");
        //printf("接受连接:%d\n",clientSock);

        //接受所做的操作
        acceptRequest((void*)(intptr_t)clientSock);


    }
    shutdown();
}

void MicroHttpd::shutdown(){
    close(httpd);
}


void MicroHttpd::acceptRequest(void* arg){
    //待完成
    int clientSock=(intptr_t)arg;
    char buff[1024];
    int numChars=0;
    badRequest(clientSock);

    close(clientSock);
}


//暂未使用
int MicroHttpd::getLine(int sock, char *buff, int size){
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        if(n<=0)break;
        if(c=='\r'){
            n=recv(sock,&c,1,MSG_PEEK);//观看\r下一个字符是不是\n
            if((n>0)&&(c=='\n')){
                recv(sock,&c,1,0);
            }else{
                c='\n';
            }
        }
        buff[i]=c;
        i++;
    }
    buff[i] = '\0';
    return i;
}

void MicroHttpd::badRequest(int client){
    char buf[1024];
    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
}