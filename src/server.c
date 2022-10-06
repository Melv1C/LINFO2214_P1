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

int main(int argc, char **argv) {

    int opt;

    int n_thread;
    int size;
    int port;

    while ((opt = getopt(argc, argv, "j:s:p:")) != -1) {
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
                return printf("ERREUR\n");

        }
    }

    printf("Argument du server : n_thread: %d, size: %d, port: %d\n",n_thread,size,port);

    // Génerer les fichiers

    srand ( time(NULL) );

    uint8_t * files;
    files = malloc(size*size*1000);
    for (int i=0; i<1000; i++){
        for (int j = 0; j < size*size; j++) {
            int r = rand() % 256;
            files[i*size*size+j] = r;
        }
    }

    // Connection

    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);

    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

    if (client_sock < 0){
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected\n");

    while(1){
        // Receive client's message:
        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
            printf("Couldn't receive\n");
            return -1;
        }

        if (strstr(client_message, "exit")){
            break;
        }

        printf("New msg from client: %s\n", client_message);

        // Respond to client:
        //strcpy(server_message, "This is the server's message.");

        char* p = server_message;

        for (int i = 0; i < size*size; i++) {
            *(uint8_t *) p = files[((int) strtol(client_message,NULL,10))*size*size+i];
            p += sizeof(uint8_t);
        }

        if (send(client_sock, server_message, strlen(server_message), 0) < 0){
            printf("Can't send\n");
            return -1;
        }
    }

    return 0;

}
