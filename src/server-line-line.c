//
// Created by melvyn on 5/10/22.
//


#include "server.h"
#include "debug.h"
//#include "log.h"

int nbre_thread = 1;
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

    while ((opt = getopt(argc, argv, "j:s:p:")) != -1) {
        switch (opt) {
            case 'j':
                nbre_thread = (int) strtol(optarg,NULL,10);
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
                ERROR("Usage: %s [-s bytes] [-p port]", argv[0]);
                return -1;
        }
    }

    DEBUG("Argument du server : size: %d, port: %d\n",size,port);

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

    for (unsigned i = 0; i < square_size; i++){
        files[0][i] = i;
    }

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

    if ((listen(sockfd, 1000)) != 0) {
        ERROR("Listen failed");
        return -1;
    }

    DEBUG("---------SERVER IS READY---------");

    int client_sock;
    int c = sizeof(servaddr);
    while(client_sock = accept(sockfd, (struct sockaddr *)&servaddr,(socklen_t*)&c)) {
        if (client_sock < 0) {
            ERROR("Can't accept");
        }else{
            if(connection_handler(client_sock)!=0){
                ERROR("error during connection handler for client sock %d",client_sock);
            }
        }
    }
}

int connection_handler(int sockfd) {

    uint32_t fileid;
    int tread = recv(sockfd, &fileid, 4, 0);
    fileid = ntohl(fileid);
    if(tread<0){
        ERROR("Error read fileid");
        return -1;
    }

    uint32_t keysz;
    tread = recv(sockfd, &keysz, 4, 0);
    keysz = ntohl(keysz);
    if(tread<0){
        ERROR("Error read key size");
        return -1;
    }

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
    ARRAY_TYPE* crypted = malloc(size*size*sizeof(ARRAY_TYPE));
    //Compute sub-matrices
    for (int v = 0; v < nr ; v ++) {
        int vstart = v * keysz;
        for (int h = 0; h < nr; h++) {
            int hstart = h * keysz;
            //Do the sub-matrix multiplication
            ARRAY_TYPE Bcolj[keysz];
            for (int j = 0; j < keysz; j++) {
                for (int k = 0; k < keysz; k++)
                    Bcolj[k] = file[(k+vstart)*size + j + hstart];

                for (int i = 0; i < keysz; i++) {
                    ARRAY_TYPE s = 0;
                    for (int k = 0; k < keysz; k++)
                        s += key[i*keysz + k] * Bcolj[k];
                    crypted[(i+vstart)*size + j + hstart] = s;
                }
            }
        }
    }

    //print_matrix(file,size);
    //print_matrix(key,keysz);
    //print_matrix(crypted,size);

    uint8_t err = 0;

    send(sockfd, &err, 1,MSG_NOSIGNAL );
    unsigned sz = htonl(square_size * sizeof(ARRAY_TYPE));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    send(sockfd, crypted, square_size * sizeof(ARRAY_TYPE),MSG_NOSIGNAL);
    //print_matrix(crypted,size);
    close(sockfd);
    free(crypted);

    return 0;

}


int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}



