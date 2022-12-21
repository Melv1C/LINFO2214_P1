//
// Created by melvyn on 18/10/22.
//

#include "client-queue.h"
#include "debug.h"
//#include "log.h"

pthread_mutex_t lock;

uint32_t size = 128;
int rate = 1;
int run_time = 1;
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

//Generate a random exponential interval time
uint64_t ran_expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) * SEC / lambda;
}

int main(int argc, char **argv) {
    int opt;

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
                run_time = (int) strtol(optarg, NULL, 10);
                break;
            default:
                ERROR("Usage: %s [-k Key_size] [-r rate] [-t time]", argv[0]);
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

    uint32_t square_size = size * size;

    pthread_t threads[run_time*rate];

    sent_times = malloc(sizeof(int)*run_time*rate);
    receive_times = malloc(sizeof(int)*run_time*rate);
    int nbre_threads = 0;
    uint64_t diffrate = 0;

    int start = getts();
    int next = getts();
    while (getts() - start < SEC * run_time) {
        diffrate = ran_expo(rate);
        FILE *fp = fopen("arrival_time.txt", "a");
        fprintf(fp, "%d\n", diffrate);
        // Close the file
        fclose(fp);
        next += diffrate;
        while (getts() < next) {
            usleep((next - getts()));
        }
        sent_times[nbre_threads] = getts();
        pthread_create( &threads[nbre_threads], NULL, rcv, (void*)(intptr_t)nbre_threads);
        nbre_threads++;
    }

    INFO("WAIT PTHREAD JOIN");
    for (int i = 0; i < nbre_threads; i++) {
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&lock);
    INFO("FINISH");
    INFO("AvgRespTime = %u µs", totalrespt/nbre_respond);
    INFO("Nbre of request : %d and respond : %d",nbre_request,nbre_respond);

    free(sent_times);
    free(receive_times);

}

void* rcv(void* r) {
    int ret;
    int sockfd;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        ERROR("socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        ERROR("Unable to connect");
        exit(EXIT_FAILURE);
    }
    //Send file id
    //unsigned fileindex = htonl(rand() % n_files);
    unsigned fileindex = htonl(0);
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
        //key[i] = (ARRAY_TYPE) rand(); //% MAX_VALUE_ARRAY_TYPE;
        //key[i] = (ARRAY_TYPE)(rand()%100);
        key[i] = (ARRAY_TYPE)rand()/(ARRAY_TYPE)(RAND_MAX/1);
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

        FILE *f;
        f = fopen("respond_time.txt", "a");
        fprintf(f,"%d\n",receive_times[t]-sent_times[t]);
        fclose(f);
    }

    free(key);

    close(sockfd);
    return 0;
}

int IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}