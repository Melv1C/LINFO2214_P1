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
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void func(int sockfd);

int main(int argc, char **argv);

