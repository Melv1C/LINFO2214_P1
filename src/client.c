//
// Created by melvyn on 18/10/22.
//

#include "client.h"
#include "debug.h"
#include "log.h"

int main(int argc, char **argv){
    int opt;

    uint32_t size = 128;
    int rate = 1000;
    int time = 10;

    while ((opt = getopt(argc, argv, "k:r:t:")) != -1) {
        switch (opt) {
            case 'k':
                size = (uint32_t) strtol(optarg,NULL,10);
                if (!IsPowerOfTwo(size)){
                    ERROR("The size must be a power of 2");
                    return -1;
                }
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

    int nbre_request = 0;
    int nbre_respond = 0;
    struct timeval tlist[rate*time];
    int totalrespt = 0;

    int socket_desc[time*rate];
    fd_set readfds;
    int max_sd;
    int sd;
    struct sockaddr_in server_addr;

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    struct arg_struct * args = (struct arg_struct *) malloc(sizeof(struct arg_struct));
    args->size = size;
    args->time = time;
    args->rate = rate;
    args->tlist = tlist;
    args->server_addr = &server_addr;
    args->nbre_request = &nbre_request;
    args->socket_desc = socket_desc;

    pthread_t threads;
    pthread_create(&(threads),NULL,&send_request,(void *) args);

    char* server_message1;
    server_message1 = malloc(sizeof(uint8_t) + sizeof(uint32_t));

    char* server_message2;

    struct timeval start,now,last_send,last_recv;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);
    gettimeofday(&last_recv, NULL);


    while (1){

        gettimeofday(&now, NULL);

        if (nbre_respond==nbre_request && (now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate){
            INFO("FINISH\n");
            if (nbre_respond == 0){nbre_respond = 1;}
            INFO("AvgRespTime = %d µs", totalrespt/nbre_respond);
            INFO("TotalRespTime = %d µs", totalrespt);
            INFO("Nbre of request and respond : %d",nbre_respond);
            break;
        }else if (((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate)&((now.tv_sec - last_recv.tv_sec) * 1000000 + now.tv_usec - last_recv.tv_usec>3*SEC)) {
            INFO("NOT ALL RESPOND RECEIVED");
            if (nbre_respond == 0){nbre_respond = 1;}
            INFO("AvgRespTime = %d", totalrespt/nbre_respond);
            INFO("Nbre of request : %d and respond : %d",nbre_request,nbre_respond);
            break;
        }

        //clear the socket set
        FD_ZERO(&readfds);

        //add child sockets to set
        for ( int i = 0 ; i < nbre_request ; i++)
        {
            //socket descriptor
            sd = socket_desc[i];

            //if valid socket descriptor then add to read list
            if(sd > 0){
                FD_SET( sd , &readfds);
            }


            //highest file descriptor number, need it for the select function
            if(sd > max_sd){
                max_sd = sd;
            }
        }
        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        struct timeval tv = {0, 1000};
        int activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

        if (activity < 0)
        {
            //ERROR("select error : %d",activity);
            //return -1;
        }else{

            for (int i = 0; i < nbre_request; i++) {
                sd = socket_desc[i];

                if (FD_ISSET(sd, &readfds)) {
                    memset(server_message1, '\0', sizeof(uint8_t) + sizeof(uint32_t));
                    if (read(socket_desc[i], server_message1, (sizeof(uint8_t) + sizeof(uint32_t))) < 0) {
                        ERROR("Error while receiving server's msg");
                        return -1;
                    }

                    if (*(uint8_t *) server_message1 == 0 &&
                        (uint32_t) * (uint32_t * )(server_message1 + sizeof(uint8_t)) > 0 &&
                        (uint32_t) * (uint32_t * )(server_message1 + sizeof(uint8_t)) <
                        (MAX_SIZE_FILE + 9)) { //ON VERIFIE LE CODE D'ERREUR

                        server_message2 = malloc(*(uint32_t * )(server_message1 + sizeof(uint8_t)));

                        memset(server_message2, '\0', *(uint32_t * )(server_message1 + sizeof(uint8_t)));


                        if (read(socket_desc[i], (char *) server_message2,
                                 *(uint32_t * )(server_message1 + sizeof(uint8_t))) < 0) {
                            ERROR("Error while receiving server's msg");
                            return -1;
                        }

                        struct timeval x;
                        gettimeofday(&x, NULL);
                        totalrespt +=
                                (x.tv_sec - tlist[nbre_respond].tv_sec) * SEC + x.tv_usec - tlist[nbre_respond].tv_usec;
                        //INFO("return time: %d µs", (x.tv_sec - tlist[nbre_respond].tv_sec)*SEC +  x.tv_usec - tlist[nbre_respond].tv_usec);



                        DEBUG("SERVER RESPOND");
                        gettimeofday(&last_recv, NULL);
                        nbre_respond++;
                        //DEBUG("%u",*(uint8_t *)  server_message1); // Code d'erreur
                        DEBUG("size : %u", *(uint32_t * )(server_message1 + sizeof(uint8_t))); // size
                        DEBUG("matrice : %u", *(uint8_t *) server_message2); // 1er elem de la matrice

                        //print_matrix((uint8_t *)  server_message2,*(uint32_t *)  (server_message1 + sizeof(uint8_t)));

                        free(server_message2);
                        close(socket_desc[i]);
                        socket_desc[i] = 0;
                    }
                }
            }
        }
    }
}

void *send_request(void * arguments) {

    struct arg_struct *args = arguments;

    int size = args->size;
    int time = args->time;
    int rate = args->rate;
    int *socket_desc = args->socket_desc;
    int *nbre_request = args->nbre_request;
    struct timeval *tlist = args->tlist;
    struct sockaddr_in server_addr = *(args->server_addr);

    struct timeval now, start, last_send;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);

    char *client_message;
    client_message = malloc(2 * sizeof(uint32_t) + size);


    while(1){
        gettimeofday(&now, NULL);
        if ((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec>time*SEC+SEC/rate){
            break;
        }
        //ON ENVOYE LES REQUEST

        if ((now.tv_sec - last_send.tv_sec) * 1000000 + now.tv_usec - last_send.tv_usec>SEC/rate){

            // Create socket:
            socket_desc[*nbre_request] = socket(AF_INET, SOCK_STREAM, 0);

            // Send connection request to server:
            if (connect(socket_desc[*nbre_request], (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                ERROR("Unable to connect");
                return -1;
            }

            memset(client_message, '\0', 2*sizeof(uint32_t)+size);

            char * copy_client_message = client_message;

            uint32_t index = (uint32_t) rand() % 1000;

            * (uint32_t *) copy_client_message = index;

            copy_client_message += sizeof(uint32_t);

            *(uint32_t *) copy_client_message = (uint32_t) sqrt(size);

            copy_client_message += sizeof(uint32_t);

            for (int i = 0; i < size; i++) {
                *(uint8_t *) (copy_client_message+i) = (uint8_t) rand() % 256;
            }
            // Send the message to server:
            if(send(socket_desc[*nbre_request], client_message, 2*sizeof(uint32_t)+size, 0) < 0){
                ERROR("Unable to send message");
                return -1;
            }
            gettimeofday(&(tlist[*nbre_request]),NULL);
            DEBUG("REQUEST OF INDEX %d SEND", index);
            (*(nbre_request))++;
            gettimeofday(&last_send, NULL);
        }
    }
    free(client_message);
    pthread_exit(NULL);
}


int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}
