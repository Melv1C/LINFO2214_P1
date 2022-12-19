//
// Created by melvyn on 11/10/22.
//

#include "log.h"

void print_matrix(float * matrix, uint32_t size){

    printf("MATRIX %d\n",size);

    for (int i = 0; i < size; i++) {
        printf("| ");
        for (int j = 0; j < size; j++) {
            printf("%.6f ",matrix[i*size+j]);
        }
        printf(" |\n");
    }
}
