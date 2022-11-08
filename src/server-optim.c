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
        files[i] = aligned_alloc(size,size*size *sizeof(ARRAY_TYPE));
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
    ARRAY_TYPE * key = aligned_alloc(keysz,keysz*keysz*sizeof(ARRAY_TYPE));
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
    struct index_t index;
    index.size = size;
    index.keysz = keysz;
    for (int v = 0; v < nr ; v ++) {
        index.vstart = v * index.keysz;
        for (int h = 0; h < nr; h++) {
            index.hstart = h * index.keysz;
            //Do the sub-matrix multiplication
            ARRAY_TYPE * Bcolj = malloc(keysz*sizeof(ARRAY_TYPE));
            for (int j = 0; j < index.keysz; j++) {
                if (keysz == MIN_SIZE_KEY){
                    Bcolj[0] = file[(index.vstart)*index.size + j + index.hstart];
                    Bcolj[1] = file[(1+index.vstart)*index.size + j + index.hstart];
                    Bcolj[2] = file[(2+index.vstart)*index.size + j + index.hstart];
                    Bcolj[3] = file[(3+index.vstart)*index.size + j + index.hstart];
                    Bcolj[4] = file[(4+index.vstart)*index.size + j + index.hstart];
                    Bcolj[5] = file[(5+index.vstart)*index.size + j + index.hstart];
                    Bcolj[6] = file[(6+index.vstart)*index.size + j + index.hstart];
                    Bcolj[7] = file[(7+index.vstart)*index.size + j + index.hstart];
                } else{
                    for (int k = 0; k < index.keysz/MIN_SIZE_KEY; k++){
                        index.new_k = k*MIN_SIZE_KEY;
                        Bcolj[index.new_k] = file[(index.new_k+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+1] = file[(index.new_k+1+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+2] = file[(index.new_k+2+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+3] = file[(index.new_k+3+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+4] = file[(index.new_k+4+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+5] = file[(index.new_k+5+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+6] = file[(index.new_k+6+index.vstart)*index.size + j + index.hstart];
                        Bcolj[index.new_k+7] = file[(index.new_k+7+index.vstart)*index.size + j + index.hstart];
                    }
                }

                for (int i = 0; i < index.keysz; i++) {
                    ARRAY_TYPE s = 0;
                    if (keysz == MIN_SIZE_KEY){
                        s += key[i*index.keysz] * Bcolj[0];
                        s += key[i*index.keysz + 1] * Bcolj[1];
                        s += key[i*index.keysz + 2] * Bcolj[2];
                        s += key[i*index.keysz + 3] * Bcolj[3];
                        s += key[i*index.keysz + 4] * Bcolj[4];
                        s += key[i*index.keysz + 5] * Bcolj[5];
                        s += key[i*index.keysz + 6] * Bcolj[6];
                        s += key[i*index.keysz + 7] * Bcolj[7];
                    }else{
                        for (int k = 0; k < index.keysz/MIN_SIZE_KEY; k++){
                            index.new_k = k*MIN_SIZE_KEY;
                            s += key[i*index.keysz + index.new_k] * Bcolj[index.new_k];
                            s += key[i*index.keysz + index.new_k+1] * Bcolj[index.new_k+1];
                            s += key[i*index.keysz + index.new_k+2] * Bcolj[index.new_k+2];
                            s += key[i*index.keysz + index.new_k+3] * Bcolj[index.new_k+3];
                            s += key[i*index.keysz + index.new_k+4] * Bcolj[index.new_k+4];
                            s += key[i*index.keysz + index.new_k+5] * Bcolj[index.new_k+5];
                            s += key[i*index.keysz + index.new_k+6] * Bcolj[index.new_k+6];
                            s += key[i*index.keysz + index.new_k+7] * Bcolj[index.new_k+7];
                        }
                    }
                    crypted[(i+index.vstart)*index.size + j + index.hstart] = s;
                }
            }
            free(Bcolj);
        }
    }



    //print_matrix(file,size);
    //print_matrix(key,keysz);
    //print_matrix(crypted,size);

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



