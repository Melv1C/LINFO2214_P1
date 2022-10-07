//
// Created by melvyn on 5/10/22.
//

#include "client.h"


int main(int argc, char **argv) {

    //---------------------------------------------------------------------------------------------------
    //PARSING DES ARGUMENTS
    //---------------------------------------------------------------------------------------------------

    int opt;

    uint32_t size = 128;
    int rate = 1000;
    int time = 10;

    while ((opt = getopt(argc, argv, "k:r:t:")) != -1) {
        switch (opt) {
            case 'k':
                size = (uint32_t) strtol(optarg,NULL,10);
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

    char* ip = "127.0.0.1";
    int port = 2241;

    ip = strtok(argv[optind], ":");
    port = (int) strtol(strtok(NULL, ":"), NULL, 10);

    printf("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d\n",size,rate,time,ip,port);

    size = size*size;

    //---------------------------------------------------------------------------------------------------
    //CREATION DU SOCKET ET CONNECTION
    //---------------------------------------------------------------------------------------------------

    int socket_desc;
    struct sockaddr_in server_addr;

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

    //---------------------------------------------------------------------------------------------------
    //ECHANGE AVEC LE SERVEUR
    //---------------------------------------------------------------------------------------------------

    struct pollfd arrayPoll[1];
    arrayPoll[0].fd = socket_desc;
    arrayPoll[0].events = POLLIN;

    struct timeval start,now,last_send;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);

    char * client_message = malloc(2*sizeof(uint32_t)+size);
    printf("%d %d\n", sizeof(client_message),2*sizeof(uint32_t)+size);

    char* server_message;
    server_message = malloc(sizeof(uint8_t) + sizeof(uint32_t) + MAX_SIZE_FILE);

    while (1) {

        int poll_count = poll(arrayPoll, 1, 5);
        //printf("ICI%d\n",poll_count);

        gettimeofday(&now, NULL);

        if ((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate){
            printf("FINISH\n");
            break;
        }

        if ((now.tv_sec - last_send.tv_sec) * 1000000 + now.tv_usec - last_send.tv_usec>SEC/rate){

            memset(client_message, '\0', 2*sizeof(uint32_t)+size);

            char * copy_client_message = client_message;

            uint32_t index = (uint32_t) rand() % 1000;

            * (uint32_t *) copy_client_message = index;

            copy_client_message += sizeof(uint32_t);

            *(uint32_t *) copy_client_message = size;

            copy_client_message += sizeof(uint32_t);

            for (int i = 0; i < size; i++) {
                *(uint8_t *) (copy_client_message+i) = (uint8_t) rand() % 256;

            }
            // Send the message to server:
            if(send(socket_desc, client_message, 2*sizeof(uint32_t)+size, 0) < 0){
                printf("Unable to send message\n");
                return -1;
            }

            printf("REQUEST OF INDEX %d SEND\n", index);
            gettimeofday(&last_send, NULL);

        }

        if (poll_count>0){

            if (arrayPoll[0].revents == POLLIN){
                // Receive the server's response:

                memset(server_message, '\0', sizeof(uint8_t) + sizeof(uint32_t) + MAX_SIZE_FILE);

                if(recv(socket_desc, server_message, sizeof(uint8_t) + sizeof(uint32_t) + MAX_SIZE_FILE, 0) < 0){
                    printf("Error while receiving server's msg\n");
                    return -1;
                }

                printf("SERVER RESPOND\n");
                //printf("%u\n",*(uint8_t *)  server_message); // Code d'erreur
                //printf("%u\n",*(uint32_t *)  (server_message+ sizeof(uint8_t))); // size
                //printf("%u\n",*(uint8_t *)  (server_message+ sizeof(uint8_t)+ sizeof(uint32_t))); // 1er elem de la matrice

            }else{
                return -1;
            }
        }
    }

    free(server_message);

    char client_message_exit[20];

    memset(client_message_exit,'\0',sizeof(client_message_exit));

    strcpy(client_message_exit, "exit");

    if(send(socket_desc, client_message_exit, strlen(client_message_exit), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    // Close the socket:
    close(socket_desc);

    return 0;

}

