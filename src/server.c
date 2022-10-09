//
// Created by melvyn on 5/10/22.
//


#include "server.h"
#include "debug.h"

void push(struct node ** head, int client, int socket, char* message) {
    struct node * current = *head;

    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = (struct node *) malloc(sizeof(struct node));
    current->next->client = client;
    current->next->socket = socket;
    current->next->message = message;
    current->next->next = NULL;
}

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

    size = size*size;   //Plus pratique

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
    int id_max = 0;
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

    pthread_t threads[n_thread];
    int free_threads[n_thread];
    for (int i = 0; i < n_thread; i++) {
        free_threads[i] = 1;
    }

    int nbre_request = 0;

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

    struct node * requests = NULL;

    while(1){

        int poll_count_server = poll(arrayPoll_server, 1, 5);
        int poll_count_client = poll(arrayPoll_client, MAX_CLIENT, 5);

        gettimeofday(&now, NULL);

        if ((now.tv_sec - last_send.tv_sec) * 1000000 + now.tv_usec - last_send.tv_usec>5*SEC  & requests==NULL){
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

                int id = 0;

                while(client_sock[id]!=0){
                    id++;
                }

                if (id_max<id+1){
                    id_max = id+1;
                }

                DEBUG("NEW CLIENT CONNECTED WITH ID %d",id);

                client_sock[id] = new_sock;
                arrayPoll_client[id].fd = client_sock[id];
            }
        }
        if (poll_count_client>0){
            for (int cli = 0; cli < id_max; cli++) {
                if (arrayPoll_client[cli].revents == POLLIN) {

                    char* client_message;
                    client_message = malloc(MAX_SIZE_KEY+ 2*sizeof(uint32_t));

                    memset(client_message, '\0', MAX_SIZE_KEY + 2 * sizeof(uint32_t));
                    if (recv(client_sock[cli], client_message, MAX_SIZE_KEY + 2 * sizeof(uint32_t), 0) < 0) {
                        ERROR("Couldn't receive");
                        return -1;
                    }

                    if (strstr(client_message, "EXIT")){
                        DEBUG("CLIENT %d CLOSE IS CONNECTION",cli);
                        close(client_sock[cli]);
                        client_sock[cli]=0;
                        arrayPoll_client[cli].fd = 0;
                    }else{
                        DEBUG("NEW REQUEST FROM %d %d", cli,*(uint32_t *) client_message);
                        nbre_request++;
                        if (requests == NULL){
                            requests = (struct node *) malloc(sizeof(struct node));
                            requests->client = cli;
                            requests->socket = client_sock[cli];
                            requests->message = client_message;
                            requests->next = NULL;
                        }else{
                            push(&requests,cli,client_sock[cli],client_message);
                        }
                    }
                    free(client_message);
                }

            }
        }
        if (requests != NULL) {

            for (int i = 0; i < n_thread; i++) {
                if (free_threads[i]==1){
                    struct arg_struct * args = (struct arg_struct *) malloc(sizeof(struct arg_struct));
                    args->client_message = requests->message;
                    args->client_sock = requests->socket;
                    args->files = files;
                    args->size = size;
                    args->cli = requests->client;
                    args->timer = &last_send;
                    args->thread_i = &free_threads[i];

                    DEBUG("REQUEST REMOVE");
                    struct node * next_request = requests->next;
                    free(requests);
                    requests = next_request;

                    pthread_create(&(threads[i]),NULL,&deal_new_request,(void *) args);
                    free_threads[i] = 0;

                    break;

                }
            }
        }
    }

    DEBUG("Nombre de request : %d",nbre_request);

    free(files);
    return 0;

}

void *deal_new_request(void * arguments){

    struct arg_struct * args = arguments;

    char * client_message = args->client_message;
    int client_sock = args->client_sock;
    uint8_t* files = args->files;
    int size = args->size;
    int cli = args->cli;
    struct timeval * timer = args->timer;
    int * thread_i = args->thread_i;

    sleep(1);

    char* server_message;
    server_message = malloc(sizeof(uint8_t) + sizeof(uint32_t) + size);

    uint8_t *key;
    uint32_t index;
    uint32_t size_key;

    index = *(uint32_t *) client_message;
    size_key = *(uint32_t * )(client_message + sizeof(uint32_t));

    DEBUG("%d %d %d",index,size_key,*(uint8_t * )(client_message + 2*sizeof(uint32_t)));

    key = malloc(size_key);

    for (int i = 0; i < size_key; i++) {
        *(uint8_t * )(key + i * sizeof(uint8_t)) = *(uint8_t * )(
                client_message + 2 * sizeof(uint32_t) + i * sizeof(uint8_t));
    }
    //DEBUG("%d",*(uint32_t *) client_message);
    DEBUG("DEAL REQUEST FROM %d OF INDEX: %d", cli, index);

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

    if (send(client_sock, server_message, sizeof(uint8_t) + sizeof(uint32_t) + size, 0) < 0) {
        ERROR("Can't send");
        return -1;
    }

    *thread_i = 1;

    free(key);
    free(server_message);
    free(args->client_message);
    free(args);
    gettimeofday(timer, NULL);
}



