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

#include <iostream>

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
        acceptRequest((void *)(intptr_t)clientSock);
    }
    shutdown();
}

void MicroHttpd::shutdown()
{
    close(httpd);
}

void MicroHttpd::acceptRequest(void *arg)
{
    int client = (intptr_t)arg;
    char buff[1024];
    char method[255];
    char url[1024];
    char path[1024];
    bool isCGI;
    char *queryString = nullptr;
    //接收一行
    int numChars = getLine(client, buff, sizeof(buff));
    //
    int i = 0;
    while (!isspace(buff[i]) && (i < sizeof(method) - 1))
    {
        method[i] = buff[i];
        i++;
    }
    method[i] = '\0';
    std::cout << method << std::endl;
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
    {
        cannotExecute(client);
        return;
    }

    if (strcasecmp(method, "POST") == 0)
        isCGI = true;

    while (isspace(buff[i]) && (i < numChars))
        i++;
    int k = 0;
    for (k = 0; !isspace(buff[i]) && (k < sizeof(url) - 1) && (i < numChars); ++i, ++k)
    {
        url[k] = buff[i];
    }
    url[k] = '\0';
    //
    if (strcasecmp(method, "GET") == 0)
    {
        queryString = url;
        while ((*queryString != '?') && (*queryString != '\0'))
            queryString++;
        if (*queryString == '?')
        {
            isCGI = true;
            *queryString = '\0';
            queryString++;
        }
    }

    sprintf(path, "HTDoc%s", url);
    if (path[strlen(path) - 1] == '/')
        strcat(path, "index.html");
    std::cout << path << std::endl;

    struct stat st;
    if (stat(path, &st) == -1)
    {
        //剩余全都丢弃
        while ((numChars > 0) && strcmp("\n", buff)) // read & discard headers
            numChars = getLine(client, buff, sizeof(buff));
        //
        notFound(client);
    }
    else
    {
        //如果是目录仍然要加这个
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            strcat(path, "/index.html");
        if ((st.st_mode & S_IXUSR) ||
            (st.st_mode & S_IXGRP) ||
            (st.st_mode & S_IXOTH))
            isCGI = 1;

        if (!isCGI)
            serveFile(client, path);

        else
            exeCGI(client, path, method, queryString);
    }

    close(client);
}

void MicroHttpd::exeCGI(int client, const char *path,
                        const char *method, const char *query_string)
{
    char buf[1024];
    int cgi_output[2];
    int cgi_input[2];
    pid_t pid;
    int status;
    int i;
    char c;
    int numchars = 1;
    int content_length = -1;

    buf[0] = 'A';
    buf[1] = '\0';
    if (strcasecmp(method, "GET") == 0)
        while ((numchars > 0) && strcmp("\n", buf)) /* read & discard headers */
            numchars = getLine(client, buf, sizeof(buf));
    else if (strcasecmp(method, "POST") == 0) /*POST*/
    {
        numchars = getLine(client, buf, sizeof(buf));
        while ((numchars > 0) && strcmp("\n", buf))
        {
            buf[15] = '\0';
            if (strcasecmp(buf, "Content-Length:") == 0)
                content_length = atoi(&(buf[16]));
            numchars = getLine(client, buf, sizeof(buf));
        }
        if (content_length == -1)
        {
            badRequest(client);
            return;
        }
    }
    else /*HEAD or other*/
    {
    }

    if (pipe(cgi_output) < 0)
    {
        cannotExecute(client);
        return;
    }
    if (pipe(cgi_input) < 0)
    {
        cannotExecute(client);
        return;
    }

    if ((pid = fork()) < 0)
    {
        cannotExecute(client);
        return;
    }
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    if (pid == 0) /* child: CGI script */
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        dup2(cgi_output[1], STDOUT);
        dup2(cgi_input[0], STDIN);
        close(cgi_output[0]);
        close(cgi_input[1]);
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);
        if (strcasecmp(method, "GET") == 0)
        {
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }
        else
        { /* POST */
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }
        execl(path, NULL);
        exit(0);
    }
    else
    { /* parent */
        close(cgi_output[1]);
        close(cgi_input[0]);
        if (strcasecmp(method, "POST") == 0)
            for (i = 0; i < content_length; i++)
            {
                recv(client, &c, 1, 0);
                write(cgi_input[1], &c, 1);
            }
        while (read(cgi_output[0], &c, 1) > 0)
            send(client, &c, 1, 0);

        close(cgi_output[0]);
        close(cgi_input[1]);
        waitpid(pid, &status, 0);
    }
}

int MicroHttpd::getLine(int sock, char *buff, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        if (n <= 0)
            break;
        if (c == '\r')
        {
            n = recv(sock, &c, 1, MSG_PEEK); //观看\r下一个字符是不是\n
            if ((n > 0) && (c == '\n'))
            {
                recv(sock, &c, 1, 0);
            }
            else
            {
                c = '\n';
            }
        }
        buff[i] = c;
        i++;
    }
    buff[i] = '\0';
    return i;
}

void MicroHttpd::headers(int client, const char *filename)
{
    char buf[1024];
    (void)filename; /* could use filename to determine file type */
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}
void MicroHttpd::cat(int client, FILE *resource)
{
    char buf[1024];
    fgets(buf, sizeof(buf), resource);
    while (!feof(resource))
    {
        send(client, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), resource);
    }
}
void MicroHttpd::serveFile(int client, const char *filename)
{

    int numchars = 1;
    char buf[1024];
    buf[0] = 'A';
    buf[1] = '\0';
    while ((numchars > 0) && strcmp("\n", buf))       /* read & discard headers */
        numchars = getLine(client, buf, sizeof(buf)); //直到读取到空行

    FILE *resource = fopen(filename, "r");
    if (resource == NULL)
        notFound(client);
    else
    {
        headers(client, filename);
        cat(client, resource);
    }
    fclose(resource);
}

//特殊response

/*

*/
void MicroHttpd::badRequest(int client)
{
    char res[] =
        "HTTP/1.0 400 BAD REQUEST\r\n"
        "Content-type: text/html\r\n"
        "\r\n"
        "<P>Your browser sent a bad request, "
        "such as a POST without a Content-Length.\r\n";
    send(client, res, sizeof(res), 0);
}

void MicroHttpd::cannotExecute(int client)
{
    char res[] =
        "HTTP/1.0 500 Internal Server Error\r\n"
        "Content-type: text/html\r\n"
        "\r\n"
        "<P>Error prohibited CGI execution.\r\n";
    send(client, res, sizeof(res), 0);
}

void MicroHttpd::notFound(int client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}

void MicroHttpd::unimplemented(int client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}

const char *MicroHttpd::SERVER_STRING = "Server: jdbhttpd/0.1.0\r\n";
const unsigned MicroHttpd::STDIN = 0;
const unsigned MicroHttpd::STDOUT = 1;
const unsigned MicroHttpd::STDERR = 2;