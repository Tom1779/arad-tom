#include "server.h"

int main()
{
    char line[MAX];
    server_init();
    while (1)
    { // Try to accept a client request
        printf("server: accepting new connection ....\n");
        // Try to accept a client connection as descriptor newsock
        len = sizeof(client_addr);
        csock = accept(mysock, (struct sockaddr *)&client_addr, &len);
        if (csock < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }
        printf("server: accepted a client connection from\n");
        printf("---------------------------------------------–\n");
        printf("Clinet: IP=%s port=%d\n",
               inet_ntoa(client_addr.sin_addr.s_addr),
               ntohs(client_addr.sin_port));
        printf("---------------------------------------------–\n");
            // Processing loop: client_sock <== data ==> client
            while (1)
        {
            n = read(csock, line, MAX);
            if (n == 0)
            {
                printf("server: client died, server loops\n");
                close(csock);
                break;
            }
            // show the line string
            printf("server: read n=%d bytes; line=%s\n", n, line);
            // echo line to client
            n = write(csock, line, MAX);
            printf("server: wrote n=%d bytes; ECHO=%s\n", n, line);
            printf("server: ready for next request\n");
        }
    }
}