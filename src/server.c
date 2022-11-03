//
// Created by melvyn on 5/10/22.
//


#include "server.h"
#include "debug.h"
#include "log.h"

int nbre_request = 0;
uint32_t size = 1024;
int port = 2241;
uint32_t square_size;
ARRAY_TYPE ** files;

int getts(){
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec*SEC+now.tv_usec;
}

int main(int argc, char **argv) {

    //---------------------------------------------------------------------------------------------------
    //PARSING DES ARGUMENTS
    //---------------------------------------------------------------------------------------------------

    int opt;

    while ((opt = getopt(argc, argv, "s:p:")) != -1) {
        switch (opt) {
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
                ERROR("Usage: %s [-s bytes] [-p port]", argv[0]);
                return -1;
        }
    }

    INFO("Argument du server : size: %d, port: %d\n",size,port);

    square_size = (uint32_t) size*size;   //Plus pratique

    //---------------------------------------------------------------------------------------------------
    //GENERATION DES FICHIERS
    //---------------------------------------------------------------------------------------------------

    files = malloc(sizeof(void*) * n_files);
    for (int i = 0 ; i < n_files; i++){
        files[i] = malloc(square_size *sizeof(ARRAY_TYPE));
        // Pour random les valeurs
        /*for (int j = 0; j < square_size; j++) {
            files[i][j] = (ARRAY_TYPE) rand() % MAX_VALUE_ARRAY_TYPE;
        }*/
    }

    //print_matrix(files[0],size);
    //print_matrix(files[500],size);

    //---------------------------------------------------------------------------------------------------
    //CREATION DU SOCKET ET ECOUTE
    //---------------------------------------------------------------------------------------------------

    // Creating socket file descriptor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd<0){
        ERROR("Error while creating socket");
        return -1;
    }
    DEBUG("Socket created successfully");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    // Bind the socket with the server address
    if(bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr))<0){
        ERROR("Couldn't bind to the port");
        return -1;
    }

    DEBUG("Done with binding");

    if ((listen(sockfd, 128)) != 0) {
        ERROR("Listen failed");
        return -1;
    }

    INFO("---------SERVER IS READY---------");

    int nbre_client = 0;

    int client_sock;
    int c = sizeof(servaddr);
    while(1) {

        fd_set set;
        FD_ZERO(&set); /* clear the set */
        FD_SET(sockfd, &set); /* add our file descriptor to the set */
        struct timeval timeout = {10, 0};
        int activity = select( sockfd+1 , &set , NULL , NULL , &timeout);

        if(activity<0){
            ERROR("Select fonction");
        } else if (activity == 0){
            //timeout
            char commande;
            printf("Arreter le serveur ? [Y,N]");
            scanf("%1s", &commande);
            while (commande!='Y'&&commande!='N'){
                printf("Veuillez indiquer soit Y pour arreter le serveur ou N pour qu'il continue à fonctionner\n");
                printf("Arreter le serveur ? [Y,N]");
                scanf("%1s", &commande);
            }
            if(commande=='Y'){
                INFO("STATS:\n\nNonbre de client qui se sont connecté au server : %d\nNombre de request receive to the server : %d",nbre_client,nbre_request);
                INFO("FINISH");
                free(files);
                return 0;
            }
        } else {
            client_sock = accept(sockfd, (struct sockaddr *)&servaddr,(socklen_t*)&c);
            if (client_sock < 0) {
                ERROR("Can't accept");
            }else{
                nbre_client++;
                if(connection_handler(client_sock)!=0){
                    ERROR("error during connection handler for client sock %d, nbre of client %d",client_sock,nbre_client);
                }
            }
        }
    }
}

int connection_handler(int sockfd) {

    uint32_t fileid;
    int tread = recv(sockfd, &fileid, 4, 0);
    if(tread<0){
        ERROR("Error read fileid");
        return -1;
    }

    DEBUG("NEW REQUEST ID %d",fileid);

    uint32_t keysz;
    tread = recv(sockfd, &keysz, 4, 0);
    if(tread<0){
        ERROR("Error read key size");
        return -1;
    }

    DEBUG("NEW REQUEST SIZE %d",keysz);

    //Network byte order
    ARRAY_TYPE key[keysz*keysz];
    unsigned tot = keysz*keysz * sizeof(ARRAY_TYPE);
    unsigned done = 0;
    while (done < tot) {
        tread = recv(sockfd, key, tot - done, 0);
        if(tread<0){
            ERROR("Error read key");
            return -1;
        }
        done += tread;
    }

    int nr = size / keysz;
    ARRAY_TYPE* file = files[fileid % n_files];
    ARRAY_TYPE* crypted = malloc(square_size * sizeof(ARRAY_TYPE));
    //Compute sub-matrices
    for (int i = 0; i < nr ; i ++) {
        int vstart = i * keysz;
        for (int j = 0; j < nr; j++) {
            int hstart = j * keysz;
            //Do the sub-matrix multiplication
            for (int ln = 0; ln < keysz; ln++) {
                int aline = (vstart + ln) * size + hstart;
                for (int col = 0; col < keysz; col++) {
                    int tot = 0;
                    for (int k = 0; k < keysz; k++) {
                        int vline = (vstart + k) * size + hstart;
                        tot += key[ln * keysz + k] * file[vline + col];
                    }
                    crypted[aline + col] = tot;
                }
            }
        }
    }

    uint8_t err = 0;

    send(sockfd, &err, 1,MSG_NOSIGNAL );
    uint32_t sz = square_size * sizeof(ARRAY_TYPE);
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    send(sockfd, crypted, square_size * sizeof(ARRAY_TYPE),MSG_NOSIGNAL);
    //print_matrix(crypted,size);
    close(sockfd);

    nbre_request++;

    return 0;

}


int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}



