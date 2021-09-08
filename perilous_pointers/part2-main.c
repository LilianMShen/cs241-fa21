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
    char c = ' ';
    *(char*)voival = c;
    empty_step(voival);
    free(voival);

    // six
    *charVal = c;
    charVal[3] = 'u';
    voival = charVal;
    two_step(voival, charVal);


    // seven
    char * charVal2 = charVal + 2;
    char * charVal3 = charVal2 + 2;
    three_step(charVal, charVal2, charVal3);

    // eight
    charVal2[2] = charVal[1] + 8;
    charVal3[3] = charVal2[2] + 8;
    step_step_step(charVal, charVal2, charVal3);

    // nine
    val = 32;
    it_may_be_odd(charVal, val);

    // ten
    char str[12] = "CS241,CS241";
    tok_step((char*) str);

    // eleven
    void* orange = malloc(sizeof(int*));
    void* blue = orange;
    ((char *)blue)[0] = 1;
    *(int*)orange = 513;
    the_end(orange, blue);

    free(orange);
    free(charVal); 
    return 0;
}
