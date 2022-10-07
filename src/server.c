//
// Created by melvyn on 5/10/22.
//


#include "server.h"
#include "debug.h"

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
                ERROR("PROBLEME AVEC LES ARGUMENTS");
                return -1;

        }
    }

    DEBUG("Argument du server : n_thread: %d, size: %d, port: %d\n",n_thread,size,port);

    size = size*size;//Plus pratique

    // Génerer les fichiers

    uint8_t * files;
    files = malloc(size*1000);
    for (int i=0; i<1000; i++){
        for (int j = 0; j < size; j++) {
            int r = rand() % 256;
            files[i*size+j] = r;
        }
    }

    // Connection

    int socket_desc;
    int client_sock[MAX_CLIENT];
    int client_size;
    int new_sock;
    int nbre_client = 0;
    struct sockaddr_in server_addr, client_addr;

    //init all socket to 0
    for (int i = 0; i < MAX_CLIENT; i++) {
        client_sock[i] = 0;
    }

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        ERROR("Error while creating socket");
        return -1;
    }
    DEBUG("Socket created successfully");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        ERROR("Couldn't bind to the port");
        return -1;
    }
    DEBUG("Done with binding");

    // Listen for clients:
    if(listen(socket_desc, 5) < 0){
        ERROR("Error while listening");
        return -1;
    }
    DEBUG("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);


    struct timeval start,now,last_send;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);

    struct pollfd arrayPoll_server[1];
    arrayPoll_server[0].fd = socket_desc;
        arrayPoll_server[0].events = POLLIN;

    struct pollfd arrayPoll_client[MAX_CLIENT];
    for (int i = 0; i < MAX_CLIENT; i++) {
        arrayPoll_client[i].fd = 0;
        arrayPoll_client[i].events = POLLIN;
    }

    char* server_message;
    server_message = malloc(sizeof(uint8_t) + sizeof(uint32_t) + size);

    char* client_message;
    client_message = malloc(MAX_SIZE_KEY+ 2*sizeof(uint32_t));

    while(1){

        int poll_count_server = poll(arrayPoll_server, 1, 5);
        int poll_count_client = poll(arrayPoll_client, MAX_CLIENT, 5);

        gettimeofday(&now, NULL);

        if ((now.tv_sec - last_send.tv_sec) * 1000000 + now.tv_usec - last_send.tv_usec>5*SEC){
            DEBUG("PLUS D'ACTIVITE SUR LE RESEAU\nFERMETURE DU RESEAU");
            break;
        }

        if (poll_count_server>0) {
            if (arrayPoll_server[0].revents == POLLIN) {

                new_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);

                if (new_sock < 0) {
                    ERROR("Can't accept");
                    return -1;
                }

                DEBUG("Client connected %d",nbre_client+1);


                client_sock[nbre_client] = new_sock;
                arrayPoll_client[nbre_client].fd = client_sock[nbre_client];
                nbre_client++;
            }
        }
        if (poll_count_client>0){
            for (int i = 0; i < nbre_client; i++) {
                if (arrayPoll_client[i].revents == POLLIN) {

                    uint8_t *key;
                    uint32_t index;
                    uint32_t size_key;

                    memset(client_message, '\0', MAX_SIZE_KEY + 2 * sizeof(uint32_t));
                    if (recv(client_sock[i], client_message, MAX_SIZE_KEY + 2 * sizeof(uint32_t), 0) < 0) {
                        ERROR("Couldn't receive");
                        return -1;
                    }

                    index = *(uint32_t *) client_message;
                    size_key = *(uint32_t * )(client_message + sizeof(uint32_t));

                    key = malloc(size_key);
                    for (int i = 0; i < size_key; i++) {
                        *(uint8_t * )(key + i * sizeof(uint8_t)) = *(uint8_t * )(
                                client_message + 2 * sizeof(uint32_t) + i * sizeof(uint8_t));
                    }
                    DEBUG("NEW REQUEST FROM %d OF INDEX: %d", i+1, index);

                    memset(server_message, '\0', sizeof(uint8_t) + sizeof(uint32_t) + size);

                    char *p = server_message;

                    *(uint8_t *) p = (uint8_t) 0;
                    p += sizeof(uint8_t);

                    *(uint32_t *) p = size;
                    p += sizeof(uint32_t);


                    for (int i = 0; i < size; i++) {
                        *(uint8_t *) p = files[index + i];
                        p += sizeof(uint8_t);
                    }

                    if (send(client_sock[i], server_message, sizeof(uint8_t) + sizeof(uint32_t) + size, 0) < 0) {
                        ERROR("Can't send");
                        return -1;
                    }

                    DEBUG("SEND RESPOND TO %d",i+1);

                    gettimeofday(&last_send, NULL);

                    free(key);
                }

            }
        }
    }

    free(server_message);
    free(client_message);
    free(files);
    return 0;

}
