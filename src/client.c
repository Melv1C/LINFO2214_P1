//
// Created by melvyn on 18/10/22.
//

#include "client.h"
#include "debug.h"
#include "log.h"

pthread_mutex_t lock;

int main(int argc, char **argv) {
    int opt;

    uint32_t size = 128;
    int rate = 1000;
    int time = 10;

    while ((opt = getopt(argc, argv, "k:r:t:")) != -1) {
        switch (opt) {
            case 'k':
                size = (uint32_t) strtol(optarg, NULL, 10);
                if (!IsPowerOfTwo(size)) {
                    ERROR("The size must be a power of 2");
                    return -1;
                }
                break;
            case 'r':
                rate = (int) strtol(optarg, NULL, 10);
                break;
            case 't':
                time = (int) strtol(optarg, NULL, 10);
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

    char *ip = "127.0.0.1";
    int port = 2241;

    ip = strtok(argv[optind], ":");
    port = (int) strtol(strtok(NULL, ":"), NULL, 10);

    INFO("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d", size, rate, time, ip, port);

    size = size * size;

    int nbre_request = 0;
    int nbre_respond = 0;
    int totalrespt = 0;

    struct timeval start, now, last_send;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);

    pthread_t threads[time*rate];
    int nbre_threads = 0;

    while (1) {

        gettimeofday(&now, NULL);

        if ((now.tv_sec - start.tv_sec) * 1000000 + now.tv_usec - start.tv_usec > time * SEC + SEC / rate) {
            INFO("ALL thread created\n");
            break;
        } else if ((now.tv_sec - last_send.tv_sec) * 1000000 + now.tv_usec - last_send.tv_usec > SEC / rate) {

            struct arg_struct * args = (struct arg_struct *) malloc(sizeof(struct arg_struct));
            args->size = size;
            args->time = time;
            args->rate = rate;
            args->ip = ip;
            args->port = port;
            args->nbre_request = &nbre_request;
            args->nbre_respond = &nbre_respond;
            args->totalrespt = &totalrespt;

            DEBUG("New thread");

            pthread_create(&(threads[nbre_threads]),NULL,&send_and_recv,(void *) args);
            nbre_threads++;
            gettimeofday(&last_send, NULL);
        }
    }
    //INFO("WAIT PTHREAD JOIN");
    for (int i = 0; i < nbre_threads; i++) {
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&lock);
    INFO("FINISH");
    //INFO("AvgRespTime = %d µs", totalrespt/nbre_respond);
    //INFO("Nbre of request : %d and respond : %d",nbre_request,nbre_respond);


    FILE *f;
    f = fopen("stat_client.txt", "a");
    fprintf(f,"%d,%d,%d,%d,%d,%d\n",rate,time,(int) sqrt(size),nbre_respond,nbre_request,totalrespt/nbre_respond);
    fclose(f);

}

void *send_and_recv(void * arguments){

    struct arg_struct *args = arguments;

    uint32_t size = args->size;
    int time = args->time;
    int rate = args->rate;
    char * ip = args->ip;
    int port = args->port;
    int * nbre_request = args->nbre_request;
    int * nbre_respond = args->nbre_respond;
    int * totalrespt = args->totalrespt;

    struct timeval send_timer, recv_timer;

    // Create socket:
    int socket_client;
    socket_client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Send connection request to server:
    if (connect(socket_client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ERROR("Unable to connect");
        return -1;
    }

    //Get the request ready to send

    char *client_message;
    client_message = malloc(2 * sizeof(uint32_t) + size);

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
    // Send the request to server:
    if(send(socket_client, client_message, 2*sizeof(uint32_t)+size, 0) < 0){
        ERROR("Unable to send message");
        return -1;
    }

    gettimeofday(&send_timer, NULL);

    pthread_mutex_lock(&lock);
    (*nbre_request)++;
    pthread_mutex_unlock(&lock);

    free(client_message);

    //Wait to recv the respond

    char* server_message1;
    server_message1 = malloc(sizeof(uint8_t) + sizeof(uint32_t));

    memset(server_message1, '\0', sizeof(uint8_t) + sizeof(uint32_t));

    if (read(socket_client, server_message1, (sizeof(uint8_t) + sizeof(uint32_t))) < 0) {
        ERROR("Error while receiving server's msg1");
        return -1;
    }
    //ON VERIFIE LE CODE D'ERREUR ET LA TAILLE
    if (*(uint8_t *) server_message1 == 0 && (uint32_t) * (uint32_t * )(server_message1 + sizeof(uint8_t)) > 0 && (uint32_t) * (uint32_t * )(server_message1 + sizeof(uint8_t)) < (MAX_SIZE_FILE + 9)) {

        char * server_message2;
        server_message2 = malloc(*(uint32_t * )(server_message1 + sizeof(uint8_t)));

        memset(server_message2, '\0', *(uint32_t * )(server_message1 + sizeof(uint8_t)));

        if (read(socket_client, (char *) server_message2, *(uint32_t * )(server_message1 + sizeof(uint8_t))) < 0) {
            ERROR("Error while receiving server's msg2");
            return -1;
        }

        gettimeofday(&recv_timer, NULL);
        pthread_mutex_lock(&lock);
        (*totalrespt) += (recv_timer.tv_sec - send_timer.tv_sec) * SEC + recv_timer.tv_usec - send_timer.tv_usec;

        (*nbre_respond)++;
        pthread_mutex_unlock(&lock);

        DEBUG("SERVER RESPOND 1er elem de la matrice : %d",*(uint8_t *) server_message2);

        free(server_message2);
    }else{
        ERROR("WRONG ERROR CODE OR SIZE");
    }
    free(server_message1);
    close(socket_client);
}

int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}
