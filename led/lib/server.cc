#include "server.h"
#include <iostream>

char pmp[64][32];

int clid;

void *t_function_disp(void *data) {
    int client_fd = *((int *)data);
    free(data); // 동적으로 할당한 메모리 해제

    while (true) {
        ssize_t sz = recv(client_fd, pmp, sizeof(pmp), 0);
        usleep(110);
    }
}

int indata[6];

void *t_function_play(void *data) {
    int client_fd = *((int *)data);
    free(data); // 동적으로 할당한 메모리 해제

    int buffer[2] = {};

    while (true) {
        memset(buffer, 0x00, sizeof(buffer));
        recv(client_fd, buffer, sizeof(buffer), 0);
        indata[buffer[0]] = buffer[1];
    }
}

void *t_function_main(void *data) {
    int server_fd; // server의 file descriptor
    struct sockaddr_in server_addr; // server 정보를 담는 struct

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // server에서 socket 구축, 오류 시, stderr에 오류 메시지 출력 후 종료
        fprintf(stderr, "[Server]: Can't open stream socket\n");
        exit(1);
    }

    memset(&server_addr, 0x00, sizeof(server_addr)); // server_addr을 초기화

    server_addr.sin_family = AF_INET; // UDP, TCP를 위한 소켓 세팅
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //
    server_addr.sin_port = htons(9000);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Server: Can't bind local address.\n");
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
        pthread_detach(tid); // 스레드 독립 실행
    }

    close(server_fd);
}
#if 0
int main(int argc, char *argv[]) {
    int option = 0, option2 = 1;
    pthread_t tid, tid2;
    int thr_id = pthread_create(&tid, NULL, t_function_main, &option);

    pthread_join(tid, NULL);

    return 0;
}
#endif