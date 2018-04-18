#pragma once
class MicroClient{


    public :
    MicroClient(unsigned short port);
    void startup();
    void shutdown();
    void recvResponse();
    void sendRequest();
    private:
    
    unsigned short port;
    int client;

};