#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "连接本地port端口并接受数据" << std::endl;
        std::cout << "Using: ./MicroHttpdClient.out port" << std::endl;
        exit(0);
    }
    int port=atoi(argv[1]);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    int len = sizeof(address);
    int result = connect(sockfd, (struct sockaddr *)&address, len);
    if (result == -1)
    {
        perror("Oops: Client Connection\n");
        exit(1);
    }
    char buff[1024];
    while (recv(sockfd, buff, sizeof(buff), 0) > 0)
        std::cout<<buff;
    close(sockfd);
    exit(0);
}
