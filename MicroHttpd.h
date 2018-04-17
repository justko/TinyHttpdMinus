#pragma once
class MicroHttpd
{
  public:
    MicroHttpd(unsigned short port=0);
    void startup();
    void looping();
    void shutdown();
    void acceptRequest(void *arg);


  private:
    int getLine(int sock,char* buff,int size);
    void errorDie(const char *sc);
    void badRequest(int clientSock);

  private:
    int httpd;
    unsigned short port;
};