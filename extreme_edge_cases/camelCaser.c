/**
 * extreme_edge_cases
 * CS 241 - Fall 2021
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    // base case, if input is NULL pointer, return NULL
    if (!input_str) {
        return NULL;
    }

    // count number of sentences
    int numSentences = 0;
    char * iterator = (char*) input_str;
    while (iterator) {
        if (ispunct(*iterator)) {
            numSentences++;
        }
        iterator++;
    }

    // allocate based on number of sentences
    char ** output_s = malloc(sizeof(char*) * numSentences);

    // count size of each sentence
    //reset iterator 
    iterator = (char*) input_str;
    int i = 0;
    int count = 0;
    while (iterator) {
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
    // one final sentence that might not end in punctuation?
    // output_s[i] = malloc(count + 1);
    
    // time for actually iterating through and adding the stuff
    int isFirstWord = 1;
    int isNewWord = 1;
    i = 0;
    int j = 0;
    //reset iterator 
    iterator = (char*) input_str;

    while (iterator) {
        if (ispunct(*iterator)) {
            output_s[i][j] = '\0';
            i++;
            j = 0;
            isFirstWord = 1;
        } else if (isspace(*iterator)){
            j--;
            isNewWord = 1;
        } else if (isalpha(*iterator)) {
            if (isFirstWord) {
                output_s[i][j] = tolower(*iterator);
                isFirstWord = 0;
            } else if (isNewWord) {
                output_s[i][j] = toupper(*iterator);
                isNewWord = 0;
            } else {
                output_s[i][j] = tolower(*iterator);
            }
        } else {
            output_s[i][j] = *iterator;;
        }
        iterator++;
        j++;
    }

    return output_s;
}

void destroy(char **result) {
    // TODO: Implement me!
    int count = 0;
    while (result[count]) {
        free(result[count]);
        count++;
    }
    free(result);
    return;
}
