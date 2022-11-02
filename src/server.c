//
// Created by melvyn on 5/10/22.
//


#include "server.h"
#include "debug.h"

void push(struct node ** head, int client, int socket, char* client_message1, char* client_message2) {
    struct node * current = *head;

    int size = 1;

    while (current->next != NULL) {
        current = current->next;
        size++;
    }

    /* now we can add a new variable */
    current->next = (struct node *) malloc(sizeof(struct node));
    current->next->client = client;
    current->next->socket = socket;
    current->next->message = client_message2;
    current->next->index = *(uint32_t *) client_message1;
    current->next->size_key = *(uint32_t *) (client_message1+ sizeof(uint32_t));
    current->next->next = NULL;

}

int main(int argc, char **argv) {

    //---------------------------------------------------------------------------------------------------
    //PARSING DES ARGUMENTS
    //---------------------------------------------------------------------------------------------------

    int opt;

    int n_thread = 4;
    uint32_t size = 1024;
    int port = 2241;

    int nerror = 0;

    while ((opt = getopt(argc, argv, "j:s:p:")) != -1) {
        switch (opt) {
            case 'j':
                n_thread = (int) strtol(optarg,NULL,10);
                break;
            case 's':
                size = (uint32_t) strtol(optarg,NULL,10);
                if (!IsPowerOfTwo(size)){
                    ERROR("The size must be a power of 2");
                    return -1;
                }
                break;
            case 'p':
                port = (int) strtol(optarg,NULL,10);
                break;
            default:
                ERROR("PROBLEME AVEC LES ARGUMENTS");
                //nerror+=1;
                return -1;

        }
    }
    size = size*size;   //Plus pratique

    //---------------------------------------------------------------------------------------------------
    //GENERATION DES FICHIERS
    //---------------------------------------------------------------------------------------------------


    uint8_t * files;
    files = malloc(size*1000);
    for (int i=0; i<1000; i++){
        for (int j = 0; j < size; j++) {
            int r = rand() % 256;
            files[i*size+j] = r;
        }
    }

    //---------------------------------------------------------------------------------------------------
    //CREATION DU SOCKET ET ECOUTE
    //---------------------------------------------------------------------------------------------------


    //set of socket descriptors
    fd_set readfds;
    int socket_desc;
    int client_sock[MAX_CLIENT];
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        client_sock[i] = 0;
    }

    int max_sd;
    int sd;
    int client_size;
    int new_sock;
    struct sockaddr_in server_addr, client_addr;


    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        ERROR("Error while creating socket");
        //nerror+=1;
        return -1;
    }

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        ERROR("Couldn't bind to the port");
        //nerror+=1;
        return -1;
    }

    // Listen for clients:
    if(listen(socket_desc, 5) < 0){
        ERROR("Error while listening");
        //nerror+=1;
        return -1;
    }

    client_size = sizeof(client_addr);

    //THREADS INIT

    pthread_t threads[n_thread];
    int free_threads[n_thread];
    for (int i = 0; i < n_thread; i++) {
        free_threads[i] = 1;
    }

    // STATS
    int nreq = 0;
    int mreq = 0;
    int * max_req = & mreq;
    int * nbre_request = &nreq;
    int nbre_client = 0;
    int size_buffer = 0;
    int size_max_buffer = 0;


    //---------------------------------------------------------------------------------------------------
    //ECHANGE AVEC LES CLIENTS
    //---------------------------------------------------------------------------------------------------

    struct timeval start,now,last_send;
    gettimeofday(&start, NULL);
    gettimeofday(&last_send, NULL);

    struct node * requests = NULL; //LINKEDLIST REQUEST

    while(1){

        gettimeofday(&now, NULL);

        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(socket_desc, &readfds);
        max_sd = socket_desc;

        //add child sockets to set
        for ( int i = 0 ; i < MAX_CLIENT ; i++)
        {
            //socket descriptor
            sd = client_sock[i];

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
            ERROR("select error");
            //nerror+=1;
            return -1;
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(socket_desc, &readfds))
        {
            new_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);
            if (new_sock < 0) {
                ERROR("Can't accept");
                //nerror+=1;
                return -1;
            }

            //add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                //if position is empty
                if( client_sock[i] == 0 )
                {
                    client_sock[i] = new_sock;
                    nbre_client++;
                    break;
                }
            }
        }
        //else its some IO operation on some other socket
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            sd = client_sock[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message

                char* client_message1;
                client_message1 = malloc(2*sizeof(uint32_t));

                int valread;
                if ((valread = read( sd , client_message1, 2*sizeof(uint32_t))) < 0)
                {
                    ERROR("Read");
                }
                //Echo back the message that came in
                else if (*(uint32_t *) (client_message1+ sizeof(uint32_t))>0)
                {
                    char* client_message2;
                    client_message2 = malloc(*(uint32_t *) (client_message1+ sizeof(uint32_t)));

                    if (read(sd,client_message2,*(uint32_t *) (client_message1+ sizeof(uint32_t)))<0){
                        ERROR("RECV");
                        //nerror+=1;
                        return -1;
                    }

                    *nbre_request += 1;
                    if (*nbre_request > *max_req){
                        *max_req = *nbre_request;
                    }
                    
                    if (requests == NULL){
                        requests = (struct node *) malloc(sizeof(struct node));
                        requests->client = i;
                        requests->socket = client_sock[i];
                        requests->message = client_message2;
                        requests->index = *(uint32_t *) client_message1;
                        requests->size_key = *(uint32_t *) (client_message1+ sizeof(uint32_t));
                        requests->next = NULL;
                    }else{
                        push(&requests,i,client_sock[i],client_message1,client_message2);
                    }

                    size_buffer++;
                    if(size_buffer>size_max_buffer){
                        size_max_buffer = size_buffer;
                    }

                    client_sock[i] = 0;
                    free(client_message1);

                }else{
                    ERROR("Wrong size :%d",*(uint32_t *) (client_message1+ sizeof(uint32_t)));
                    nerror++;
                }
            }
        }
        // GERER LES REQUEST

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
                    args->thread_i = &(free_threads[i]);
                    args->size_key = requests->size_key;
                    args->index = requests->index;
                    args->nbre_request = nbre_request;
                    args->max_req = max_req;

                    struct node * next_request = requests->next;
                    free(requests);
                    requests = next_request;

                    size_buffer--;

                    free_threads[i] = 0;
                    pthread_create(&(threads[i]),NULL,&deal_new_request,(void *) args);
                    break;
                }
            }
        }
    }
}

