#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

extern int indata[6];

void *t_function_play(void *data);
void *t_function_disp(void *data);
void *t_function_main(void *data);

#endif //SERVER_H
