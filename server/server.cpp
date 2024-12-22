#include "server.h"
#include <iostream>

char pmp[64][32];

/**************************************************************
 * socket server
 * socket read & write
 * read: sensor data
 * write: hp, cooltime
***************************************************************/

void *t_function_disp(void *data) {
    int client_fd = *((int *)data);
    free(data);

    while (true) {
        ssize_t sz = recv(client_fd, pmp, sizeof(pmp), 0);
        std::cout << sz << '\n';
        usleep(100);
    }
}

int indata[6] = {-1, -1, -1, -1, -1, -1};

void *t_function_play(void *data) {
    int client_fd = *((int *)data);
    free(data);

    int buffer[2] = {};

    while (true) {
        memset(buffer, 0x00, sizeof(buffer));
        recv(client_fd, buffer, sizeof(buffer), 0);
        indata[buffer[0]] = buffer[1];
    }
}

void *t_function_main(void *data) {
    int server_fd;
    struct sockaddr_in server_addr;\

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "[Server]: Can't open stream socket\n");
        exit(1);
    }

    memset(&server_addr, 0x00, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(9000);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "[Server]: Can't bind local address.\n");
        close(server_fd);
        exit(2);
    }

    if (listen(server_fd, 5) < 0) {
        fprintf(stderr, "[Server]: Can't listen for connections.\n");
        close(server_fd);
        exit(3);
    }

    printf("[Server]: Waiting for connection requests.\n");

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        char temp[INET_ADDRSTRLEN];

        int *client_fd = (int *)malloc(sizeof(int));
        if (!client_fd) {
            fprintf(stderr, "[Server]: Memory allocation failed\n");
            continue;
        }

        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (*client_fd < 0) {
            free(client_fd);
            continue;
        }

        inet_ntop(AF_INET, &client_addr.sin_addr, temp, sizeof(temp));
        printf("[Server]: %s client connected (FD: %d)\n", temp, *client_fd);

        if (strcmp(temp, "192.168.1.8") == 0) {
            indata[4] = *client_fd;
        } 
        else if (strcmp(temp, "192.168.1.5") == 0) {
            indata[5] = *client_fd;
        } 
        else {
            printf("[Server]: Invalid Client\n");
            close(*client_fd);
            free(client_fd);
            continue;
        }

        pthread_t tid;
        int thr_id;
        if (*(int *)data == 0)
            thr_id = pthread_create(&tid, NULL, t_function_disp, client_fd);
        else
            thr_id = pthread_create(&tid, NULL, t_function_play, client_fd);

        if (thr_id != 0) {
            fprintf(stderr, "[Server]: Error creating thread\n");
            close(*client_fd);
            free(client_fd);
        }
        pthread_detach(tid);
    }

    close(server_fd);
}