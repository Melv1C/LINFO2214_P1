//
// Created by melvyn on 5/10/22.
//

#include "client.h"

void func(int sockfd) {
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

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
    port = (int) strtol(strtok(argv[optind], ":"), NULL, 10);

    printf("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d\n",size,rate,time,ip,port);

    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
}
