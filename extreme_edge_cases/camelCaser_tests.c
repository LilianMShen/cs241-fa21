/**
 * extreme_edge_cases
 * CS 241 - Fall 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int correctOutput(char* correct[], char**result) {
    if (correct[0] == NULL && result == NULL) {
        return 1;
    }
    int index = 0;
    char* i = correct[index];
    char* j = result[index];
    
    while (i != NULL || j != NULL) {
        // printf("i is %s ", i);
        // printf("j is %s \n", j);
        // printf("start of loop %d\n", index);

        //base case, if inequal number of items
        if (i == NULL || j == NULL) return 0;
        // if ((*i && !*j) || (*j && !*i)) return 0;

        if (strcmp(i, j) != 0) return 0;

        index++;
        i = correct[index];
        j = result[index];
        // printf("end of loop %d\n", index - 1);
    }
    // printf("beat the loop \n");

    return 1;
}


int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!

    // NULL input test
    char* input = NULL;
    char *correct1[] = {NULL};
    char** output = (*camelCaser)(input);
    if (!correctOutput(correct1, output)) {
        printf("failed 1");
        (*destroy)(output);
        return 0;
    }
    (*destroy)(output);


    // // crazy lettering test, multiple punctuation test
    input = "thIs is a test iNPuT. cs241 iS SO MUCH FUN!!!!!!";
    char *correct2[] = {"thisIsATestInput", "cs241IsSoMuchFun", "", "", "", "", "", NULL};
    output = (*camelCaser)(input);
    if (!correctOutput(correct2, output)) {
        printf("failed 2");
        (*destroy)(output);
        return 0;
    }
    (*destroy)(output);

    // // caps lock test
    input = "CAPS LOCK TEST. THIS IS A CAPS LOCK TEST.";
    char *correct3[] = {"capsLockTest", "thisIsACapsLockTest", NULL};
    output = (*camelCaser)(input);
    if (!correctOutput(correct3, output)) {
        printf("failed 3");
        (*destroy)(output);
        return 0;
    }
    (*destroy)(output);

    // // lower case test
    input = "lower case test. this is a lower case test.";
    char *correct4[] = {"lowerCaseTest", "thisIsALowerCaseTest", NULL};
    output = (*camelCaser)(input);
    if (!correctOutput(correct4, output)) {
        printf("failed 4");
        (*destroy)(output);
        return 0;
    }
    (*destroy)(output);

    // // space test
    input = "                  .";
    char *correct5[] = {"", NULL};
    output = (*camelCaser)(input);
    if (!correctOutput(correct5, output)) {
        printf("failed 5");
        (*destroy)(output);
        return 0;
    }
    (*destroy)(output);

    // // no punctuation test
    input = "no punctuation";
    char *correct6[] = {NULL};
    output = (*camelCaser)(input);
    if (!correctOutput(correct6, output)) {
        printf("failed 6");
        (*destroy)(output);
        return 0;
    }
    (*destroy)(output);

    // char* input = "^&$@%#%$*^%#&((&%&^$$&^*&^*&%^&$&^%*&^(";
    printf("all passed! \n");
    return 1;
}

