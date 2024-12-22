#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

extern pthread_t ttid;
extern int indata[3];

void error_handling(char *message);
int sock_init(char *ip, int port);
void *t_function_w(void *data);

#endif
