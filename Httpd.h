#pragma once
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
class Httpd
{
  public:

    Httpd(unsigned short port=0);
    
    static void execute_cgi(int, const char *, const char *, const char *);



    static void acceptRequest(void *arg);
    static int getLine(int sock,char *buff,int size);
    static void serveFile(int client, const char *filename);
    int startup();


    unsigned short getPort();
    int getSockNum();
    void closed();


    //给client发送bad request信息
    static void badRequest(int client);
    //给client发送can not execute信息
    static void cannotExecute(int client);
    //给client发送resource内容
    static void cat(int client, FILE *resource);
    //向perror发送sc信息
    static void errorDie(const char *sc);
    //向client发送filename的headers
    static void headers(int client, const char *filename);
    //向client发送404 not found信息
    static void notFound(int client);
    //向client发送unimplemented信息
    static void unimplemented(int client);
  private:
    static  bool isSpace(int x){
      return isspace((int)x);
    }
    unsigned short port;
    int socketNum;
    const static char* SERVER_STRING;
    const static unsigned STDIN;
    const static unsigned STDOUT;
    const static unsigned STDERR;
};