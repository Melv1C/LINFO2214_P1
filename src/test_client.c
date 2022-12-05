//
// Created by melvyn on 18/10/22.
//

#include "test_client.h"
#include "debug.h"
//#include "log.h"

pthread_mutex_t lock;

uint32_t size = 128;
int rate = 100;
int run_time = 10;
char *ip = "127.0.0.1";
int port = 2241;

int nbre_request = 0;
int nbre_respond = 0;
unsigned long long totalrespt = 0;

int* sent_times;
int* receive_times;

int getts(){
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec*SEC+now.tv_usec;
}

int main(int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, "k:")) != -1) {
        switch (opt) {
            case 'k':
                size = (uint32_t) strtol(optarg, NULL, 10);
                if (!IsPowerOfTwo(size)) {
                    ERROR("The size must be a power of 2");
                    return -1;
                }
                if(size == 128){
                    rate = 25;
                }else if (size==8){
                    rate = 150;
                }
                break;
            default:
                ERROR("Usage: %s [-k Key_size]", argv[0]);
                return -1;
        }
    }

    if (optind + 1 != argc) {
        ERROR("Unexpected number of positional arguments");
        return -1;
    }

    ip = strtok(argv[optind], ":");
    port = (int) strtol(strtok(NULL, ":"), NULL, 10);

    DEBUG("Argument du client : size: %d, rate: %d, time: %d, ip: %s, port: %d", size, rate, run_time, ip, port);

    pthread_t threads[run_time*rate];

    sent_times = malloc(sizeof(int)*run_time*rate);
    receive_times = malloc(sizeof(int)*run_time*rate);
    int nbre_threads = 0;
    int diffrate = SEC / rate;

    int start = getts();
    int next = getts();
    while (getts() - start < SEC * run_time) {
        next += diffrate;
        while (getts() < next) {
            usleep((next - getts()));
        }
        sent_times[nbre_threads] = getts();
        pthread_create( &threads[nbre_threads], NULL, rcv, (void*)(intptr_t)nbre_threads);
        nbre_threads++;
    }

    INFO("Nbre respond in 10s : %d",nbre_respond);
    DEBUG("WAIT PTHREAD JOIN");
    for (int i = 0; i < nbre_threads; i++) {
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&lock);
    DEBUG("FINISH");
    DEBUG("AvgRespTime = %u µs", totalrespt/nbre_respond);
    DEBUG("Nbre of request : %d and respond : %d",nbre_request,nbre_respond);


    free(sent_times);
    free(receive_times);

    //FILE *f;
    //f = fopen("vm_stat_client.txt", "a");
    //fprintf(f,"%d,%d,%d,%d,%d,%d\n",rate,run_time,(int) sqrt(size),nbre_respond,nbre_request,totalrespt/nbre_respond);
    //fclose(f);

}

void* rcv(void* r) {
    int ret;
    int sockfd;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        ERROR("socket creation failed");
        //exit(EXIT_FAILURE);
        return ;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        ERROR("Unable to connect");
        //exit(EXIT_FAILURE);
        return;
    }
    //Send file id
    unsigned fileindex = htonl(rand() % n_files);
    ret = send(sockfd, &fileindex, 4, 0);
    if (ret<0){
        ERROR("Error while sending fileindex");
        exit(EXIT_FAILURE);
    }
    //Send key size
    unsigned revkey = htonl(size);
    ret = send(sockfd, &revkey, 4, 0);
    if (ret<0){
        ERROR("Error while sending size");
        exit(EXIT_FAILURE);
    }
    //Send key
    ARRAY_TYPE * key = malloc(sizeof(ARRAY_TYPE)*size*size);
    for (int i = 0; i < size*size; i++) {
        key[i] = (ARRAY_TYPE) rand() % MAX_VALUE_ARRAY_TYPE;
    }
    ret = send(sockfd, key, sizeof(ARRAY_TYPE) * size * size, 0);
    if (ret<0){
        ERROR("Error while sending the key");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&lock);
    nbre_request++;
    pthread_mutex_unlock(&lock);

    unsigned char error;
    if(recv(sockfd, &error, 1, 0)<0){
        ERROR("Error while recv error code");
        exit(EXIT_FAILURE);
    }

    if(error!=0){
        ERROR("RESPOND WITH WRONG ERROR CODE");
    }else{
        unsigned filesz;
        if (recv(sockfd, &filesz, 4, 0)<0){
            ERROR("Error while recv file_size");
            exit(EXIT_FAILURE);
        }
        if (filesz > 0) {
            long int left = ntohl(filesz);
            char buffer[MAX_SIZE_T];
            while (left > 0) {
                unsigned b = left;
                if (b > MAX_SIZE_T)
                    b = MAX_SIZE_T;
                ret = recv(sockfd, &buffer, b, 0);
                if (ret<0){
                    ERROR("Error while recv the file");
                    exit(EXIT_FAILURE);
                }
                left -= ret;
            }
        }
        unsigned t = (unsigned)(intptr_t)r;
        receive_times[t] = getts();

        pthread_mutex_lock(&lock);
        totalrespt += receive_times[t]-sent_times[t];
        nbre_respond++;
        pthread_mutex_unlock(&lock);
    }

    free(key);

    close(sockfd);
    return 0;
}

int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}
