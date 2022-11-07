//
// Created by melvyn on 5/10/22.
//


#include "server-optim.h"
#include "debug.h"
//#include "log.h"

int port = 2241;
int nbre_thread = 1;
uint32_t size = 1024;
int nbre_request = 0;
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
                if (nbre_thread!=1){
                    ERROR("Only support 1 thread");
                    return -1;
                }
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


    //---------------------------------------------------------------------------------------------------
    //GENERATION DES FICHIERS
    //---------------------------------------------------------------------------------------------------

    files = malloc(sizeof(void*) * n_files);
    for (int i = 0 ; i < n_files; i++){
        files[i] = aligned_alloc(size*size,size*size *sizeof(ARRAY_TYPE));
        // Pour random les valeurs
        /*for (int j = 0; j < size*size; j++) {
            files[i][j] = (ARRAY_TYPE) rand() % MAX_VALUE_ARRAY_TYPE;
        }*/
    }

    for (unsigned i = 0; i < size * size; i++){
        files[0][i] = i;
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

    if ((listen(sockfd, 1000)) != 0) {
        ERROR("Listen failed");
        return -1;
    }

    DEBUG("---------SERVER IS READY---------");

    int nbre_client = 0;

    int client_sock;
    int c = sizeof(servaddr);
    while(client_sock = accept(sockfd, (struct sockaddr *)&servaddr,(socklen_t*)&c)) {


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
    ARRAY_TYPE key[keysz*keysz*sizeof(ARRAY_TYPE)];
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

    /*int nr = size / keysz;
    ARRAY_TYPE* file = files[fileid % n_files];
    ARRAY_TYPE* crypted = calloc(size*size,sizeof(ARRAY_TYPE));
    //Compute sub-matrices
    for (int i = 0; i < nr ; i ++) {
        int vstart = i * keysz;
        for (int j = 0; j < nr; j++) {
            int hstart = j * keysz;
            //Do the sub-matrix multiplication
            for (int k = 0; k < keysz; k++) {
                for (int l = 0; l < keysz; l++) {
                    int a = key[k*keysz + l];
                    for (int m = 0; m < keysz; m++) {
                        crypted[(vstart+k)*size + (hstart+m)] += a * file[(vstart+l)*size + (hstart+m)];
                    }
                }
            }
        }
    }*/

    int nr = size / keysz;
    ARRAY_TYPE* file = files[fileid % n_files];
    ARRAY_TYPE* crypted = calloc(size*size,sizeof(ARRAY_TYPE));
    //Compute sub-matrices
    for (int i = 0; i < nr ; i ++) {
        int vstart = i * keysz;
        for (int j = 0; j < nr; j++) {
            int hstart = j * keysz;
            //Do the sub-matrix multiplication
            for (int k = 0; k < keysz; k++) {
                for (int l = 0; l < keysz/MIN_SIZE_KEY; l++) {
                    int new_l = l*MIN_SIZE_KEY;
                    int a = key[k*keysz + new_l];
                    int a1 = key[k*keysz + new_l+1];
                    int a2 = key[k*keysz + new_l+2];
                    int a3 = key[k*keysz + new_l+3];
                    int a4 = key[k*keysz + new_l+4];
                    int a5 = key[k*keysz + new_l+5];
                    int a6 = key[k*keysz + new_l+6];
                    int a7 = key[k*keysz + new_l+7];

                    for (int m = 0; m < keysz/MIN_SIZE_KEY; m++) {
                        int new_m = m*MIN_SIZE_KEY;
                        crypted[(vstart+k)*size + (hstart+new_m)] += a * file[(vstart+new_l)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a * file[(vstart+new_l)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a * file[(vstart+new_l)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a * file[(vstart+new_l)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a * file[(vstart+new_l)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a * file[(vstart+new_l)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a * file[(vstart+new_l)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a * file[(vstart+new_l)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a1 * file[(vstart+new_l+1)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a2 * file[(vstart+new_l+2)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a3 * file[(vstart+new_l+3)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a4 * file[(vstart+new_l+4)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a5 * file[(vstart+new_l+5)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a6 * file[(vstart+new_l+6)*size + (hstart+new_m+7)];

                        crypted[(vstart+k)*size + (hstart+new_m)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m)];
                        crypted[(vstart+k)*size + (hstart+new_m+1)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+1)];
                        crypted[(vstart+k)*size + (hstart+new_m+2)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+2)];
                        crypted[(vstart+k)*size + (hstart+new_m+3)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+3)];
                        crypted[(vstart+k)*size + (hstart+new_m+4)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+4)];
                        crypted[(vstart+k)*size + (hstart+new_m+5)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+5)];
                        crypted[(vstart+k)*size + (hstart+new_m+6)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+6)];
                        crypted[(vstart+k)*size + (hstart+new_m+7)] += a7 * file[(vstart+new_l+7)*size + (hstart+new_m+7)];
                    }
                }
            }
        }
    }

    /*print_matrix(file,size);
    print_matrix(key,keysz);
    print_matrix(crypted,size);*/

    uint8_t err = 0;

    send(sockfd, &err, 1,MSG_NOSIGNAL );
    unsigned sz = htonl(size*size * sizeof(ARRAY_TYPE));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    send(sockfd, crypted, size*size * sizeof(ARRAY_TYPE),MSG_NOSIGNAL);

    close(sockfd);
    free(crypted);

    nbre_request++;

    return 0;

}


int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}



