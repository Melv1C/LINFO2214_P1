//
// Created by melvyn on 5/10/22.
//


#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void func(int connfd)
{
    char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        // copy server message in the buffer
        while ((buff[n++] = getchar()) != '\n')
            ;

        // and send that buffer to client
        write(connfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}


int main(int argc, char **argv) {

    int opt;

    int n_thread;
    int size;
    int port;

    while ((opt = getopt(argc, argv, "j:s:p")) != -1) {
        switch (opt) {
            case 'j':
                n_thread = (int) strtol(optarg,NULL,10);
                break;
            case 's':
                size = (int) strtol(optarg,NULL,10);
                break;
            case 'p':
                port = (int) strtol(optarg,NULL,10);
                break;
            default:
                return printf("ERREUR");

        }
    }

    // Génerer les fichiers





    // Connection

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd == -1)
    {
        printf("Failed to create the socket.");
        return -1;
    }else{
        printf("Socket ok \n");
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htonl(port);

    if (bind(sockfd, (SA*)&servaddr, sizeof(servaddr) != 0))
    {
        printf("erreur socket : bind");
        return -1;
    }else{
        printf("Bind ok \n");
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 2)) != 0) {
        printf("Listen failed...\n");
        return -1;
    }
    else{
        printf("Server listening..\n");
    }

    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);

}
