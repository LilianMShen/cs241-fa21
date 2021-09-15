/**
 * vector
 * CS 241 - Fall 2021
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char * string;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring * s = malloc(sizeof(sstring));
    s->string = malloc(strlen(input) + 1);
    strcpy(s->string, input);
    return s;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char * toReturn = malloc(strlen(input->string) + 1);
    strcpy(toReturn, input->string);
    return toReturn;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    int n = strlen(this->string);
    this->string = realloc(this->string, n + (strlen(addition->string) + 1));
    strcpy(this->string + n, addition->string);
    n = strlen(this->string);
    return n;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector * toReturn = string_vector_create();
    char * iterator = this->string;
    int i = 0;
    int j = 0;
    while(*iterator) {
        if (*iterator == delimiter) {
            vector_push_back(toReturn, sstring_slice(this, i, j));
            i = j + 1;
        }
        j++;
        iterator++;
    }
    vector_push_back(toReturn, sstring_slice(this, i, j));
    // strcpy(copy, this->string);
    // char * token = strtok(copy, &delimiter);
    // while (token) {
    //     vector_push_back(toReturn, token);
    //     token = strtok(NULL, &delimiter);
    // }
    // vector_push_back(toReturn, NULL);
    // free(copy);
    return toReturn;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    char * iterator = &(this->string[offset]);
    size_t len = strlen(target);
    size_t i = offset;
    int flagFound = 0;
    char * copy;
    while(*iterator) {
        if(strncmp(iterator, target, len) == 0) {
            flagFound = 1;
            break;
        }
        i++;
        iterator++;
    }
    if (flagFound) {
        copy = sstring_slice(this, 0, i);
        strncpy(&copy[i], substitution, strlen(substitution));
        strcpy(&copy[i + strlen(substitution)], &this->string[i + strlen(target)]);
        strcpy(this->string, copy);
        free(copy);
        return 0;
    }
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    int size = end - start;
    char * toReturn = malloc((sizeof(char) * size) + 1);
    for (int i = 0; i < size; i++) {
        toReturn[i] = this->string[start + i];
    }
    toReturn[size] = '\0';
    return toReturn;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this->string);
    free(this);
}
