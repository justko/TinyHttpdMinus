#include"MicroClient.h"
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        std::cout << "连接本地port端口并接受数据" << std::endl;
        std::cout << "Using: ./MicroHttpdClient.out port" << std::endl;
        exit(0);
    }
    //int port=atoi(argv[1]);
    int port=10086;
    MicroClient client(port);
    client.startup();
    client.recvResponse();
    client.shutdown();
    return 0;
}