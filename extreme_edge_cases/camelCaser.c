/**
 * extreme_edge_cases
 * CS 241 - Fall 2021
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    // base case, if input is NULL pointer, return NULL
    if (!*input_str || !input_str) {
        return NULL;
    }

    // count number of sentences
    int numSentences = 0;
    char * iterator = (char*) input_str;
    while (*iterator) {
        if (ispunct(*iterator)) {
            numSentences++;
        }
        iterator++;
    }
    // printf("num sentences %d \n", numSentences);
    // printf("loop 1 passed \n");

    if (numSentences == 0) {
        return NULL;
    }

    // allocate based on number of sentences
    char ** output_s = malloc(sizeof(char*) * numSentences + 1);

    // count size of each sentence
    //reset iterator 
    iterator = (char*) input_str;
    int i = 0;
    int count = 0;
    while (*iterator) {
        if (ispunct(*iterator)) {
            // plus one for the null byte
            output_s[i] = malloc(count + 1);
            i++;
            count = 0;
        } else if (isspace(*iterator)){
            count--;
        }
        count++;
        iterator++;
    }

    // printf("loop 2 passed \n");
    
    // time for actually iterating through and adding the stuff
    int isFirstWord = 1;
    int isNewWord = 0;
    i = 0;
    int j = 0;
    //reset iterator 
    iterator = (char*) input_str;

    while (*iterator) {
        if (ispunct(*iterator)) {
            output_s[i][j] = '\0';
            i++;
            j = -1;
            isFirstWord = 1;
        } else if (isspace(*iterator)){
            j--;
            isNewWord = 1;
        } else if (isalpha(*iterator)) {
            if (isFirstWord) {
                // printf("hit first word\n");
                output_s[i][j] = tolower(*iterator);
                // printf("output_s[i][j] %c \n", *iterator);
                // printf("i is %d j is %d \n", i, j);
                isFirstWord = 0;
                isNewWord = 0;
            } else if (isNewWord) {
                output_s[i][j] = toupper(*iterator);
                isNewWord = 0;
            } else {
                output_s[i][j] = tolower(*iterator);
            }
        } else {
            output_s[i][j] = *iterator;
        }
        iterator++;
        j++;
    }

    // printf("loop 3 passed \n");
    // printf("iterator is %c \n", *iterator);

    // printf("output_s[1][0] is %s \n", output_s[1]);

    return output_s;
}

void destroy(char **result) {
    // TODO: Implement me!
    if (!result) {
        return;
    }

    int count = 0;
    while (result[count]) {
        free(result[count]);
        count++;
    }
    free(result);
    result = NULL;
    return;
}
