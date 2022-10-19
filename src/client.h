//
// Created by melvyn on 18/10/22.
//

#ifndef PROJET1_NEW_CLIENT_H
#define PROJET1_NEW_CLIENT_H

#endif //PROJET1_NEW_CLIENT_H


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

struct arg_struct {
    uint32_t size;
    int time;
    int rate;
    char * ip;
    int port;
    int * nbre_request;
    int * nbre_respond;
    int * totalrespt;
};

int SEC = 1000000;
int MAX_SIZE_FILE = 1024*1024;
uint32_t MAX_SIZE_T = 65535;

int main(int argc, char **argv);

void *send_and_recv(void * arguments);

int IsPowerOfTwo(uint32_t x);