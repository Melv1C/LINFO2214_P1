//
// Created by melvyn on 5/10/22.
//

#ifndef PROJET1_CLIENT_H
#define PROJET1_CLIENT_H

#endif //PROJET1_CLIENT_H
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <poll.h>
#include <sys/time.h>
#include <pthread.h>

int SEC = 1000000;
int MAX_SIZE_FILE = 1024*1024;

uint32_t MAX_SIZE_T = 65535;

struct arg_struct {
    int size;
    int socket_desc;
    int *nbre_request;
    int time;
    int rate;
    struct timeval * tlist;
};

int main(int argc, char **argv);

void * send_request(void * arguments);

