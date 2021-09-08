/**
 * extreme_edge_cases
 * CS 241 - Fall 2021
 */
#include <stdio.h>

#include "camelCaser_ref_utils.h"

int main() {
    // Enter the string you want to test with the reference here.
    char *input = "hello. welcome to cs241";

    // This function prints the reference implementation output on the terminal.
    print_camelCaser(input);

    // Put your expected output for the given input above.
    char *correct[] = {"hello", NULL};
    char *wrong[] = {"hello", "welcomeToCs241", NULL};

    // Compares the expected output you supplied with the reference output.
    printf("check_output test 1: %d\n", check_output(input, correct));
    printf("check_output test 2: %d\n", check_output(input, wrong));

    // Feel free to add more test cases.

    char c;
    input = &c;
    char *correct3[] = {NULL, NULL};
    printf("check_output test 3: %d\n", check_output(input, correct3));

    input = "thIs is a test iNPuT. cs241 iS SO MUCH FUN!!!!!!";
    char *correct4[] = {"thisIsATestInput", "cs241IsSoMuchFun", "", "", "", "", "", NULL};
    printf("check_output test 4: %d\n", check_output(input, correct4));

    input = "CAPS LOCK TEST. THIS IS A CAPS LOCK TEST.";
    char *correct5[] = {"capsLockTest", "thisIsACapsLockTest", NULL};
    printf("check_output test 5: %d\n", check_output(input, correct5));
    
    input = "lower case test. this is a lower case test.";
    char *correct6[] = {"lowerCaseTest", "thisIsALowerCaseTest", NULL};
    printf("check_output test 6: %d\n", check_output(input, correct6));

    input = "                  .";
    char *correct7[] = {"", NULL};
    printf("check_output test 7: %d\n", check_output(input, correct7));

    input = "no punctuation";
    char *correct8[] = {NULL};
    printf("check_output test 8: %d\n", check_output(input, correct8));
    // char* input = "^&$@%#%$*^%#&((&%&^$$&^*&^*&%^&$&^%*&^(";
    return 0;
}
