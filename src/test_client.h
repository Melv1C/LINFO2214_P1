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
#include <stdint.h>

#define ARRAY_TYPE uint32_t

int MAX_VALUE_ARRAY_TYPE = UINT32_MAX;

int n_files = 1000;

int SEC = 1000000;
uint32_t MAX_SIZE_T = 65536;

int getts();

int main(int argc, char **argv);

void* rcv(void* r);

int IsPowerOfTwo(uint32_t x);