/**
 * mini_memcheck
 * CS 241 - Fall 2021
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Your tests here using malloc and free
    char * tenString = malloc(10);
    // char * fiveString = malloc(5);
    tenString = realloc(tenString, 15);
    free(tenString);
    // free(fiveString);

    return 0;
}