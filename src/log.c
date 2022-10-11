//
// Created by melvyn on 11/10/22.
//

#include "log.h"

void print_matrix(uint8_t* matrix, uint32_t size){
    for (int i = 0; i < sqrt(size); i++) {
        printf("| ");
        for (int j = 0; j < 10; j++) {
            printf("%u ",matrix[i*(int)sqrt(size)+j]);
        }
        printf(" |\n");
    }
}
