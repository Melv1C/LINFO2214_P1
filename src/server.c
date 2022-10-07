//
// Created by melvyn on 5/10/22.
//


#include "server.h"

int main(int argc, char **argv) {

    int opt;

    int n_thread = 4;
    uint32_t size = 1024;
    int port = 2241;

    while ((opt = getopt(argc, argv, "j:s:p:")) != -1) {
        switch (opt) {
            case 'j':
                n_thread = (int) strtol(optarg,NULL,10);
                break;
            case 's':
                size = (uint32_t) strtol(optarg,NULL,10);
                break;
            case 'p':
                port = (int) strtol(optarg,NULL,10);
                break;
            default:
                return printf("ERREUR\n");

        }
    }

    printf("Argument du server : n_thread: %d, size: %d, port: %d\n",n_thread,size,port);

    size = size*size;//Plus pratique

    // Génerer les fichiers

    //srand ( time(NULL) );

    uint8_t * files;
    files = malloc(size*1000);
    for (int i=0; i<1000; i++){
        for (int j = 0; j < size; j++) {
            int r = rand() % 256;
            files[i*size+j] = r;
        }
    }

    // Connection

    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;

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

    struct pollfd arrayPoll[1];
    arrayPoll[0].fd = client_sock;
    arrayPoll[0].events = POLLIN;

    char* server_message;
    server_message = malloc(sizeof(uint8_t) + sizeof(uint32_t) + size);

    char* client_message;
    client_message = malloc(MAX_SIZE_KEY+ 2*sizeof(uint32_t));

    while(1){

        int poll_count = poll(arrayPoll, 1, 5);

        if (poll_count>0){
            if (arrayPoll[0].revents == POLLIN){

                uint8_t * key;
                uint32_t index;
                uint32_t size_key;

                memset(client_message, '\0', MAX_SIZE_KEY+ 2*sizeof(uint32_t));

                if (recv(client_sock, client_message, MAX_SIZE_KEY+ 2*sizeof(uint32_t), 0) < 0){
                    printf("Couldn't receive\n");
                    return -1;
                }

                if (strstr(client_message, "exit")){
                    printf("EXIT\n");
                    break;
                }

                index = *(uint32_t *)  client_message;
                size_key = *(uint32_t *) (client_message+sizeof(uint32_t));

                key = malloc(size_key);
                for (int i = 0; i < size_key; i++) {
                    *(uint8_t *) (key+i*sizeof(uint8_t)) = *(uint8_t *)  (client_message + 2*sizeof(uint32_t) + i*sizeof(uint8_t));
                }
                printf("NEW REQUEST OF INDEX: %d\n", index);

                memset(server_message, '\0', sizeof(uint8_t) + sizeof(uint32_t) + size);

                char* p = server_message;

                *(uint8_t *) p = (uint8_t) 0;
                p += sizeof(uint8_t);

                *(uint32_t *) p = size;
                p += sizeof(uint32_t);


                for (int i = 0; i < size; i++) {
                    *(uint8_t *) p = files[index+i];
                    p += sizeof(uint8_t);
                }

                if (send(client_sock, server_message, sizeof(uint8_t) + sizeof(uint32_t) + size, 0) < 0){
                    printf("Can't send\n");
                    return -1;
                }

                printf("SEND RESPOND\n");

                free(key);

            }
        }
    }

    free(server_message);
    free(client_message);

    return 0;

}
