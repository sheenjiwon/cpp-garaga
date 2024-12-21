//
// Created by Jiwon Sheen on 2024-12-06
//

#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void error_handling(char *message);
int sock_init(char *ip, int port);

#endif //CLIENT_H
