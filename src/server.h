//
// Created by melvyn on 7/10/22.
//

#ifndef PROJET1_SERVER_H
#define PROJET1_SERVER_H

#endif //PROJET1_SERVER_H

#include <stdio.h>
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

int SEC = 1000000;
int MAX_SIZE_KEY = 128*128;

int MAX_CLIENT = 100;

struct arg_struct {
    int client_sock;
    char* client_message;
    uint8_t* files;
    int size;
    int cli;
    struct timeval * timer;
    int * thread_i;
};

struct node
{
    int client;
    int socket;
    char* message;
    struct node *next;
};

struct client_sock_list_node
{
    int client_sock;
    struct client_sock_list_node *next;
};

void push(struct node** head, int client, int socket, char* message);

int main(int argc, char **argv);

void *deal_new_request(void * arguments);


