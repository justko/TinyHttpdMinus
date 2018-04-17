#pragma once
class MicroClient{


    public :
    MicroClient(unsigned short port);
    void startup();
    void shutdown();
    void recvResponse();
    private:
    
    unsigned short port;
    int client;

};