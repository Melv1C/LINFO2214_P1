//
// Created by melvyn on 5/10/22.
//

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()

int main(int argc, char **argv) {

    int opt;

    int size;
    int rate;
    int time;

    while ((opt = getopt(argc, argv, "k:r:t:")) != -1) {
        switch (opt) {
            case 'k':
                size = (int) strtol(optarg,NULL,10);
                break;
            case 'r':
                rate = (int) strtol(optarg,NULL,10);
                break;
            case 't':
                time = (int) strtol(optarg,NULL,10);
                break;
            default:
                return printf("ERREUR\n");
        }
    }

    if (optind + 1 != argc) {
        printf("Unexpected number of positional arguments\n");
        return -1;
    }

    char* ip;
    int port;

    ip = strtok(argv[optind], ":");
    port = (int) strtol(strtok(NULL, ":"), NULL, 10);

    printf("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d\n",size,rate,time,ip,port);

    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    for (int i = 0; i < 10; i++) {
        // Get input from the user:
        //printf("Enter message: ");
        //gets(client_message);

        sprintf(client_message, "%d", rand() % 255);

        // Send the message to server:
        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }

        // Receive the server's response:
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }

        printf("Server's response received\n");

        //printf("%u\n",*(uint8_t *)  server_message); // 1er elem de la matrice
        //printf("%u\n",*(uint8_t *)  (server_message + sizeof(uint8_t))); // 2eme elem de la matrice

    }

    strcpy(client_message, "exit");

    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    // Close the socket:
    close(socket_desc);

    return 0;

}