void *deal_new_request(void * arguments){

    struct arg_struct * args = arguments;

    char * client_message = args->client_message;
    int client_sock = args->client_sock;
    uint8_t* files = args->files;
    uint32_t size = args->size;
    int cli = args->cli;
    struct timeval * timer = args->timer;
    int * thread_i = args->thread_i;
    int * nbre_request = args->nbre_request;
    int * max_req = args->max_req;
    if (client_sock>0){
        char* server_message;
        server_message = malloc(sizeof(uint8_t) + sizeof(uint32_t) + size);

        uint8_t *key;
        uint32_t index;
        uint32_t size_key;

        index = args->index;
        size_key = args->size_key;

        size_key = size_key*size_key;

        if (size<size_key || size%size_key != 0){
            ERROR("sizekey error");
            *thread_i = 1;
            free(args->client_message);
            free(args);
            *nbre_request -= 1;
            
            return -1;
        }

        key = malloc(size_key);

        for (int i = 0; i < size_key; i++) {
            *(uint8_t * )(key + i * sizeof(uint8_t)) = *(uint8_t * )(client_message+ i * sizeof(uint8_t));
        }

        memset(server_message, '\0', sizeof(uint8_t) + sizeof(uint32_t) + size);

        char *p = server_message;

        *(uint8_t *) p = (uint8_t) 0;
        p += sizeof(uint8_t);

        *(uint32_t *) p = (uint32_t) size;
        p += sizeof(uint32_t);

        uint32_t hafsize, hafkeysize;
        hafsize = (uint32_t) sqrt(size);
        hafkeysize = (uint32_t) sqrt(size_key);   //encrypt takes sizes such as the matrix is size*size

        encrypt(&key,hafkeysize,index,&p,files,hafsize);

        int error;
        if (error = send(client_sock, server_message, sizeof(uint8_t) + sizeof(uint32_t) + size, 0) < 0) {
            ERROR("Can't send %d",error);
            //nerror+=1;
            return -1;
        }
        *nbre_request -= 1;

        free(key);
        free(server_message);
    }

    close( client_sock );
    client_sock = 0;

    *thread_i = 1;
    free(args->client_message);
    free(args);
    gettimeofday(timer, NULL);
}
void encrypt(uint8_t** addr_key,uint32_t size_key,uint32_t index,char** server_message,uint8_t* files,uint32_t size) {

    char* copy_server_message = *server_message;
    uint8_t * file = files + sizeof(uint8_t)*index;

    uint8_t* key = *addr_key;

    for (int i = 0; i < size*size; i++) {
        uint8_t val = 0;
        int i_file = i%size;
        int i_key = i/size%size_key;
        int j_file = i/size/size_key;

        for (int j = 0; j < size_key; j++) {
            val += (uint8_t) (file[i_file+(j+j_file*size_key)*size] * key[j+i_key*size_key]);
        }
        *(uint8_t *) (copy_server_message+i) = (uint8_t) val;
    }
}


int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}



