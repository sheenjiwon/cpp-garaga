#include "client.h"

#define BUF_SIZE 512

/**************************************************************
 * socket client
 * socket read & write
 * read: hp, cooltime
 * write: sensor data
***************************************************************/

void error_handling(char *message)
{
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

pthread_t ttid;
int indata[3];

void *t_function_w(void *data) {
    int sock_fd = *((int *)data);
    free(data);

    while (true) {
        ssize_t n = read(sock_fd, indata, sizeof(indata));
        usleep(100);
    }
    return 0;
}

int sock_init(char *ip, int port) {
    struct sockaddr_in serv_addr;
    int *sock = (int *)malloc(sizeof(int));

    if (sock == NULL) {
        fprintf(stderr, "[Client]: Memory allocation failed\n");
        return -1;
    }

    *sock = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock == -1) {
        free(sock);
        error_handling("[Client]: socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if (connect(*sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        fprintf(stderr, "[Client]: connection error\n");
        close(*sock);
        free(sock);
        return -1;
    }

    printf("[Client]: Connected to server at %s:%d\n", ip, port);

    int thr_id = pthread_create(&ttid, NULL, t_function_w, sock);
    if (thr_id != 0) {
        perror("[Client]: Thread creation failed");
        close(*sock);
        free(sock);
        return -1;
    }

    return *sock;
}