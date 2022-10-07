//
// Created by melvyn on 5/10/22.
//

#include "client.h"
#include "debug.h"


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
                ERROR("PROBLEME AVEC LES ARGUMENTS");
                return -1;
        }
    }

    if (optind + 1 != argc) {
        ERROR("Unexpected number of positional arguments");
        return -1;
    }

    char* ip = "127.0.0.1";
    int port = 2241;

    ip = strtok(argv[optind], ":");
    port = (int) strtol(strtok(NULL, ":"), NULL, 10);

    DEBUG("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d",size,rate,time,ip,port);

    size = size*size;

    //---------------------------------------------------------------------------------------------------
    //CREATION DU SOCKET ET CONNECTION
    //---------------------------------------------------------------------------------------------------

    int socket_desc;
    struct sockaddr_in server_addr;

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        ERROR("Unable to create socket");
        return -1;
    }

    DEBUG("Socket created successfully");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        ERROR("Unable to connect");
        return -1;
    }
    DEBUG("Connected with server successfully");

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

    char* server_message;
    server_message = malloc(sizeof(uint8_t) + sizeof(uint32_t) + MAX_SIZE_FILE);

    int nbre_request = 0;
    int nbre_respond = 0;

    while (1) {

        int poll_count = poll(arrayPoll, 1, 5);

        gettimeofday(&now, NULL);

        if ((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate){
            if (nbre_request==nbre_respond){
                DEBUG("FINISH\n");
                break;
            }else if ((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>(time+5)*SEC) {
                ERROR("NOT ALL RESPOND RECEIVED");
                break;
            }
        }

        if (((now.tv_sec - last_send.tv_sec) * 1000000 + now.tv_usec - last_send.tv_usec>SEC/rate)&((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec<time*SEC+SEC/rate)){

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
                ERROR("Unable to send message");
                return -1;
            }

            DEBUG("REQUEST OF INDEX %d SEND", index);
            nbre_request++;
            gettimeofday(&last_send, NULL);

        }

        if (poll_count>0){

            if (arrayPoll[0].revents == POLLIN){
                // Receive the server's response:

                memset(server_message, '\0', sizeof(uint8_t) + sizeof(uint32_t) + MAX_SIZE_FILE);

                if(recv(socket_desc, server_message, sizeof(uint8_t) + sizeof(uint32_t) + MAX_SIZE_FILE, 0) < 0){
                    ERROR("Error while receiving server's msg");
                    return -1;
                }

                DEBUG("SERVER RESPOND");
                nbre_respond++;
                //DEBUG("%d",*(uint8_t *)  server_message); // Code d'erreur
                //DEBUG("%d",*(uint32_t *)  (server_message+ sizeof(uint8_t))); // size
                //DEBUG("%d",*(uint8_t *)  (server_message+ sizeof(uint8_t)+ sizeof(uint32_t))); // 1er elem de la matrice

            }else{
                return -1;
            }
        }
    }

    free(server_message);
    free(client_message);

    // Close the socket:
    close(socket_desc);

    return 0;

}

