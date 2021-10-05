/**
 * malloc
 * CS 241 - Fall 2021
 */
#include "tester-utils.h"

#define NUM_CYCLES 10000000

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; i < NUM_CYCLES; i++) {
        if ((i + 1) % 10 == 0) printf("%d \n", i);
        int *ptr = malloc(sizeof(int));

        if (ptr == NULL) {
            printf("hit NULL ERR\n");
            fprintf(stderr, "Memory failed to allocate!\n");
            return 1;
        }

        *ptr = 4;
        free(ptr);
    }
    printf("exit\n");

    fprintf(stderr, "Memory was allocated, used, and freed!\n");
    return 0;
}
