
#include "Httpd.h"
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

/**********************************************************************/

int main(void)
{
    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    pthread_t newthread;

    Httpd server(4000);
    server.startup();

    printf("httpd running on port %d\n", server.getPort());

    while (1)
    {
        client_sock =accept(server.getSockNum(),
                             (struct sockaddr *)&client_name,
                             &client_name_len);
        if (client_sock == -1)
            server.errorDie("accept");
        /* accept_request(&client_sock); */
        if (pthread_create(&newthread, NULL, (void *(*)(void*))Httpd::acceptRequest, (void *)(intptr_t)client_sock) != 0)
            perror("pthread_create");
    }

    server.closed();

    return (0);
}