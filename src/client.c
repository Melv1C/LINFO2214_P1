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

    INFO("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d",size,rate,time,ip,port);

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
    INFO("----------    CONNECT TO THE SERVER   ----------");

    //---------------------------------------------------------------------------------------------------
    //ECHANGE AVEC LE SERVEUR
    //---------------------------------------------------------------------------------------------------

    struct pollfd arrayPoll[1];
    arrayPoll[0].fd = socket_desc;
    arrayPoll[0].events = POLLIN;

    struct timeval start,now,last_send,last_recv;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);
    gettimeofday(&last_recv, NULL);

    char * client_message;
    client_message = malloc(2*sizeof(uint32_t)+size);

    char* server_message1;
    server_message1 = malloc(sizeof(uint8_t) + sizeof(uint32_t));

    char* server_message2;

    int nbre_request = 0;
    int nbre_respond = 0;

    struct arg_struct * args = (struct arg_struct *) malloc(sizeof(struct arg_struct));
    args->nbre_request = &nbre_request;
    args->size = size;
    args->socket_desc = socket_desc;
    args->time = time;
    args->rate = rate;

    pthread_t threads;
    pthread_create(&(threads),NULL,&send_request,(void *) args);

    while (1) {

        gettimeofday(&now, NULL);
        int poll_count = poll(arrayPoll, 1, 5);

        if (nbre_respond==nbre_request && (now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate){
            INFO("FINISH\n");
            break;
        }else if (((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate)&((now.tv_sec - last_recv.tv_sec) * 1000000 + now.tv_usec - last_recv.tv_usec>3*SEC)) {
            INFO("NOT ALL RESPOND RECEIVED");
            break;
        }

        //SI ON RECOIT UNE RESPOND DU SERVER

        if (poll_count>0){

            if (arrayPoll[0].revents == POLLIN){
                // Receive the server's response:

                memset(server_message1, '\0', sizeof(uint8_t) + sizeof(uint32_t));

                if(read(socket_desc, server_message1, (sizeof(uint8_t) + sizeof(uint32_t))) < 0) {
                    ERROR("Error while receiving server's msg");
                    return -1;
                }

                if(*(uint8_t *) server_message1 == 0 & *(uint32_t *) (server_message1+ sizeof(uint8_t)) > 0){ //ON VERIFIE LE CODE D'ERREUR

                    server_message2 = malloc(*(uint32_t *)  (server_message1+ sizeof(uint8_t)));

                    if(read(socket_desc, server_message2, (*(uint32_t *)  (server_message1+ sizeof(uint8_t)))) < 0){
                        ERROR("Error while receiving server's msg");
                        return -1;
                    }

                    DEBUG("SERVER RESPOND");
                    gettimeofday(&last_recv, NULL);
                    nbre_respond++;
                    //DEBUG("%d",*(uint8_t *)  server_message1); // Code d'erreur
                    DEBUG("size : %d",*(uint32_t *)  (server_message1 + sizeof(uint8_t))); // size
                    DEBUG("matrice : %d",*(uint8_t *)  server_message2); // 1er elem de la matrice

                    free(server_message2);

                } else{
                    //ERROR("RESPOND WITH WRONG ERROR CODE %d",*(uint8_t *)  server_message1);
                }
            }else{
                ERROR("NOT A POLLIN %d",arrayPoll->revents);
            }
        }
    }

    //AFFICHER LES STATS
    INFO("STATS\n\nNombre de request : %d\nNombre de respond : %d\n",nbre_request,nbre_respond);

    free(server_message1);
    free(client_message);
    //CLOSE DE SOCKET
    close(socket_desc);

    return 0;

}

void *send_request(void * arguments){

    struct arg_struct * args = arguments;

    int size = args->size;
    int socket_desc = args->socket_desc;
    int* nbre_request = args->nbre_request;
    int time = args->time;
    int rate = args->rate;

    struct timeval now,start,last_send;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);

    char * client_message;
    client_message = malloc(2*sizeof(uint32_t)+size);

    while(1){
        gettimeofday(&now, NULL);
        if ((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate){
            break;
        }
        //ON ENVOYE LES REQUEST

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
                ERROR("Unable to send message");
                return -1;
            }

            DEBUG("REQUEST OF INDEX %d SEND", index);
            (*(nbre_request))++;
            gettimeofday(&last_send, NULL);
        }
    }
    free(client_message);
    pthread_exit(NULL);
}

