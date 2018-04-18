#include"MicroClient.h"
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc == 2)  //暂时不启用port选项
    {
        std::cout << "连接本地port端口并接受数据" << std::endl;
        std::cout << "Using: ./MicroClient.out port" << std::endl;
        exit(0);
    }
    //int port=atoi(argv[1]);
    int port=10086;
    MicroClient client(port);
    client.startup();
    client.sendRequest();       //请求"/"路径
    client.recvResponse();
    client.shutdown();
    return 0;
}