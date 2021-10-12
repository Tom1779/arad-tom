#include "client.h"

int main()
{
    int n;
    char line[MAX], ans[MAX];
    client_init();
    printf("******** processing loop *********\n");
    while (1)
    {
        printf("input a line : ");
        bzero(line, MAX);           // zero out line[ ]
        fgets(line, MAX, stdin);    // get a line from stdin
        line[strlen(line) - 1] = 0; // kill \n at end
        if (line[0] == 0)           // exit if NULL line
            exit(0);
        // Send line to server
        n = write(sock, line, MAX);
        printf("client: wrote n=%d bytes; line=%s\n", n, line);
        // Read a line from sock and show it
        n = read(sock, ans, MAX);
        printf("client: read n=%d bytes; echo=%s\n", n, ans);
    }
}