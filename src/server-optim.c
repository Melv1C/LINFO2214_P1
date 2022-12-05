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
    ARRAY_TYPE* crypted = aligned_alloc(size,size*size*sizeof(ARRAY_TYPE));
    //Compute sub-matrices
    struct index_t index;
    index.size = size;
    index.keysz = keysz;
    ARRAY_TYPE * Bcolj = aligned_alloc(keysz,keysz*sizeof(ARRAY_TYPE));
    for (int v = 0; v < nr ; v ++) {
        index.vstart = v * index.keysz;
        for (int h = 0; h < nr; h++) {
            index.hstart = h * index.keysz;
            //Do the sub-matrix multiplication
            for (int j = 0; j < index.keysz; j++) {

                if (keysz == MIN_SIZE_KEY) {
                    Bcolj[0] = file[(index.vstart) * index.size + j + index.hstart];
                    Bcolj[1] = file[(1 + index.vstart) * index.size + j + index.hstart];
                    Bcolj[2] = file[(2 + index.vstart) * index.size + j + index.hstart];
                    Bcolj[3] = file[(3 + index.vstart) * index.size + j + index.hstart];
                    Bcolj[4] = file[(4 + index.vstart) * index.size + j + index.hstart];
                    Bcolj[5] = file[(5 + index.vstart) * index.size + j + index.hstart];
                    Bcolj[6] = file[(6 + index.vstart) * index.size + j + index.hstart];
                    Bcolj[7] = file[(7 + index.vstart) * index.size + j + index.hstart];
                }else{
                    Bcolj[0] = file[(index.vstart)*index.size + j + index.hstart];
                    Bcolj[1] = file[(1+index.vstart)*index.size + j + index.hstart];
                    Bcolj[2] = file[(2+index.vstart)*index.size + j + index.hstart];
                    Bcolj[3] = file[(3+index.vstart)*index.size + j + index.hstart];
                    Bcolj[4] = file[(4+index.vstart)*index.size + j + index.hstart];
                    Bcolj[5] = file[(5+index.vstart)*index.size + j + index.hstart];
                    Bcolj[6] = file[(6+index.vstart)*index.size + j + index.hstart];
                    Bcolj[7] = file[(7+index.vstart)*index.size + j + index.hstart];
                    Bcolj[8] = file[(8+index.vstart)*index.size + j + index.hstart];
                    Bcolj[9] = file[(9+index.vstart)*index.size + j + index.hstart];
                    Bcolj[10] = file[(10+index.vstart)*index.size + j + index.hstart];
                    Bcolj[11] = file[(11+index.vstart)*index.size + j + index.hstart];
                    Bcolj[12] = file[(12+index.vstart)*index.size + j + index.hstart];
                    Bcolj[13] = file[(13+index.vstart)*index.size + j + index.hstart];
                    Bcolj[14] = file[(14+index.vstart)*index.size + j + index.hstart];
                    Bcolj[15] = file[(15+index.vstart)*index.size + j + index.hstart];
                    Bcolj[16] = file[(16+index.vstart)*index.size + j + index.hstart];
                    Bcolj[17] = file[(17+index.vstart)*index.size + j + index.hstart];
                    Bcolj[18] = file[(18+index.vstart)*index.size + j + index.hstart];
                    Bcolj[19] = file[(19+index.vstart)*index.size + j + index.hstart];
                    Bcolj[20] = file[(20+index.vstart)*index.size + j + index.hstart];
                    Bcolj[21] = file[(21+index.vstart)*index.size + j + index.hstart];
                    Bcolj[22] = file[(22+index.vstart)*index.size + j + index.hstart];
                    Bcolj[23] = file[(23+index.vstart)*index.size + j + index.hstart];
                    Bcolj[24] = file[(24+index.vstart)*index.size + j + index.hstart];
                    Bcolj[25] = file[(25+index.vstart)*index.size + j + index.hstart];
                    Bcolj[26] = file[(26+index.vstart)*index.size + j + index.hstart];
                    Bcolj[27] = file[(27+index.vstart)*index.size + j + index.hstart];
                    Bcolj[28] = file[(28+index.vstart)*index.size + j + index.hstart];
                    Bcolj[29] = file[(29+index.vstart)*index.size + j + index.hstart];
                    Bcolj[30] = file[(30+index.vstart)*index.size + j + index.hstart];
                    Bcolj[31] = file[(31+index.vstart)*index.size + j + index.hstart];
                    Bcolj[32] = file[(32+index.vstart)*index.size + j + index.hstart];
                    Bcolj[33] = file[(33+index.vstart)*index.size + j + index.hstart];
                    Bcolj[34] = file[(34+index.vstart)*index.size + j + index.hstart];
                    Bcolj[35] = file[(35+index.vstart)*index.size + j + index.hstart];
                    Bcolj[36] = file[(36+index.vstart)*index.size + j + index.hstart];
                    Bcolj[37] = file[(37+index.vstart)*index.size + j + index.hstart];
                    Bcolj[38] = file[(38+index.vstart)*index.size + j + index.hstart];
                    Bcolj[39] = file[(39+index.vstart)*index.size + j + index.hstart];
                    Bcolj[40] = file[(40+index.vstart)*index.size + j + index.hstart];
                    Bcolj[41] = file[(41+index.vstart)*index.size + j + index.hstart];
                    Bcolj[42] = file[(42+index.vstart)*index.size + j + index.hstart];
                    Bcolj[43] = file[(43+index.vstart)*index.size + j + index.hstart];
                    Bcolj[44] = file[(44+index.vstart)*index.size + j + index.hstart];
                    Bcolj[45] = file[(45+index.vstart)*index.size + j + index.hstart];
                    Bcolj[46] = file[(46+index.vstart)*index.size + j + index.hstart];
                    Bcolj[47] = file[(47+index.vstart)*index.size + j + index.hstart];
                    Bcolj[48] = file[(48+index.vstart)*index.size + j + index.hstart];
                    Bcolj[49] = file[(49+index.vstart)*index.size + j + index.hstart];
                    Bcolj[50] = file[(50+index.vstart)*index.size + j + index.hstart];
                    Bcolj[51] = file[(51+index.vstart)*index.size + j + index.hstart];
                    Bcolj[52] = file[(52+index.vstart)*index.size + j + index.hstart];
                    Bcolj[53] = file[(53+index.vstart)*index.size + j + index.hstart];
                    Bcolj[54] = file[(54+index.vstart)*index.size + j + index.hstart];
                    Bcolj[55] = file[(55+index.vstart)*index.size + j + index.hstart];
                    Bcolj[56] = file[(56+index.vstart)*index.size + j + index.hstart];
                    Bcolj[57] = file[(57+index.vstart)*index.size + j + index.hstart];
                    Bcolj[58] = file[(58+index.vstart)*index.size + j + index.hstart];
                    Bcolj[59] = file[(59+index.vstart)*index.size + j + index.hstart];
                    Bcolj[60] = file[(60+index.vstart)*index.size + j + index.hstart];
                    Bcolj[61] = file[(61+index.vstart)*index.size + j + index.hstart];
                    Bcolj[62] = file[(62+index.vstart)*index.size + j + index.hstart];
                    Bcolj[63] = file[(63+index.vstart)*index.size + j + index.hstart];
                    Bcolj[64] = file[(64+index.vstart)*index.size + j + index.hstart];
                    Bcolj[65] = file[(65+index.vstart)*index.size + j + index.hstart];
                    Bcolj[66] = file[(66+index.vstart)*index.size + j + index.hstart];
                    Bcolj[67] = file[(67+index.vstart)*index.size + j + index.hstart];
                    Bcolj[68] = file[(68+index.vstart)*index.size + j + index.hstart];
                    Bcolj[69] = file[(69+index.vstart)*index.size + j + index.hstart];
                    Bcolj[70] = file[(70+index.vstart)*index.size + j + index.hstart];
                    Bcolj[71] = file[(71+index.vstart)*index.size + j + index.hstart];
                    Bcolj[72] = file[(72+index.vstart)*index.size + j + index.hstart];
                    Bcolj[73] = file[(73+index.vstart)*index.size + j + index.hstart];
                    Bcolj[74] = file[(74+index.vstart)*index.size + j + index.hstart];
                    Bcolj[75] = file[(75+index.vstart)*index.size + j + index.hstart];
                    Bcolj[76] = file[(76+index.vstart)*index.size + j + index.hstart];
                    Bcolj[77] = file[(77+index.vstart)*index.size + j + index.hstart];
                    Bcolj[78] = file[(78+index.vstart)*index.size + j + index.hstart];
                    Bcolj[79] = file[(79+index.vstart)*index.size + j + index.hstart];
                    Bcolj[80] = file[(80+index.vstart)*index.size + j + index.hstart];
                    Bcolj[81] = file[(81+index.vstart)*index.size + j + index.hstart];
                    Bcolj[82] = file[(82+index.vstart)*index.size + j + index.hstart];
                    Bcolj[83] = file[(83+index.vstart)*index.size + j + index.hstart];
                    Bcolj[84] = file[(84+index.vstart)*index.size + j + index.hstart];
                    Bcolj[85] = file[(85+index.vstart)*index.size + j + index.hstart];
                    Bcolj[86] = file[(86+index.vstart)*index.size + j + index.hstart];
                    Bcolj[87] = file[(87+index.vstart)*index.size + j + index.hstart];
                    Bcolj[88] = file[(88+index.vstart)*index.size + j + index.hstart];
                    Bcolj[89] = file[(89+index.vstart)*index.size + j + index.hstart];
                    Bcolj[90] = file[(90+index.vstart)*index.size + j + index.hstart];
                    Bcolj[91] = file[(91+index.vstart)*index.size + j + index.hstart];
                    Bcolj[92] = file[(92+index.vstart)*index.size + j + index.hstart];
                    Bcolj[93] = file[(93+index.vstart)*index.size + j + index.hstart];
                    Bcolj[94] = file[(94+index.vstart)*index.size + j + index.hstart];
                    Bcolj[95] = file[(95+index.vstart)*index.size + j + index.hstart];
                    Bcolj[96] = file[(96+index.vstart)*index.size + j + index.hstart];
                    Bcolj[97] = file[(97+index.vstart)*index.size + j + index.hstart];
                    Bcolj[98] = file[(98+index.vstart)*index.size + j + index.hstart];
                    Bcolj[99] = file[(99+index.vstart)*index.size + j + index.hstart];
                    Bcolj[100] = file[(100+index.vstart)*index.size + j + index.hstart];
                    Bcolj[101] = file[(101+index.vstart)*index.size + j + index.hstart];
                    Bcolj[102] = file[(102+index.vstart)*index.size + j + index.hstart];
                    Bcolj[103] = file[(103+index.vstart)*index.size + j + index.hstart];
                    Bcolj[104] = file[(104+index.vstart)*index.size + j + index.hstart];
                    Bcolj[105] = file[(105+index.vstart)*index.size + j + index.hstart];
                    Bcolj[106] = file[(106+index.vstart)*index.size + j + index.hstart];
                    Bcolj[107] = file[(107+index.vstart)*index.size + j + index.hstart];
                    Bcolj[108] = file[(108+index.vstart)*index.size + j + index.hstart];
                    Bcolj[109] = file[(109+index.vstart)*index.size + j + index.hstart];
                    Bcolj[110] = file[(110+index.vstart)*index.size + j + index.hstart];
                    Bcolj[111] = file[(111+index.vstart)*index.size + j + index.hstart];
                    Bcolj[112] = file[(112+index.vstart)*index.size + j + index.hstart];
                    Bcolj[113] = file[(113+index.vstart)*index.size + j + index.hstart];
                    Bcolj[114] = file[(114+index.vstart)*index.size + j + index.hstart];
                    Bcolj[115] = file[(115+index.vstart)*index.size + j + index.hstart];
                    Bcolj[116] = file[(116+index.vstart)*index.size + j + index.hstart];
                    Bcolj[117] = file[(117+index.vstart)*index.size + j + index.hstart];
                    Bcolj[118] = file[(118+index.vstart)*index.size + j + index.hstart];
                    Bcolj[119] = file[(119+index.vstart)*index.size + j + index.hstart];
                    Bcolj[120] = file[(120+index.vstart)*index.size + j + index.hstart];
                    Bcolj[121] = file[(121+index.vstart)*index.size + j + index.hstart];
                    Bcolj[122] = file[(122+index.vstart)*index.size + j + index.hstart];
                    Bcolj[123] = file[(123+index.vstart)*index.size + j + index.hstart];
                    Bcolj[124] = file[(124+index.vstart)*index.size + j + index.hstart];
                    Bcolj[125] = file[(125+index.vstart)*index.size + j + index.hstart];
                    Bcolj[126] = file[(126+index.vstart)*index.size + j + index.hstart];
                    Bcolj[127] = file[(127+index.vstart)*index.size + j + index.hstart];
                
                }

                for (int i = 0; i < index.keysz; i++) {

                    ARRAY_TYPE s = 0;

                    if (keysz == MIN_SIZE_KEY) {
                        s+= key[i * index.keysz] * Bcolj[0];
                        s+= key[i * index.keysz + 1] * Bcolj[1];
                        s+= key[i * index.keysz + 2] * Bcolj[2];
                        s+= key[i * index.keysz + 3] * Bcolj[3];
                        s+= key[i * index.keysz + 4] * Bcolj[4];
                        s+= key[i * index.keysz + 5] * Bcolj[5];
                        s+= key[i * index.keysz + 6] * Bcolj[6];
                        s+= key[i * index.keysz + 7] * Bcolj[7];

                    }else{
                        
                        s += key[i * index.keysz] * Bcolj[0];
                        s += key[i * index.keysz+1] * Bcolj[1];
                        s += key[i * index.keysz+2] * Bcolj[2];
                        s += key[i * index.keysz+3] * Bcolj[3];
                        s += key[i * index.keysz+4] * Bcolj[4];
                        s += key[i * index.keysz+5] * Bcolj[5];
                        s += key[i * index.keysz+6] * Bcolj[6];
                        s += key[i * index.keysz+7] * Bcolj[7];
                        s += key[i * index.keysz+8] * Bcolj[8];
                        s += key[i * index.keysz+9] * Bcolj[9];
                        s += key[i * index.keysz+10] * Bcolj[10];
                        s += key[i * index.keysz+11] * Bcolj[11];
                        s += key[i * index.keysz+12] * Bcolj[12];
                        s += key[i * index.keysz+13] * Bcolj[13];
                        s += key[i * index.keysz+14] * Bcolj[14];
                        s += key[i * index.keysz+15] * Bcolj[15];
                        s += key[i * index.keysz+16] * Bcolj[16];
                        s += key[i * index.keysz+17] * Bcolj[17];
                        s += key[i * index.keysz+18] * Bcolj[18];
                        s += key[i * index.keysz+19] * Bcolj[19];
                        s += key[i * index.keysz+20] * Bcolj[20];
                        s += key[i * index.keysz+21] * Bcolj[21];
                        s += key[i * index.keysz+22] * Bcolj[22];
                        s += key[i * index.keysz+23] * Bcolj[23];
                        s += key[i * index.keysz+24] * Bcolj[24];
                        s += key[i * index.keysz+25] * Bcolj[25];
                        s += key[i * index.keysz+26] * Bcolj[26];
                        s += key[i * index.keysz+27] * Bcolj[27];
                        s += key[i * index.keysz+28] * Bcolj[28];
                        s += key[i * index.keysz+29] * Bcolj[29];
                        s += key[i * index.keysz+30] * Bcolj[30];
                        s += key[i * index.keysz+31] * Bcolj[31];
                        s += key[i * index.keysz+32] * Bcolj[32];
                        s += key[i * index.keysz+33] * Bcolj[33];
                        s += key[i * index.keysz+34] * Bcolj[34];
                        s += key[i * index.keysz+35] * Bcolj[35];
                        s += key[i * index.keysz+36] * Bcolj[36];
                        s += key[i * index.keysz+37] * Bcolj[37];
                        s += key[i * index.keysz+38] * Bcolj[38];
                        s += key[i * index.keysz+39] * Bcolj[39];
                        s += key[i * index.keysz+40] * Bcolj[40];
                        s += key[i * index.keysz+41] * Bcolj[41];
                        s += key[i * index.keysz+42] * Bcolj[42];
                        s += key[i * index.keysz+43] * Bcolj[43];
                        s += key[i * index.keysz+44] * Bcolj[44];
                        s += key[i * index.keysz+45] * Bcolj[45];
                        s += key[i * index.keysz+46] * Bcolj[46];
                        s += key[i * index.keysz+47] * Bcolj[47];
                        s += key[i * index.keysz+48] * Bcolj[48];
                        s += key[i * index.keysz+49] * Bcolj[49];
                        s += key[i * index.keysz+50] * Bcolj[50];
                        s += key[i * index.keysz+51] * Bcolj[51];
                        s += key[i * index.keysz+52] * Bcolj[52];
                        s += key[i * index.keysz+53] * Bcolj[53];
                        s += key[i * index.keysz+54] * Bcolj[54];
                        s += key[i * index.keysz+55] * Bcolj[55];
                        s += key[i * index.keysz+56] * Bcolj[56];
                        s += key[i * index.keysz+57] * Bcolj[57];
                        s += key[i * index.keysz+58] * Bcolj[58];
                        s += key[i * index.keysz+59] * Bcolj[59];
                        s += key[i * index.keysz+60] * Bcolj[60];
                        s += key[i * index.keysz+61] * Bcolj[61];
                        s += key[i * index.keysz+62] * Bcolj[62];
                        s += key[i * index.keysz+63] * Bcolj[63];
                        s += key[i * index.keysz+64] * Bcolj[64];
                        s += key[i * index.keysz+65] * Bcolj[65];
                        s += key[i * index.keysz+66] * Bcolj[66];
                        s += key[i * index.keysz+67] * Bcolj[67];
                        s += key[i * index.keysz+68] * Bcolj[68];
                        s += key[i * index.keysz+69] * Bcolj[69];
                        s += key[i * index.keysz+70] * Bcolj[70];
                        s += key[i * index.keysz+71] * Bcolj[71];
                        s += key[i * index.keysz+72] * Bcolj[72];
                        s += key[i * index.keysz+73] * Bcolj[73];
                        s += key[i * index.keysz+74] * Bcolj[74];
                        s += key[i * index.keysz+75] * Bcolj[75];
                        s += key[i * index.keysz+76] * Bcolj[76];
                        s += key[i * index.keysz+77] * Bcolj[77];
                        s += key[i * index.keysz+78] * Bcolj[78];
                        s += key[i * index.keysz+79] * Bcolj[79];
                        s += key[i * index.keysz+80] * Bcolj[80];
                        s += key[i * index.keysz+81] * Bcolj[81];
                        s += key[i * index.keysz+82] * Bcolj[82];
                        s += key[i * index.keysz+83] * Bcolj[83];
                        s += key[i * index.keysz+84] * Bcolj[84];
                        s += key[i * index.keysz+85] * Bcolj[85];
                        s += key[i * index.keysz+86] * Bcolj[86];
                        s += key[i * index.keysz+87] * Bcolj[87];
                        s += key[i * index.keysz+88] * Bcolj[88];
                        s += key[i * index.keysz+89] * Bcolj[89];
                        s += key[i * index.keysz+90] * Bcolj[90];
                        s += key[i * index.keysz+91] * Bcolj[91];
                        s += key[i * index.keysz+92] * Bcolj[92];
                        s += key[i * index.keysz+93] * Bcolj[93];
                        s += key[i * index.keysz+94] * Bcolj[94];
                        s += key[i * index.keysz+95] * Bcolj[95];
                        s += key[i * index.keysz+96] * Bcolj[96];
                        s += key[i * index.keysz+97] * Bcolj[97];
                        s += key[i * index.keysz+98] * Bcolj[98];
                        s += key[i * index.keysz+99] * Bcolj[99];
                        s += key[i * index.keysz+100] * Bcolj[100];
                        s += key[i * index.keysz+101] * Bcolj[101];
                        s += key[i * index.keysz+102] * Bcolj[102];
                        s += key[i * index.keysz+103] * Bcolj[103];
                        s += key[i * index.keysz+104] * Bcolj[104];
                        s += key[i * index.keysz+105] * Bcolj[105];
                        s += key[i * index.keysz+106] * Bcolj[106];
                        s += key[i * index.keysz+107] * Bcolj[107];
                        s += key[i * index.keysz+108] * Bcolj[108];
                        s += key[i * index.keysz+109] * Bcolj[109];
                        s += key[i * index.keysz+110] * Bcolj[110];
                        s += key[i * index.keysz+111] * Bcolj[111];
                        s += key[i * index.keysz+112] * Bcolj[112];
                        s += key[i * index.keysz+113] * Bcolj[113];
                        s += key[i * index.keysz+114] * Bcolj[114];
                        s += key[i * index.keysz+115] * Bcolj[115];
                        s += key[i * index.keysz+116] * Bcolj[116];
                        s += key[i * index.keysz+117] * Bcolj[117];
                        s += key[i * index.keysz+118] * Bcolj[118];
                        s += key[i * index.keysz+119] * Bcolj[119];
                        s += key[i * index.keysz+120] * Bcolj[120];
                        s += key[i * index.keysz+121] * Bcolj[121];
                        s += key[i * index.keysz+122] * Bcolj[122];
                        s += key[i * index.keysz+123] * Bcolj[123];
                        s += key[i * index.keysz+124] * Bcolj[124];
                        s += key[i * index.keysz+125] * Bcolj[125];
                        s += key[i * index.keysz+126] * Bcolj[126];
                        s += key[i * index.keysz+127] * Bcolj[127];

                    }
                    crypted[(i+index.vstart)*index.size + j + index.hstart] = s;

                }
            }
        }
    }
    free(Bcolj);



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



