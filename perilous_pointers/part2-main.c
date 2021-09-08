/**
 * perilous_pointers
 * CS 241 - Fall 2021
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    // one
    int val = 81;
    first_step(val);

    // two
    val = 132;
    int* val2 = &val;
    second_step(val2);

    // three
    val = 8942;
    int* val3[] = {val2};
    double_step(val3);

    // four
    char* charVal = malloc(sizeof(char));
    charVal[5] = '\n' + 5;\
    strange_step(charVal);
    

    // five
    void* voival = malloc(sizeof(char));
    char a = ' ';
    *(char*)voival = a;
    empty_step(voival);

    // six


    //seven

    
    free(charVal);
    free(voival);   
    return 0;
}
