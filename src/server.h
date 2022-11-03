//
// Created by melvyn on 7/10/22.
//

#ifndef PROJET1_SERVER_H
#define PROJET1_SERVER_H

#endif //PROJET1_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>

#define ARRAY_TYPE uint32_t

int MAX_VALUE_ARRAY_TYPE = UINT32_MAX;

int n_files = 1000;

int SEC = 1000000;

int getts();

int main(int argc, char **argv);

int connection_handler(int socket_desc);

int IsPowerOfTwo(uint32_t x);


