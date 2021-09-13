/**
 * vector
 * CS 241 - Fall 2021
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

int main(int argc, char *argv[]) {
    vector *v = string_vector_create();

    // Testing basic adding
    printf("Testing basic adding... ");
    char *t = "test";
    char *u = "testing";
    vector_push_back(v, t);
    vector_push_back(v, u);
    assert(strcmp("testing", vector_get(v, 1)) == 0);
    printf("done\n");

    // Testing erase
    printf("Testing erase... ");
    vector_erase(v, 0);
    assert(vector_size(v) == 1);
    assert(strcmp("testing", vector_get(v, 0)) == 0);
    assert(strcmp("testing", *(char**)vector_begin(v)) == 0);
    assert(strcmp("testing", *(char**)vector_back(v)) == 0);
    printf("done\n");

    // Test automatic resize
    printf("Testing automatic resize... ");
    int i;
    for (i = 0; i < 128; i++) {
        vector_push_back(v, "hello");
    }
    assert(vector_size(v) == 129);
    assert(strcmp("hello", vector_get(v, 128)) == 0);
    printf("done\n");

    // Test destroy
    printf("Testing destroy... ");
    vector_destroy(v);
    v = NULL;
    v = int_vector_create();
    for (int i = 0; i < 128; i++) {
        vector_push_back(v, &i);
    }
    printf("done\n");

    // Test resize
    printf("Testing resize... ");

    vector_resize(v, 64);
    assert(vector_size(v) == 64);
    assert(**(int**)vector_back(v) == 63);
    
    vector_resize(v, 128);
    assert(vector_size(v) == 128);
    assert(**(int**)vector_back(v) == 0);

    printf("done\n");

    // Test insert
    printf("Testing insert... ");
    int x = -1;
    vector_insert(v, 1, &x);
    assert(*(int*)vector_get(v, 1) == -1);
    assert(vector_size(v) == 129);
    printf("done\n");

    // Test pop back
    printf("Testing pop back... ");
    vector_resize(v, 12);
    vector_pop_back(v);
    assert(vector_size(v) == 11);
    assert(**(int**)vector_back(v) == 9);
    printf("done\n");

    // Test clear
    printf("Testing clear... ");
    vector_clear(v);
    assert(vector_size(v) == 0);
    printf("done\n");
    
    vector_destroy(v);

    printf("All tests passed!\n");

    // vector_destroy(v);
    v = NULL;

}