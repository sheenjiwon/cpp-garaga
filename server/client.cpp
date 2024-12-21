#include "client.h"

#define BUF_SIZE 512

/**************************************************************
 * socket client
 * socket write
 * write: 64*32 matrix
***************************************************************/

void error_handling(char *message)
{
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

int sock_init(char *ip, int port) {
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("[Client]: socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        fprintf(stderr, "[Client]: connection error\n");
        return 1;
    }

    return sock;
}